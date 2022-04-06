/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "HousingModification.h"

#include <typeinfo>

#include "IIndividualHumanContext.h"
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainerContexts.h"  // for IHousingModificationConsumer methods
#include "Log.h"

SETUP_LOGGING( "SimpleHousingModification" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(SimpleHousingModification)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleHousingModification)

    IMPLEMENT_FACTORY_REGISTERED(SimpleHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(IRSHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(ScreeningHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(SpatialRepellentHousingModification)

    REGISTER_SERIALIZABLE(SimpleHousingModification);
    REGISTER_SERIALIZABLE(IRSHousingModification);
    REGISTER_SERIALIZABLE(ScreeningHousingModification);
    REGISTER_SERIALIZABLE(SpatialRepellentHousingModification);

    // ------------------------------------------------------------------------
    // --- SimpleHousingModification
    // ------------------------------------------------------------------------

    SimpleHousingModification::SimpleHousingModification()
    : BaseIntervention()
    , blocking_effect(nullptr)
    , killing_effect(nullptr)
    , m_pIHMC(nullptr)
    {
        initSimTypes( 2, "VECTOR_SIM", "MALARIA_SIM" );
        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, HM_Cost_To_Consumer_DESC_TEXT, 0, 999999, 8.0);
    }

    SimpleHousingModification::SimpleHousingModification( const SimpleHousingModification& master )
    : BaseIntervention( master )
    , blocking_effect( nullptr )
    , killing_effect( nullptr )
    , m_pIHMC( nullptr )
    {
        if( master.blocking_effect != nullptr )
        {
            blocking_effect = master.blocking_effect->Clone();
        }
        if( master.killing_effect != nullptr )
        {
            killing_effect = master.killing_effect->Clone();
        }
    }

    SimpleHousingModification::~SimpleHousingModification()
    {
        delete blocking_effect;
        delete killing_effect;
    }

    bool SimpleHousingModification::Configure( const Configuration * inputJson )
    {
        WaningConfig repelling_config;
        WaningConfig killing_config;

        initConfigRepelling( &repelling_config );
        initConfigKilling( &killing_config );
        bool configured = BaseIntervention::Configure( inputJson );
        if( !JsonConfigurable::_dryrun && configured )
        {
            blocking_effect = WaningEffectFactory::getInstance()->CreateInstance( repelling_config._json, inputJson->GetDataLocation(), "Blocking_Config");
            killing_effect  = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json,   inputJson->GetDataLocation(), "Killing_Config");
        }
        return configured;
    }



    void SimpleHousingModification::initConfigRepelling( WaningConfig* pRepellingConfig )
    {
        initConfigComplexType( "Blocking_Config", pRepellingConfig, HM_Blocking_Config_DESC_TEXT );
    }

    void SimpleHousingModification::initConfigKilling( WaningConfig* pKillingConfig )
    {
        initConfigComplexType( "Killing_Config", pKillingConfig, HM_Killing_Config_DESC_TEXT );
    }

    bool
    SimpleHousingModification::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * const pCCO
    )
    {
        if( AbortDueToDisqualifyingInterventionStatus( context->GetParent() ) )
        {
            return false;
        }

        if (s_OK != context->QueryInterface(GET_IID(IHousingModificationConsumer), (void**)&m_pIHMC) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IHousingModificationConsumer", "IIndividualHumanInterventionsContext" );
        }

        context->PurgeExisting( typeid(*this).name() );

        return BaseIntervention::Distribute( context, pCCO );
    }

    void SimpleHousingModification::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        BaseIntervention::SetContextTo( context );
        if( blocking_effect != nullptr )
        {
            blocking_effect->SetContextTo( context );
        }
        if( killing_effect != nullptr )
        {
            killing_effect->SetContextTo( context );
        }

        LOG_DEBUG("SimpleHousingModification::SetContextTo (probably deserializing)\n");
        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IHousingModificationConsumer), (void**)&m_pIHMC) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IHousingModificationConsumer", "IIndividualHumanContext" );
        }
    }

    void SimpleHousingModification::Update( float dt )
    {
        if( !BaseIntervention::UpdateIndividualsInterventionStatus() ) return;

        blocking_effect->Update(dt);
        killing_effect->Update(dt);

        ApplyEffectsRepelling( dt );
        ApplyEffectsKilling( dt );
    }

    void SimpleHousingModification::ApplyEffectsRepelling( float dt )
    {
        float current_repellingrate = blocking_effect->Current();

        release_assert( m_pIHMC != nullptr );

        m_pIHMC->ApplyHouseBlockingProbability( current_repellingrate );
    }

    void SimpleHousingModification::ApplyEffectsKilling( float dt )
    {
        float current_killingrate = killing_effect->Current();

        release_assert( m_pIHMC != nullptr );

        m_pIHMC->UpdateProbabilityOfScreenKilling( current_killingrate );
    }
    void SimpleHousingModification::serialize(IArchive& ar, SimpleHousingModification* obj)
    {
        BaseIntervention::serialize( ar, obj );
        SimpleHousingModification& mod = *obj;
        ar.labelElement("blocking_effect") & mod.blocking_effect;
        ar.labelElement("killing_effect") & mod.killing_effect;
    }

    // ------------------------------------------------------------------------
    // --- IRSHousingModification
    // ------------------------------------------------------------------------

    void IRSHousingModification::serialize(IArchive& ar, IRSHousingModification* obj)
    {
        SimpleHousingModification::serialize(ar, obj);
    }

    // ------------------------------------------------------------------------
    // --- ScreeningHousingModification
    // ------------------------------------------------------------------------

    void ScreeningHousingModification::serialize(IArchive& ar, ScreeningHousingModification* obj)
    {
        SimpleHousingModification::serialize(ar, obj);
    }

    // ------------------------------------------------------------------------
    // --- SpatialRepellentHousingModification
    // ------------------------------------------------------------------------

    void SpatialRepellentHousingModification::initConfigKilling( WaningConfig* pKillingConfig )
    {
        // do not include killing
    }

    void SpatialRepellentHousingModification::ApplyEffectsKilling( float dt )
    {
        // no killing
    }

    void SpatialRepellentHousingModification::serialize(IArchive& ar, SpatialRepellentHousingModification* obj)
    {
        SimpleHousingModification::serialize(ar, obj);
    }
}

