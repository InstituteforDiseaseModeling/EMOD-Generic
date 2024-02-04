/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "Environment.h"
#include "Susceptibility.h"
#include "ConfigParams.h"
#include "Common.h"

SETUP_LOGGING( "Susceptibility" )

namespace Kernel
{
    // Post-infection immunity
    float SusceptibilityConfig::baseacqupdate  = 1.0f;
    float SusceptibilityConfig::basetranupdate = 1.0f;
    float SusceptibilityConfig::basemortupdate = 1.0f;

    // Decay of post-infection immunity
    bool SusceptibilityConfig::enable_immune_decay = false;
    float SusceptibilityConfig::acqdecayrate       = 0.0f;
    float SusceptibilityConfig::trandecayrate      = 0.0f;
    float SusceptibilityConfig::mortdecayrate      = 0.0f;
    float SusceptibilityConfig::baseacqoffset      = 0.0f;
    float SusceptibilityConfig::basetranoffset     = 0.0f;
    float SusceptibilityConfig::basemortoffset     = 0.0f;

    GET_SCHEMA_STATIC_WRAPPER_IMPL(Susceptibility,SusceptibilityConfig)
    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityConfig)
    END_QUERY_INTERFACE_BODY(SusceptibilityConfig)

    bool SusceptibilityConfig::Configure(const Configuration* config)
    {
        // Infection derived immunity
        // Maximum values were revised down to 1.0 from 1000.0. Values > 1.0 imply greater than 
        // baseline susceptibility post infection, which is almost certainly not the desired behavior.
        // If that IS the desired behavior, then consider a disease specific build.
        initConfigTypeMap( "Post_Infection_Acquisition_Multiplier",  &baseacqupdate,  Post_Infection_Acquisition_Multiplier_DESC_TEXT,  0.0f, 1.0f, 0.0f, "Enable_Immunity" );
        initConfigTypeMap( "Post_Infection_Transmission_Multiplier", &basetranupdate, Post_Infection_Transmission_Multiplier_DESC_TEXT, 0.0f, 1.0f, 0.0f, "Enable_Immunity" );
        initConfigTypeMap( "Post_Infection_Mortality_Multiplier",    &basemortupdate, Post_Infection_Mortality_Multiplier_DESC_TEXT,    0.0f, 1.0f, 0.0f, "Enable_Immunity" );

        // Decay of infection derived immunity
        // Maximum values were revised down to 1.0 from 1000.0. Values > 1.0 imply waning rate >100%/day.
        // If that IS the desired behavior, consider turning off immunity.
        const std::map<std::string, std::string> dset_immundecay01  {{"Simulation_Type", "GENERIC_SIM,ENVIRONMENTAL_SIM,POLIO_SIM,TYPHOID_SIM,STI_SIM,AIRBORNE_SIM,TBHIV_SIM,VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,PY_SIM"}, {"Enable_Immunity", "1"}};
        const std::map<std::string, std::string> dset_immundecay02  {{"Simulation_Type", "GENERIC_SIM,ENVIRONMENTAL_SIM,POLIO_SIM,TYPHOID_SIM,STI_SIM,AIRBORNE_SIM,TBHIV_SIM,VECTOR_SIM,MALARIA_SIM,DENGUE_SIM,PY_SIM"}, {"Enable_Immunity", "1"}, {"Enable_Immune_Decay", "1"}};

        initConfigTypeMap( "Enable_Immune_Decay", &enable_immune_decay, Enable_Immune_Decay_DESC_TEXT, true, nullptr, nullptr, &dset_immundecay01);
        initConfigTypeMap( "Acquisition_Blocking_Immunity_Decay_Rate",  &acqdecayrate,  Acquisition_Blocking_Immunity_Decay_Rate_DESC_TEXT,  0.0f, 1.0f, 0.001f, nullptr, nullptr, &dset_immundecay02);
        initConfigTypeMap( "Transmission_Blocking_Immunity_Decay_Rate", &trandecayrate, Transmission_Blocking_Immunity_Decay_Rate_DESC_TEXT, 0.0f, 1.0f, 0.001f, nullptr, nullptr, &dset_immundecay02);
        initConfigTypeMap( "Mortality_Blocking_Immunity_Decay_Rate",    &mortdecayrate, Mortality_Blocking_Immunity_Decay_Rate_DESC_TEXT,    0.0f, 1.0f, 0.001f, nullptr, nullptr, &dset_immundecay02);
        initConfigTypeMap( "Acquisition_Blocking_Immunity_Duration_Before_Decay",  &baseacqoffset,  Acquisition_Blocking_Immunity_Duration_Before_Decay_DESC_TEXT,  0.0f, MAX_HUMAN_LIFETIME, 0.0f, nullptr, nullptr, &dset_immundecay02);
        initConfigTypeMap( "Transmission_Blocking_Immunity_Duration_Before_Decay", &basetranoffset, Transmission_Blocking_Immunity_Duration_Before_Decay_DESC_TEXT, 0.0f, MAX_HUMAN_LIFETIME, 0.0f, nullptr, nullptr, &dset_immundecay02);
        initConfigTypeMap( "Mortality_Blocking_Immunity_Duration_Before_Decay",    &basemortoffset, Mortality_Blocking_Immunity_Duration_Before_Decay_DESC_TEXT,    0.0f, MAX_HUMAN_LIFETIME, 0.0f, nullptr, nullptr, &dset_immundecay02);

        bool bRet = JsonConfigurable::Configure( config );

        LOG_DEBUG_F( "baseacqoffset = %f\n",  baseacqoffset );
        LOG_DEBUG_F( "basetranoffset = %f\n", basetranoffset );
        LOG_DEBUG_F( "basemortoffset = %f\n", basemortoffset );

        LOG_DEBUG_F( "immune_decay = %d\n", enable_immune_decay );
        LOG_DEBUG_F( "acqdecayrate= %f\n",   acqdecayrate );
        LOG_DEBUG_F( "trandecayrate = %f\n", trandecayrate );
        LOG_DEBUG_F( "mortdecayrate = %f\n", mortdecayrate );
        LOG_DEBUG_F( "baseacqupdate = %f\n",  baseacqupdate );
        LOG_DEBUG_F( "basetranupdate = %f\n", basetranupdate );
        LOG_DEBUG_F( "basemortupdate = %f\n", basemortupdate );

        return bRet;
    }

    // QI stuff in case we want to use it more extensively
    BEGIN_QUERY_INTERFACE_BODY(Susceptibility)
        HANDLE_INTERFACE(ISusceptibilityContext)
        HANDLE_ISUPPORTS_VIA(ISusceptibilityContext)
    END_QUERY_INTERFACE_BODY(Susceptibility)

    Susceptibility::Susceptibility()
        : mod_acquire( 0.0f )
        , mod_transmit( 0.0f )
        , mod_mortality( 0.0f )
        , acqdecayoffset( 0.0f )
        , trandecayoffset( 0.0f )
        , mortdecayoffset( 0.0f )
        , m_demographic_risk( 0.0f )
        , effect_mat_acquire(nullptr)
        , parent(nullptr)
    { }

    Susceptibility::Susceptibility(IIndividualHumanContext* context)
        : mod_acquire( 0.0f )
        , mod_transmit( 0.0f )
        , mod_mortality( 0.0f )
        , acqdecayoffset( 0.0f )
        , trandecayoffset( 0.0f )
        , mortdecayoffset( 0.0f )
        , m_demographic_risk( 0.0f )
        , effect_mat_acquire(nullptr)
        , parent(context)
    { }

    Susceptibility::~Susceptibility()
    {
        delete effect_mat_acquire;

        effect_mat_acquire = nullptr;
    }

    Susceptibility* Susceptibility::CreateSusceptibility(IIndividualHumanContext* context, float immmod, float riskmod)
    {
        Susceptibility* newsusceptibility = _new_ Susceptibility(context);
        newsusceptibility->Initialize(immmod, riskmod);

        return newsusceptibility;
    }

    void Susceptibility::Initialize(float immmod, float riskmod)
    {
        // immune modifiers
        mod_acquire   = immmod;
        mod_transmit  = 1.0f;
        mod_mortality = 1.0f;

        // risk modifier
        m_demographic_risk = riskmod;

        // Maternal immunity; waning effect is initialized by SetContextTo
        effect_mat_acquire = parent->GetParams()->effect_mat_acquire->Clone();
        effect_mat_acquire->SetContextTo(parent);
        effect_mat_acquire->Update(parent->GetAge());
        if(effect_mat_acquire->Expired())
        {
            delete effect_mat_acquire;
            effect_mat_acquire = nullptr;
        }
    }

    void Susceptibility::SetContextTo(IIndividualHumanContext* context)
    {
        parent = context;
        if(effect_mat_acquire)
        {
            effect_mat_acquire->SetContextTo(context);
        }
    }

    IIndividualHumanContext* Susceptibility::GetParent()
    {
        return parent;
    }

    float Susceptibility::getModAcquire() const
    {
        float acq_factor = mod_acquire;
        if(effect_mat_acquire)
        {
            acq_factor *= (1.0f - effect_mat_acquire->Current());
        }
 
        LOG_VALID_F("id = %d, age = %f, mod_acquire = %f\n", parent->GetSuid().data, parent->GetAge(), acq_factor);

        return acq_factor;
    }

    float Susceptibility::getModTransmit() const
    {
        return mod_transmit;
    }

    float Susceptibility::getModMortality() const
    {
        return mod_mortality;
    }

    float Susceptibility::getModRisk() const
    {
        return m_demographic_risk;
    }

    void Susceptibility::Update( float dt )
    {
        if(effect_mat_acquire)
        {
            effect_mat_acquire->Update(dt);
            if(effect_mat_acquire->Expired())
            {
                delete effect_mat_acquire;
                effect_mat_acquire = nullptr;
            }
        }

        // Immunity decay calculations
        // Logic was revised to eliminate oscillations and ensure decay works correctly even if
        // for mod_XX > 1.0. (As of Feb2018, no simulations are able to specify mod_XX > 1.0)
        if(SusceptibilityConfig::enable_immune_decay)
        {
            // Acquisition immunity decay
            if (acqdecayoffset > 0.0f)
            {
                acqdecayoffset -= dt;
            }
            else
            {
                float net_change = SusceptibilityConfig::acqdecayrate * dt;
                mod_acquire += (1.0f - mod_acquire) * (net_change < 1.0f ? net_change : 1.0f);
            }

            // Transmission immunity decay
            if (trandecayoffset > 0.0f)
            {
                trandecayoffset -= dt;
            }
            else
            {
                float net_change = SusceptibilityConfig::trandecayrate * dt;
                mod_transmit += (1.0f - mod_transmit) * (net_change < 1.0f ? net_change : 1.0f);
            }

            // Disease mortality immunity decay
            if (mortdecayoffset > 0.0f)
            {
                mortdecayoffset -= dt;
            }
            else
            {
                float net_change = SusceptibilityConfig::mortdecayrate * dt;
                mod_mortality += (1.0f - mod_mortality) * (net_change < 1.0f ? net_change : 1.0f);
            }
        }
    }

    void Susceptibility::UpdateInfectionCleared()
    {
        mod_acquire   *= SusceptibilityConfig::baseacqupdate;
        mod_transmit  *= SusceptibilityConfig::basetranupdate;
        mod_mortality *= SusceptibilityConfig::basemortupdate;

        acqdecayoffset  = SusceptibilityConfig::baseacqoffset;
        trandecayoffset = SusceptibilityConfig::basetranoffset;
        mortdecayoffset = SusceptibilityConfig::basemortoffset;
    }

    bool Susceptibility::HasMaternalImmunity() const
    {
        // Used in MC down-sampling logic
        return (effect_mat_acquire ? true : false);
    }

    bool Susceptibility::IsImmune() const
    {
        // Overriden as needed (used by TB)
        release_assert(false);
        return false;
    }

    void Susceptibility::InitNewInfection()
    {
        // Overriden as needed (used by TB)
        release_assert(false);
    }

    REGISTER_SERIALIZABLE(Susceptibility);

    void Susceptibility::serialize(IArchive& ar, Susceptibility* obj)
    {
        Susceptibility& susceptibility = *obj;

        ar.labelElement("mod_acquire")                & susceptibility.mod_acquire;
        ar.labelElement("mod_transmit")               & susceptibility.mod_transmit;
        ar.labelElement("mod_mortality")              & susceptibility.mod_mortality;

        ar.labelElement("acqdecayoffset")             & susceptibility.acqdecayoffset;
        ar.labelElement("trandecayoffset")            & susceptibility.trandecayoffset;
        ar.labelElement("mortdecayoffset")            & susceptibility.mortdecayoffset;

        ar.labelElement("m_demographic_risk")         & susceptibility.m_demographic_risk;

        ar.labelElement("effect_mat_acquire")         & susceptibility.effect_mat_acquire;
    }
}
