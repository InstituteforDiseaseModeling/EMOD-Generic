/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "AdherentDrug.h"

#include <numeric>      // std::accumulate

#include "IWaningEffect.h"
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "IndividualEventContext.h"
#include "NodeEventContext.h"
#include "IIndividualHumanContext.h"
#include "RANDOM.h"

SETUP_LOGGING( "AdherentDrug" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED( AdherentDrug, AntimalarialDrug )
    END_QUERY_INTERFACE_DERIVED( AdherentDrug, AntimalarialDrug )

    IMPLEMENT_FACTORY_REGISTERED( AdherentDrug )

    AdherentDrug::AdherentDrug()
        : AntimalarialDrug()
        , m_pAdherenceEffect(nullptr)
        , m_NonAdherenceOptions()
        , m_NonAdherenceCdf()
        , m_TookDoseEvent()
        , m_MaxDuration( FLT_MAX )
        , m_CurrentDuration(0.0)
        , m_CurrentDose(0)
        , m_TotalDoses(0)
        , m_DoseEffects(1.0f,FLT_MAX,0.0f,1.0f)
    {
    }

    AdherentDrug::AdherentDrug( const AdherentDrug& rOrig )
        : AntimalarialDrug( rOrig )
        , m_pAdherenceEffect( nullptr )
        , m_NonAdherenceOptions( rOrig.m_NonAdherenceOptions )
        , m_NonAdherenceCdf( rOrig.m_NonAdherenceCdf )
        , m_TookDoseEvent( rOrig.m_TookDoseEvent )
        , m_MaxDuration( rOrig.m_MaxDuration )
        , m_CurrentDuration( rOrig.m_CurrentDuration )
        , m_CurrentDose( rOrig.m_CurrentDose )
        , m_TotalDoses( rOrig.m_TotalDoses )
        , m_DoseEffects( rOrig.m_DoseEffects )
    {
        if(rOrig.m_pAdherenceEffect)
        {
            m_pAdherenceEffect = rOrig.m_pAdherenceEffect->Clone();
        }
    }

    AdherentDrug::~AdherentDrug()
    {
        delete m_pAdherenceEffect;
        m_pAdherenceEffect = nullptr;
    }

    bool AdherentDrug::Configure( const Configuration * inputJson )
    {
        std::vector<float> non_adherence_probability;

        initVectorConfig( "Non_Adherence_Options",
                          m_NonAdherenceOptions,
                          inputJson,
                          MetadataDescriptor::VectorOfEnum("Non_Adherence_Options", AD_Non_Adherence_Options_DESC_TEXT, MDD_ENUM_ARGS( NonAdherenceOptionsType )));

        initConfigTypeMap(     "Non_Adherence_Distribution",      &non_adherence_probability, AD_Non_Adherence_Distribution_DESC_TEXT, 0.0f, 1.0f ); 
        initConfigTypeMap(     "Max_Dose_Consideration_Duration", &m_MaxDuration,             AD_Max_Dose_Consideration_Duration_DESC_TEXT, (1.0f/24.0f), FLT_MAX, FLT_MAX ); // 1-hour=1/24
        initConfig( "Took_Dose_Event", m_TookDoseEvent, inputJson, MetadataDescriptor::Enum("Non_Adherence_Options", AD_Took_Dose_Event_DESC_TEXT, MDD_ENUM_ARGS( EventTrigger ) ) ); 

        m_pAdherenceEffect = WaningEffectFactory::CreateInstance();

        initConfigTypeMap( "Adherence_Config",    m_pAdherenceEffect->GetConfigurable(),  AD_Adherence_Config_DESC_TEXT);
        initConfigTypeMap( "Adherence_By_Dose",  &m_DoseEffects,                          AD_Adherence_By_Dose_DESC_TEXT);


        bool ret = AntimalarialDrug::Configure( inputJson );

        if( ret && !JsonConfigurable::_dryrun )
        {
            int num_doses = GetNumDoses();
            if( m_DoseEffects.size() && m_DoseEffects.rbegin()->first < num_doses )
            {
                std::stringstream ss;
                ss << "'Adherence_By_Dose' is not configured correctly." << std::endl;
                ss << "'Drug_Type'=" << drug_name << " is configured for " << num_doses << " dose(s)" << std::endl;
                ss << "but the Adherence_By_Dose does not support that number of doses." << std::endl;
                ss << "There should probably be one entry for each dose.";
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }

            // ----------------------------------------------------
            // --- Ensure that both input array are the same size.
            // --- There must be one probability for each option.
            // ----------------------------------------------------
            if( m_NonAdherenceOptions.size() != non_adherence_probability.size() )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                                                        "Non_Adherence_Options.<num elements>",      int(m_NonAdherenceOptions.size()), 
                                                        "Non_Adherence_Distribution.<num elements>", int( non_adherence_probability.size()),
                                                        "These two arrays must have the same number of elements." );
            }
            else if( m_NonAdherenceOptions.size() == 0 )
            {
                m_NonAdherenceOptions.push_back( NonAdherenceOptionsType::NEXT_UPDATE );
                non_adherence_probability.push_back( 1.0 );
            }

            // --------------------------------------------
            // --- There can be at most one of each option
            // --------------------------------------------
            std::set<NonAdherenceOptionsType::Enum> opts_set;
            for( int i = 0 ; i < m_NonAdherenceOptions.size() ; ++i )
            {
                opts_set.insert( m_NonAdherenceOptions[ i ] );
            }
            if( opts_set.size() != m_NonAdherenceOptions.size() )
            {
                std::stringstream ss;
                ss << "'Non_Adherence_Options' has duplicate entries.  It can have at most one of each enum.\n";
                ss << "'Non_Adherence_Options' values are: ";
                for( int i = 0; i < m_NonAdherenceOptions.size(); ++i )
                {
                    ss << NonAdherenceOptionsType::pairs::lookup_key( m_NonAdherenceOptions[i] );
                    if( (i + 1) < m_NonAdherenceOptions.size() )
                    {
                        ss << ", ";
                    }
                }
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }

            // ------------------------------------------
            // --- Ensure that the probabilities sum to 1
            // ------------------------------------------
            float total_prob = std::accumulate( non_adherence_probability.begin(), non_adherence_probability.end(), 0.0f );
            if( (total_prob < 0.99999) || (1.000001 < total_prob) )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                                                        "Sum(Non_Adherence_Distribution)", total_prob,
                                                        "Required", 1.0f,
                                                        "The values of 'Non_Adherence_Distribution' must sum to 1.0." );
            }

            // ------------------------------------------------
            // --- Remove any option with a probability of zero
            // ------------------------------------------------
            for( int i = 0; i < non_adherence_probability.size(); /* incremented in loop */ )
            {
                if( non_adherence_probability[ i ] == 0.0 )
                {
                    non_adherence_probability.erase( non_adherence_probability.begin() + i );
                    m_NonAdherenceOptions.erase(     m_NonAdherenceOptions.begin()     + i );
                }
                else
                {
                    ++i;
                }
            }

            // ---------------------------------------------------------------------
            // --- Turn the probabilities into a cumulative distribution function.
            // --- This will allow us to draw one random number when selecting the
            // --- option.
            // ---------------------------------------------------------------------
            m_NonAdherenceCdf.push_back( non_adherence_probability[ 0 ] );
            for( int i = 1; i < non_adherence_probability.size(); ++i )
            {
                float val = m_NonAdherenceCdf[ i - 1 ] + non_adherence_probability[ i ];
                m_NonAdherenceCdf.push_back( val );
            }
            m_NonAdherenceCdf.back() = 1.0;

            release_assert( m_NonAdherenceOptions.size() == m_NonAdherenceCdf.size() );
        }
        return ret;
    }

    void AdherentDrug::ConfigureDrugTreatment( IIndividualHumanInterventionsContext * ivc )
    {
        AntimalarialDrug::ConfigureDrugTreatment( ivc );
        m_TotalDoses = remaining_doses;
        if( m_TotalDoses == 0 )
        {
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "A drug with zero doses?" );
        }

        // ---------------------------------------------------------------
        // --- Warn the user if the configure m_MaxDuration such that the
        // --- individual cannot take all of the doses
        // ---------------------------------------------------------------
        float normal_duration = m_TotalDoses * time_between_doses;
        if( (m_TotalDoses != -1) && (m_MaxDuration < normal_duration) )
        {
            LOG_WARN_F("'Max_Dose_Consideration_Duration'(=%f) is less than total doses(=%d) times Drug_Dose_Interval(=%f).  The individual will not take all doses.\n",
                        m_MaxDuration,m_TotalDoses,time_between_doses);
        }
    }

    void AdherentDrug::SetContextTo( IIndividualHumanContext *context )
    {
        if(m_pAdherenceEffect)
        {
            m_pAdherenceEffect->SetContextTo( context );
        }
        AntimalarialDrug::SetContextTo( context );
    }

    void AdherentDrug::Update( float dt )
    {
        if(m_pAdherenceEffect)
        {
            m_pAdherenceEffect->Update( dt );
            if(m_pAdherenceEffect->Expired())
            {
                delete m_pAdherenceEffect;
                m_pAdherenceEffect = nullptr;
            }
        }

        m_CurrentDose = 1; // m_TotalDoses = -1 implies infinte doses so always the first dose
        if( m_TotalDoses > 0 )
        {
            m_CurrentDose = m_TotalDoses - remaining_doses + 1;
        }

        m_CurrentDuration += dt;

        AntimalarialDrug::Update( dt );
    }

    bool AdherentDrug::IsTakingDose( float dt )
    {
        // --------------------------------------------------------------------
        // --- If m_CurrentDuration > m_MaxDuration, then the user should stop
        // --- trying to take the drug.  We don't expire the drug because if
        // --- the person took a dose, they still get its effect.
        // --------------------------------------------------------------------

        bool is_taking_dose = false;
        if( m_CurrentDuration <= m_MaxDuration )
        {
            float prob_adhere = 1.0f;
            if(m_pAdherenceEffect)
            {
                prob_adhere *= m_pAdherenceEffect->Current();
            }
            if(m_DoseEffects.size())
            {
                prob_adhere *= m_DoseEffects.getValuePiecewiseConstant(m_CurrentDose);
            }
            is_taking_dose = parent->GetRng()->SmartDraw( prob_adhere );
        }

        if( !is_taking_dose )
        {
            NonAdherenceOptionsType::Enum non_adherence_opt = NonAdherenceOptionsType::STOP;
            if( m_CurrentDuration <= m_MaxDuration )
            {
                non_adherence_opt = SelectNonAdherenceOption();
            }

            switch( non_adherence_opt )
            {
                case NonAdherenceOptionsType::NEXT_UPDATE:
                    dosing_timer = dt;
                    break;

                case NonAdherenceOptionsType::NEXT_DOSAGE_TIME:
                    dosing_timer = time_between_doses;
                    break;

                case NonAdherenceOptionsType::LOST_TAKE_NEXT:
                    remaining_doses -= 1;
                    is_taking_dose = (remaining_doses > 0);
                    break;

                case NonAdherenceOptionsType::STOP:
                    remaining_doses = 0;
                    break;

                default:
                    throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "non_adherence_opt", non_adherence_opt, NonAdherenceOptionsType::pairs::lookup_key( non_adherence_opt ) );
            }
        }

        if( is_taking_dose && m_TookDoseEvent != EventTrigger::NoTrigger )
        {
            IIndividualEventBroadcaster* broadcaster = parent->GetEventContext()->GetNodeEventContext()->GetIndividualEventBroadcaster();
            broadcaster->TriggerObservers( parent->GetEventContext(), m_TookDoseEvent );
        }

        return is_taking_dose;
    }

    NonAdherenceOptionsType::Enum AdherentDrug::SelectNonAdherenceOption()
    {
        NonAdherenceOptionsType::Enum non_adherence_opt = m_NonAdherenceOptions[ 0 ];
        if( m_NonAdherenceOptions.size() > 1 )
        {
            float ran = parent->GetRng()->e();
            for( int i = 0; i < m_NonAdherenceCdf.size(); ++i )
            {
                if( ran <= m_NonAdherenceCdf[ i ] )
                {
                    non_adherence_opt = m_NonAdherenceOptions[ i ];
                    break;
                }
            }
        }
        return non_adherence_opt;
    }

    REGISTER_SERIALIZABLE( AdherentDrug );

    void serialize( IArchive& ar, std::vector<NonAdherenceOptionsType::Enum>& rOptions )
    {
        size_t count = ar.IsWriter() ? rOptions.size() : -1;

        ar.startArray( count );
        if( ar.IsWriter() )
        {
            for( auto& entry : rOptions )
            {
                ar & (uint32_t&)entry;
            }
        }
        else
        {
            rOptions.resize( count );
            for( size_t i = 0; i < count; ++i )
            {
                ar & (uint32_t&)rOptions[ i ];
            }
        }
        ar.endArray();
    }

    void AdherentDrug::serialize( IArchive& ar, AdherentDrug* obj )
    {
        AntimalarialDrug::serialize( ar, obj );
        AdherentDrug& drug = *obj;
        ar.labelElement( "m_pAdherenceEffect"    ) & drug.m_pAdherenceEffect;
        ar.labelElement( "m_NonAdherenceOptions" ); 
        Kernel::serialize( ar, drug.m_NonAdherenceOptions );

        ar.labelElement( "m_NonAdherenceCdf"     ) & drug.m_NonAdherenceCdf;
        ar.labelElement( "m_MaxDuration"         ) & drug.m_MaxDuration;
        ar.labelElement( "m_CurrentDuration"     ) & drug.m_CurrentDuration;
        ar.labelElement( "m_CurrentDose"         ) & drug.m_CurrentDose;
        ar.labelElement( "m_TotalDoses"          ) & drug.m_TotalDoses;
        ar.labelElement( "m_DoseEffects"         ) & drug.m_DoseEffects;
    }
}
