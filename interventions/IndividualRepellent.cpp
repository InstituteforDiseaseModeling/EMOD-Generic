/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "IndividualRepellent.h"

#include "IIndividualHumanContext.h"
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainerContexts.h"  // for IIndividualRepellentConsumer methods

SETUP_LOGGING( "SimpleIndividualRepellent" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(SimpleIndividualRepellent)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleIndividualRepellent)

    IMPLEMENT_FACTORY_REGISTERED(SimpleIndividualRepellent)

    SimpleIndividualRepellent::SimpleIndividualRepellent()
    : BaseIntervention()
    , blocking_effect(nullptr)
    , m_pIRC(nullptr)
    {
        initSimTypes( 2, "VECTOR_SIM", "MALARIA_SIM" );
    }

    SimpleIndividualRepellent::SimpleIndividualRepellent( const SimpleIndividualRepellent& master )
    : BaseIntervention( master )
    , blocking_effect( nullptr )
    , m_pIRC( nullptr )
    {
        if( master.blocking_effect != nullptr )
        {
            blocking_effect = master.blocking_effect->Clone();
        }
    }

    SimpleIndividualRepellent::~SimpleIndividualRepellent()
    {
        delete blocking_effect;
    }

    bool
    SimpleIndividualRepellent::Configure(
        const Configuration * inputJson
    )
    {
        WaningConfig   blocking_config;

        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, SIR_Cost_To_Consumer_DESC_TEXT, 0, 999999, 8.0);
        initConfigComplexType("Blocking_Config", &blocking_config, SIR_Blocking_Config_DESC_TEXT );

        bool configured = BaseIntervention::Configure( inputJson );

        if( !JsonConfigurable::_dryrun  && configured )
        {
            blocking_effect = WaningEffectFactory::getInstance()->CreateInstance(blocking_config._json, inputJson->GetDataLocation(), "Blocking_Config");
        }
        return configured;
    }

    bool
    SimpleIndividualRepellent::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * const pCCO
    )
    {
        if (s_OK != context->QueryInterface(GET_IID(IIndividualRepellentConsumer), (void**)&m_pIRC) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__,
                                           "context", "IIndividualRepellentConsumer", "IIndividualHumanInterventionsContext" );
        }
        return BaseIntervention::Distribute( context, pCCO );
    }

    void SimpleIndividualRepellent::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        BaseIntervention::SetContextTo( context );
        blocking_effect->SetContextTo( context );

        LOG_DEBUG("SimpleIndividualRepellent::SetContextTo (probably deserializing)\n");
        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IIndividualRepellentConsumer), (void**)&m_pIRC) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__,
                                           "context", "IIndividualRepellentConsumer", "IIndividualHumanContext" );
        }
    }

    void SimpleIndividualRepellent::Update( float dt )
    {
        if( !BaseIntervention::UpdateIndividualsInterventionStatus() ) return;

        blocking_effect->Update(dt);
        float current_blockingrate = blocking_effect->Current();

        release_assert( m_pIRC != nullptr );
        m_pIRC->UpdateProbabilityOfIndRepBlocking( current_blockingrate );
    }

    REGISTER_SERIALIZABLE(SimpleIndividualRepellent);

    void SimpleIndividualRepellent::serialize(IArchive& ar, SimpleIndividualRepellent* obj)
    {
        BaseIntervention::serialize( ar, obj );
        SimpleIndividualRepellent& repellent = *obj;
        ar.labelElement("blocking_effect") & repellent.blocking_effect;
    }
}
