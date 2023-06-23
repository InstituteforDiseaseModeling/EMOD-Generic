/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Vaccine.h"

#include "Common.h"
#include "IIndividualHumanContext.h"
#include "InterventionsContainer.h"
#include "ISusceptibilityContext.h"
#include "IndividualEventContext.h"
#include "RANDOM.h"

SETUP_LOGGING("Vaccine")

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(Vaccine)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(Vaccine)

    IMPLEMENT_FACTORY_REGISTERED(Vaccine)

    REGISTER_SERIALIZABLE(Vaccine);

    Vaccine::Vaccine() 
        : BaseIntervention()
        , vaccine_take(0.0f)
        , vaccine_took(false)
        , frac_acq_blocking_take(0.0f)
        , vax_route(IVRoute::ALL)
        , take_by_age_map(0.0f,FLT_MAX,0.0f,1.0f)
        , init_acq_by_effect_map(0.0f,1.0f,0.0f,1000.0f)
        , init_trn_by_effect_map(0.0f,1.0f,0.0f,1000.0f)
        , init_mor_by_effect_map(0.0f,1.0f,0.0f,1000.0f)
        , effect_acquire(nullptr)
        , effect_transmit(nullptr)
        , effect_mortality(nullptr)
        , ivc(nullptr)
    {
    }

    Vaccine::Vaccine(const Vaccine& existing_instance)
        : BaseIntervention(existing_instance)
        , vaccine_take(existing_instance.vaccine_take)
        , vaccine_took(existing_instance.vaccine_took)
        , frac_acq_blocking_take(existing_instance.frac_acq_blocking_take)
        , vax_route(existing_instance.vax_route)
        , take_by_age_map(existing_instance.take_by_age_map)
        , init_acq_by_effect_map(existing_instance.init_acq_by_effect_map)
        , init_trn_by_effect_map(existing_instance.init_trn_by_effect_map)
        , init_mor_by_effect_map(existing_instance.init_mor_by_effect_map)
        , effect_acquire(nullptr)
        , effect_transmit(nullptr)
        , effect_mortality(nullptr)
        , ivc(existing_instance.ivc)
    {
        if(existing_instance.effect_acquire)
        {
            effect_acquire   = existing_instance.effect_acquire->Clone();
        }
        if(existing_instance.effect_transmit)
        {
            effect_transmit  = existing_instance.effect_transmit->Clone();
        }
        if(existing_instance.effect_mortality)
        {
            effect_mortality = existing_instance.effect_mortality->Clone();
        }
    }

    Vaccine::~Vaccine()
    {
        delete effect_acquire;
        delete effect_transmit;
        delete effect_mortality;

        effect_acquire   = nullptr;
        effect_transmit  = nullptr;
        effect_mortality = nullptr;
    }

    bool Vaccine::Configure(const Configuration* inputJson)
    {
        initConfig("Vaccine_Route",  vax_route,  inputJson,  MetadataDescriptor::Enum("Vaccine_Route", VAC_Vaccine_Route_DESC_TEXT, MDD_ENUM_ARGS(IVRoute)));

        initConfigTypeMap("Vaccine_Take",                &vaccine_take,                 VAC_Vaccine_Take_DESC_TEXT,                      0.0f,     1.0f,  1.0f);
        initConfigTypeMap("Cost_To_Consumer",            &cost_per_unit,                VAC_Cost_To_Consumer_DESC_TEXT,                  0.0f,  FLT_MAX, 10.0f);
        initConfigTypeMap("Take_Reduced_By_Acquire_Immunity", &frac_acq_blocking_take,  VAC_Take_Reduced_By_Acquire_Immunity_DESC_TEXT,  0.0f,     1.0f,  0.0f);

        initConfigTypeMap("Take_By_Age_Multiplier",                          &take_by_age_map,         VAC_Take_By_Age_Multiplier_DESC_TEXT);
        initConfigTypeMap("Initial_Acquire_By_Current_Effect_Multiplier",    &init_acq_by_effect_map,  VAC_Initial_Acquire_By_Current_Effect_Multiplier_DESC_TEXT);
        initConfigTypeMap("Initial_Transmit_By_Current_Effect_Multiplier",   &init_trn_by_effect_map,  VAC_Initial_Transmit_By_Current_Effect_Multiplier_DESC_TEXT);
        initConfigTypeMap("Initial_Mortality_By_Current_Effect_Multiplier",  &init_mor_by_effect_map,  VAC_Initial_Mortality_By_Current_Effect_Multiplier_DESC_TEXT);

        effect_acquire   = WaningEffectFactory::CreateInstance();
        effect_transmit  = WaningEffectFactory::CreateInstance();
        effect_mortality = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Acquire_Config",    effect_acquire->GetConfigurable(),    VAC_Acquire_Config_DESC_TEXT);
        initConfigTypeMap("Transmit_Config",   effect_transmit->GetConfigurable(),   VAC_Mortality_Config_DESC_TEXT);
        initConfigTypeMap("Mortality_Config",  effect_mortality->GetConfigurable(),  VAC_Transmit_Config_DESC_TEXT);

        bool retVal = BaseIntervention::Configure(inputJson);

        return retVal;
    }

    bool Vaccine::Distribute(IIndividualHumanInterventionsContext* p_ihic, ICampaignCostObserver* const p_cco)
    {
        // Call base distribute first to check eligibility conditions and call SetContextTo
        bool was_distributed = BaseIntervention::Distribute(p_ihic, p_cco);

        if(was_distributed)
        {
            // Vaccine take
            float acq_current = 1.0f - parent->GetVaccineContext()->GetIVReducedAcquire(vax_route)*
                                       parent->GetSusceptibilityContext()->getModAcquire();
            float take_mult   = 1.0f - frac_acq_blocking_take*acq_current;

            if(take_by_age_map.size())
            {
                take_mult *= take_by_age_map.getValueLinearInterpolation(parent->GetAge(), -1.0f);
                if(take_mult < 0.0f)
                {
                    throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__,
                        "Individual age less than minimum in Take_By_Age_Multiplier.");
                }
            }

            vaccine_took = parent->GetRng()->SmartDraw(vaccine_take*take_mult);
            LOG_VALID_F("Vaccine did %stake for individual %d with age %f.\n",
                             (vaccine_took?"":"not "), parent->GetSuid(), parent->GetAge());

            // Vaccine effect multipliers
            if(init_acq_by_effect_map.size() && effect_acquire && !effect_acquire->Expired())
            {
                float acq_current = 1.0f - parent->GetVaccineContext()->GetIVReducedAcquire(vax_route)*
                                           parent->GetSusceptibilityContext()->getModAcquire();
                float acq_mult    = init_acq_by_effect_map.getValueLinearInterpolation(acq_current, -1.0f);
                if(acq_mult < 0.0f)
                {
                    throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__,
                        "Current effect less than minimum in Initial_Acquire_By_Current_Effect_Multiplier.");
                }
                effect_acquire->SetInitial(acq_mult*effect_acquire->GetInitial());
            }
            if(init_trn_by_effect_map.size() && effect_transmit && !effect_transmit->Expired())
            {
                float trn_current = 1.0f - parent->GetVaccineContext()->GetIVReducedTransmit(vax_route)*
                                           parent->GetSusceptibilityContext()->getModTransmit();
                float trn_mult    = init_trn_by_effect_map.getValueLinearInterpolation(trn_current, -1.0f);
                if(trn_mult < 0.0f)
                {
                    throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__,
                        "Current effect less than minimum in Initial_Transmit_By_Current_Effect_Multiplier.");
                }
                effect_transmit->SetInitial(trn_mult*effect_transmit->GetInitial());
            }
            if(init_mor_by_effect_map.size() && effect_mortality && !effect_mortality->Expired())
            {
                float mor_current = 1.0f - parent->GetVaccineContext()->GetIVReducedMortality(vax_route)*
                                           parent->GetSusceptibilityContext()->getModMortality();
                float mor_mult    = init_mor_by_effect_map.getValueLinearInterpolation(mor_current, -1.0f);
                if(mor_mult < 0.0f)
                {
                    throw GeneralConfigurationException(__FILE__, __LINE__, __FUNCTION__,
                             "Current effect less than minimum in Initial_Mortality_By_Current_Effect_Multiplier.");
                }
                effect_mortality->SetInitial(mor_mult*effect_mortality->GetInitial());
            }
        }

        return was_distributed;
    }

    void Vaccine::SetContextTo(IIndividualHumanContext* p_ihc)
    {
        // Sets parent to p_ihc
        BaseIntervention::SetContextTo(p_ihc);

        if(effect_acquire)
        {
            effect_acquire->SetContextTo(p_ihc);
        }
        if(effect_transmit)
        {
            effect_transmit->SetContextTo(p_ihc);
        }
        if(effect_mortality)
        {
            effect_mortality->SetContextTo(p_ihc);
        }

        // Get pointer to agent's invterventions container; use vaccine interface
        ivc = p_ihc->GetVaccineContext();
    }

    void Vaccine::Update(float dt)
    {
        // Should never get here without a valid pointer to agent's interventions container
        release_assert(ivc);

        if(effect_acquire)
        {
            effect_acquire->Update(dt);
            if(effect_acquire->Expired())
            {
                delete effect_acquire;
                effect_acquire = nullptr;
            }
            else if(vaccine_took)
            {
                ivc->UpdateIVAcquireRate(effect_acquire->Current(), vax_route);
            }
        }

        if(effect_transmit)
        {
            effect_transmit->Update(dt);
            if(effect_transmit->Expired())
            {
                delete effect_transmit;
                effect_transmit = nullptr;
            }
            else if(vaccine_took)
            {
                ivc->UpdateIVTransmitRate(effect_transmit->Current(), vax_route);
            }
        }

        if(effect_mortality)
        {
            effect_mortality->Update(dt);
            if(effect_mortality->Expired())
            {
                delete effect_mortality;
                effect_mortality = nullptr;
            }
            else if(vaccine_took)
            {
                ivc->UpdateIVMortalityRate(effect_mortality->Current(), vax_route);
            }

        }

        // Expire if no vaccine effects
        expired = (!effect_acquire && !effect_transmit && !effect_mortality);
    }

    bool Vaccine::NeedsInfectiousLoopUpdate() const
    {
        bool retVal = true;

        // Only needs to return true if no mortality effect
        // Currently returns true always to avoid changing the
        // random number stream compared to (legacy) SimpleVaccine
        //if(!effect_mortality)
        //{
        //    retVal = false;
        //}

        return retVal;
    }

    void Vaccine::serialize(IArchive& ar, Vaccine* obj)
    {
        BaseIntervention::serialize(ar, obj);

        Vaccine& vaccine_obj = *obj;

        ar.labelElement("vaccine_took"              ) & vaccine_obj.vaccine_took;

        ar.labelElement("effect_acquire"            ) & vaccine_obj.effect_acquire;
        ar.labelElement("effect_transmit"           ) & vaccine_obj.effect_transmit;
        ar.labelElement("effect_mortality"          ) & vaccine_obj.effect_mortality;
    }
}
