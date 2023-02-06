/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "HumanHostSeekingTrap.h"

#include <typeinfo>

#include "IIndividualHumanContext.h"
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainerContexts.h"  // for IVectorInterventionEffectsSetter methods

SETUP_LOGGING( "HumanHostSeekingTrap" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(HumanHostSeekingTrap)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(HumanHostSeekingTrap)

    IMPLEMENT_FACTORY_REGISTERED(HumanHostSeekingTrap)

    HumanHostSeekingTrap::HumanHostSeekingTrap()
        : BaseIntervention()
        , killing_effect(nullptr)
        , attract_effect(nullptr)
        , ivies(nullptr)
    {
        initSimTypes( 2, "VECTOR_SIM", "MALARIA_SIM" );
    }

    HumanHostSeekingTrap::HumanHostSeekingTrap( const HumanHostSeekingTrap& master )
        : BaseIntervention( master )
        , killing_effect( nullptr )
        , attract_effect( nullptr )
        , ivies( nullptr )
    {
        if(master.killing_effect)
        {
            killing_effect = master.killing_effect->Clone();
        }
        if(master.attract_effect)
        {
            attract_effect = master.attract_effect->Clone();
        }
    }

    HumanHostSeekingTrap::~HumanHostSeekingTrap()
    {
        delete killing_effect;
        delete attract_effect;

        killing_effect = nullptr;
        attract_effect = nullptr;
    }

    bool HumanHostSeekingTrap::Configure(const Configuration* inputJson)
    {
        initConfigTypeMap( "Cost_To_Consumer", &cost_per_unit, HST_Cost_To_Consumer_DESC_TEXT, 0, 999999, 3.75 );

        killing_effect = WaningEffectFactory::CreateInstance();
        attract_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",  killing_effect->GetConfigurable(),  HST_Killing_Config_DESC_TEXT);
        initConfigTypeMap("Attract_Config",  attract_effect->GetConfigurable(),  HST_Attract_Config_DESC_TEXT);

        bool configured = BaseIntervention::Configure( inputJson );

        return configured;
    }

    bool HumanHostSeekingTrap::Distribute(IIndividualHumanInterventionsContext* context, ICampaignCostObserver* const pCCO)
    {
        if( AbortDueToDisqualifyingInterventionStatus( context->GetParent() ) )
        {
            return false;
        }
        context->PurgeExisting( typeid(*this).name() );

        bool distributed = BaseIntervention::Distribute( context, pCCO );
        if( distributed )
        {
            if (s_OK != context->QueryInterface(GET_IID(IVectorInterventionEffectsSetter), (void**)&ivies) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IVectorInterventionEffectsSetter", "IIndividualHumanInterventionsContext" );
            }
        }
        return distributed;
    }

    void HumanHostSeekingTrap::Update( float dt )
    {
        if( !BaseIntervention::UpdateIndividualsInterventionStatus() ) return;

        // Effects of human host-seeking trap are updated with indoor-home artificial-diet interfaces in VectorInterventionsContainer::Update.
        // Attraction rate diverts indoor feeding attempts from humans to trap; killing rate kills a fraction of diverted feeding attempts.
        if(killing_effect)
        {
            killing_effect->Update(dt);
            if(killing_effect->Expired())
            {
                delete killing_effect;
                killing_effect = nullptr;
            }
            else
            {
                ivies->UpdateArtificialDietKillingRate( killing_effect->Current() );
            }
        }

        if(attract_effect)
        {
            attract_effect->Update(dt);
            if(attract_effect->Expired())
            {
                delete attract_effect;
                attract_effect = nullptr;
            }
            else
            {
                ivies->UpdateArtificialDietAttractionRate( attract_effect->Current() );
            }
        }
    }

    void HumanHostSeekingTrap::SetContextTo(IIndividualHumanContext *context)
    {
        BaseIntervention::SetContextTo( context );

        if(killing_effect)
        {
            killing_effect->SetContextTo( context );
        }
        if(attract_effect)
        {
            attract_effect->SetContextTo( context );
        }

        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IVectorInterventionEffectsSetter), (void**)&ivies) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IVectorInterventionEffectsSetter", "IIndividualHumanContext" );
        }
    }

    REGISTER_SERIALIZABLE(HumanHostSeekingTrap);

    void HumanHostSeekingTrap::serialize(IArchive& ar, HumanHostSeekingTrap* obj)
    {
        BaseIntervention::serialize( ar, obj );
        HumanHostSeekingTrap& trap = *obj;
        ar.labelElement("attract_effect") & trap.attract_effect;
        ar.labelElement("killing_effect") & trap.killing_effect;
    }
}

