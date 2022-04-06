/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Ivermectin.h"

#include <typeinfo>

#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainerContexts.h"
#include "IIndividualHumanContext.h"

SETUP_LOGGING( "Ivermectin" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(Ivermectin)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(Ivermectin)

    IMPLEMENT_FACTORY_REGISTERED(Ivermectin)

    Ivermectin::Ivermectin()
    : BaseIntervention()
    , killing_effect(nullptr)
    , m_pIVIES(nullptr)
    {
        initSimTypes( 2, "VECTOR_SIM", "MALARIA_SIM" );
        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, IVM_Cost_To_Consumer_DESC_TEXT, 0, 999999, 8.0);
    }

    Ivermectin::Ivermectin( const Ivermectin& master )
    : BaseIntervention( master )
    , killing_effect( nullptr )
    , m_pIVIES( nullptr )
    {
        if( master.killing_effect != nullptr )
        {
            killing_effect = master.killing_effect->Clone();
        }
    }

    Ivermectin::~Ivermectin()
    {
        delete killing_effect;
    }
    bool Ivermectin::Configure( const Configuration * inputJson )
    {
        WaningConfig killing_config;

        initConfigComplexType("Killing_Config",  &killing_config, IVM_Killing_Config_DESC_TEXT );

        bool configured = BaseIntervention::Configure( inputJson );
        if( !JsonConfigurable::_dryrun && configured )
        {
            killing_effect = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json, inputJson->GetDataLocation(), "Killing_Config" );
        }
        return configured;
    }

    bool Ivermectin::Distribute( IIndividualHumanInterventionsContext *context,
                                 ICampaignCostObserver * const pCCO )
    {
        if( AbortDueToDisqualifyingInterventionStatus( context->GetParent() ) )
        {
            return false;
        }
        context->PurgeExisting( typeid(*this).name() );

        bool distributed = BaseIntervention::Distribute( context, pCCO );
        if( distributed )
        {
            if (s_OK != context->QueryInterface(GET_IID(IVectorInterventionEffectsSetter), (void**)&m_pIVIES) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__,
                                               "context",
                                               "IVectorInterventionEffectsSetter",
                                               "IIndividualHumanInterventionsContext" );
            }
        }
        return distributed;
    }

    void Ivermectin::SetContextTo( IIndividualHumanContext *context )
    {
        BaseIntervention::SetContextTo( context );
        killing_effect->SetContextTo( context );

        LOG_DEBUG("Ivermectin::SetContextTo (probably deserializing)\n");
        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IVectorInterventionEffectsSetter), (void**)&m_pIVIES) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__,
                                           "context",
                                           "IVectorInterventionEffectsSetter",
                                           "IIndividualHumanContext" );
        }
    }

    void Ivermectin::Update( float dt )
    {
        if( !BaseIntervention::UpdateIndividualsInterventionStatus() ) return;

        killing_effect->Update(dt);

        float killing = killing_effect->Current();

        m_pIVIES->UpdateInsecticidalDrugKillingProbability( killing );

        // Discard if efficacy is sufficiently low
        if (killing < 1e-5)
        {
            expired = true;
        }
    }



    REGISTER_SERIALIZABLE(Ivermectin);

    void Ivermectin::serialize(IArchive& ar, Ivermectin* obj)
    {
        BaseIntervention::serialize( ar, obj );
        Ivermectin& ivermectin = *obj;
        ar.labelElement("killing_effect") & ivermectin.killing_effect;
    }
}
