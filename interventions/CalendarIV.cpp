/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "CalendarIV.h"

#include <stdlib.h>

#include "Debug.h"
#include "CajunIncludes.h"     // for parsing calendar and actual interventions
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "NodeEventContext.h"  // for INodeEventContext (ICampaignCostObserver)
#include "IIndividualHumanContext.h"
#include "ISimulationContext.h"
#include "RANDOM.h"
#include "JsonConfigurableCollection.h"

SETUP_LOGGING( "IVCalendar" )

namespace Kernel
{
    // --------------------
    // --- AgeAndProbability
    // --------------------
    class AgeAndProbability : public JsonConfigurable
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) { return e_NOINTERFACE; }

    public:
        AgeAndProbability()
            : JsonConfigurable()
            , m_Age( 0.0 )
            , m_Prob( 0.0 )
        {
        }

        AgeAndProbability( const AgeAndProbability& rMaster )
            : JsonConfigurable( rMaster )
            , m_Age( rMaster.m_Age )
            , m_Prob( rMaster.m_Prob )
        {
        }

        virtual ~AgeAndProbability()
        {
        }

        virtual bool Configure( const Configuration* inputJson ) override
        {
            initConfigTypeMap("Age", &m_Age, CAL_Age_DESC_TEXT, 0.0, MAX_HUMAN_AGE*DAYSPERYEAR, 0.0 );
            initConfigTypeMap("Probability", &m_Prob, CAL_Probability_DESC_TEXT, 0.0, 1.0, 0.0 );

            bool configured = JsonConfigurable::Configure( inputJson );
            return configured;
        }

        float m_Age;
        float m_Prob;
    };

    class AgeAndProbabilityList : public JsonConfigurableCollection<AgeAndProbability>
    {
    public:
        AgeAndProbabilityList()
            : JsonConfigurableCollection("AgeAndProbabilityList")
        {
        }

        AgeAndProbabilityList( const AgeAndProbabilityList& rMaster )
            : JsonConfigurableCollection( rMaster )
        {
        }

        virtual ~AgeAndProbabilityList()
        {
        }

    protected:
        virtual AgeAndProbability* CreateObject() override
        {
            return new AgeAndProbability();
        }
    };


    BEGIN_QUERY_INTERFACE_BODY(IVCalendar)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IBaseIntervention)
    END_QUERY_INTERFACE_BODY(IVCalendar)

    IMPLEMENT_FACTORY_REGISTERED(IVCalendar)

    IVCalendar::IVCalendar()
    : BaseIntervention()
    , age2ProbabilityMap()
    , actual_intervention_config()
    , dropout(false)
    , scheduleAges()
    {
    }

    IVCalendar::~IVCalendar()
    {
        LOG_DEBUG("Calendar was destroyed.\n");
    }

    bool
    IVCalendar::Configure(
        const Configuration * inputJson
    )
    {
        AgeAndProbabilityList age_prob_list;
        initConfigTypeMap("Dropout", &dropout, CAL_Dropout_DESC_TEXT, false);
        initConfigComplexCollectionType("Calendar", &age_prob_list, CAL_Calendar_DESC_TEXT);
        initConfigComplexType("Actual_IndividualIntervention_Configs", &actual_intervention_config, CAL_Actual_Intervention_Configs_DESC_TEXT);

        bool ret = BaseIntervention::Configure( inputJson );
        if( ret && !JsonConfigurable::_dryrun )
        {
            InterventionValidator::ValidateInterventionArray( GetTypeName(), 
                                                              InterventionTypeValidation::INDIVIDUAL,
                                                              actual_intervention_config._json,
                                                              inputJson->GetDataLocation() );

            for( int i = 0; i < age_prob_list.Size(); ++i )
            {
                float age = age_prob_list[ i ]->m_Age;
                float prob = age_prob_list[ i ]->m_Prob;
                age2ProbabilityMap.insert( std::make_pair( age, prob ) );
            }
        }
        return ret ;
    }

    bool
    IVCalendar::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * const pICCO
    )
    {
        parent = context->GetParent(); // is there a better way to get the parent?

        LOG_DEBUG("IVCalendar::Distribute\n");
        release_assert( parent );
        release_assert( parent->GetRng() );

        // Now's as good a time as any to parse in the calendar schedule.
        for( auto &entry: age2ProbabilityMap )
        {
            float age = entry.first;
            float probability = entry.second;

            if( parent->GetRng()->SmartDraw( probability ) )
            {
                scheduleAges.push_back( age );
            }
            else if( dropout )
            {
                LOG_DEBUG_F("dropout = true, so since %f dose was missed, all others missed as well\n", age);
                break;
            }
            else
            {
                LOG_DEBUG_F("Calendar stochastically rejected vaccine dose at age %f, but dropout = false so still might get others\n", age);
            }
        }

        LOG_DEBUG_F("%s\n", dumpCalendar().c_str());

        // Purge calendar entries that are in the past for this individual
        release_assert( parent->GetEventContext() );
        while( scheduleAges.size() > 0 && parent->GetEventContext()->GetAge() > scheduleAges.front() )
        {
            LOG_DEBUG("Calender given to individual already past age for part of schedule. Purging.\n" );
            scheduleAges.pop_front();
            if( scheduleAges.size() == 0 )
            {
                expired = true;
            }
        }

        return BaseIntervention::Distribute( context, pICCO );
    }

    // Each time this is called, the HSB intervention is going to decide for itself if
    // health should be sought. For start, just do it based on roll of the dice. If yes,
    // an intervention needs to be created (from what config?) and distributed to the
    // individual who owns us. Hmm...
    void IVCalendar::Update( float dt )
    {
        if( !BaseIntervention::UpdateIndividualsInterventionStatus() ) return;

        //calendar_age += dt;
        if( ( scheduleAges.size() > 0 ) && ( parent->GetEventContext()->GetAge() >= scheduleAges.front() ) )
        {
            scheduleAges.pop_front();
            if( scheduleAges.size() == 0 )
            {
                expired = true;
            }
            LOG_DEBUG_F("Calendar says it's time to apply an intervention...\n");
            // Check if actual_intervention_config is an array instead of JObject
            try
            {
                const json::Array & interventions_array = json::QuickInterpreter( actual_intervention_config._json ).As<json::Array>();
                LOG_DEBUG_F("interventions array size = %d\n", interventions_array.Size());

                // Important: Use the instance method to obtain the intervention factory obj instead of static method to cross the DLL boundary
                IGlobalContext *pGC = nullptr;
                const IInterventionFactory* ifobj = nullptr;
                if (s_OK == parent->QueryInterface(GET_IID(IGlobalContext), (void**)&pGC))
                {
                    ifobj = pGC->GetInterventionFactory();
                }
                if (!ifobj)
                {
                    throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "The pointer to IInterventionFactory object is not valid (could be DLL specific)" );
                }
                for( int idx=0; idx<interventions_array.Size(); ++idx )
                {
                    const json::Object& actualIntervention = json_cast<const json::Object&>(interventions_array[idx]);
                    Configuration * tmpConfig = Configuration::CopyFromElement( actualIntervention, "campaign" );
                    assert( tmpConfig );
                    IDistributableIntervention *di = const_cast<IInterventionFactory*>(ifobj)->CreateIntervention(tmpConfig);
                    delete tmpConfig;
                    tmpConfig = nullptr;
                    if( !di )
                    {
                        // Calendar wanted to distribute intervention but factory returned null pointer.
                        throw FactoryCreateFromJsonException( __FILE__, __LINE__, __FUNCTION__, "Unable to create intervention object from actualIntervention (apparently). Factory should actually throw exception. This exception throw is just paranoid exception handling." );
                    }
                    float interventionCost = 0.0f;
                    LOG_DEBUG_F("Calendar (intervention) distributed actual intervention at age %f\n", parent->GetEventContext()->GetAge());

                    // Now make sure cost gets reported.
                    ICampaignCostObserver* pICCO;
                    assert( parent->GetEventContext()->GetNodeEventContext() );
                    if (s_OK == parent->GetEventContext()->GetNodeEventContext()->QueryInterface(GET_IID(ICampaignCostObserver), (void**)&pICCO) )
                    {
                        di->Distribute( parent->GetInterventionsContext(), pICCO );
                    }
                    else
                    {
                        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent->GetEventContext()->GetNodeEventContext()", "ICampaignCostObserver", "INodeEventContext" );
                    }
                }
            }
            catch( json::Exception )
            {
                throw Kernel::JsonTypeConfigurationException( __FILE__, __LINE__, __FUNCTION__, "N/A", actual_intervention_config._json, "Expected STRING" );
            }
        }
        // TODO: Calendar may be done, should be disposed of somehow. How about parent->Release()??? :)
    }

    // This is a debug only utility function to dump out actual dosing calendars.
    std::string IVCalendar::dumpCalendar()
    {
        std::ostringstream msg;
        msg << "Dose Calendar: ";
        for (float age : scheduleAges)
        {
            msg << age << ",";
        }
        return msg.str();
    }

    REGISTER_SERIALIZABLE(IVCalendar);

    void IVCalendar::serialize(IArchive& ar, IVCalendar* obj)
    {
        BaseIntervention::serialize( ar, obj );
        IVCalendar& cal = *obj;
        ar.labelElement("age2ProbabilityMap") & cal.age2ProbabilityMap;
        ar.labelElement("actual_intervention_config") & cal.actual_intervention_config;
        ar.labelElement("dropout") & cal.dropout;
        ar.labelElement("scheduleAges") & cal.scheduleAges;
    }
}
