/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "SimpleAnalyzer.h"

#include "Exceptions.h"
#include "NodeEventContext.h"
#include "Interventions.h"

SETUP_LOGGING( "SimpleAnalyzer" )

namespace Kernel
{

    //---------------------------------------------------------------------------------------------
    // ctor
    //---------------------------------------------------------------------------------------------

    SimpleAnalyzer::SimpleAnalyzer() : m_timer(0), m_parent(NULL)
    {
        initConfigTypeMap("Reporting_Interval", &m_reporting_interval, "Reporting Interval in Days", 0, 1000000, 1);
        initConfigTypeMap("Coverage",           &m_coverage, "Coverage Fraction", 0, 1, 1);
    }

    //---------------------------------------------------------------------------------------------
    // dtor
    //---------------------------------------------------------------------------------------------

    SimpleAnalyzer::~SimpleAnalyzer()
    {
    }

    //---------------------------------------------------------------------------------------------
    // InterventionFactory
    //---------------------------------------------------------------------------------------------

    IMPLEMENT_FACTORY_REGISTERED(SimpleAnalyzer)

    //---------------------------------------------------------------------------------------------
    // JsonConfigurable
    //---------------------------------------------------------------------------------------------

    bool SimpleAnalyzer::Configure( const Configuration * inputJson )
    {
        initConfig( "Trigger_Condition", m_trigger_condition, "Condition triggering analyzer" );
        return JsonConfigurable::Configure( inputJson );
    }

    //---------------------------------------------------------------------------------------------
    // INodeDistributableIntervention
    //---------------------------------------------------------------------------------------------

    bool SimpleAnalyzer::Distribute( INodeEventContext *pNodeEventContext, IEventCoordinator2 *pEC )
    {
        // QI to register NodeEventContext as a "consumer" of this individual event observer
        IIndividualEventBroadcaster * broadcaster = pNodeEventContext->GetIndividualEventBroadcaster();
        broadcaster->RegisterObserver( this, m_trigger_condition );

        // Set up the function for registering individuals when we visit them
        INodeEventContext::individual_visit_function_t visit_func = 
            [this](IIndividualHumanEventContext *ihec)
        {
            // Some logic for whom to register
            if ( m_coverage < 1 && m_parent->GetRng()->e() > m_coverage ) return;
            
            // QI to register an individual's InterventionsContainer as a "consumer" of this individual event observer
            IIndividualTriggeredInterventionConsumer * pITIC = NULL;
            if (s_OK == ihec->GetInterventionsContext()->QueryInterface(GET_IID(IIndividualTriggeredInterventionConsumer), (void**)&pITIC) )
            {
                pITIC->RegisterObserver( this );
            }
            else
            {
                throw QueryInterfaceException(__FILE__,__LINE__,__FUNCTION__,"ihec->GetInterventionsContext()","IIndividualTriggeredInterventionConsumer","IIndividualHumanInterventionsContext");
            }
        };

        // Now visit all the individuals 
        pNodeEventContext->VisitIndividuals(visit_func);

        return true;
    }

    void SimpleAnalyzer::SetContextTo(INodeEventContext *context)
    {
        m_parent = context;
    }

    void SimpleAnalyzer::Update(float dt)
    {
        // Do something more interesting than this eventually...
        m_timer += dt;

        if ( m_timer >= m_reporting_interval )
        {
            LOG_INFO("Resetting SimpleAnalyzer timer...\n");
            m_timer = 0;
        }
    }

    //---------------------------------------------------------------------------------------------
    // ISupports
    //---------------------------------------------------------------------------------------------

    BEGIN_QUERY_INTERFACE_BODY(SimpleAnalyzer)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_INTERFACE(IIndividualEventObserver)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleAnalyzer)

    //---------------------------------------------------------------------------------------------
    // IIndividualEventObserver
    //---------------------------------------------------------------------------------------------

    bool SimpleAnalyzer::notifyOnEvent( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger )
    {
        // Check if this type of stat-change warrants a notification
        if(StateChange != GetTriggerCondition())
        {
            return false;
        }

        // Do something more interesting than this eventually...
        LOG_INFO_F( "SimpleAnalyzer notified of event by %d-year old individual.\n", (int) (context->GetAge() / DAYSPERYEAR) );

        return true;
    }

    std::string SimpleAnalyzer::GetTriggerCondition() const
    {
        return m_trigger_condition.ToString();
    }

    void SimpleAnalyzer::JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const
    {
        throw new NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "JSON serialization not yet implemented.");
    }

    void SimpleAnalyzer::JDeserialize( IJsonObjectAdapter* root, JSerializer* helper )
    {
        throw new NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "JSON deserialization not yet implemented.");
    }
}
