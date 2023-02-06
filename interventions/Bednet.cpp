/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Bednet.h"

#include <typeinfo>

#include "IIndividualHumanContext.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainerContexts.h"  // for IBednetConsumer methods
#include "Log.h"
#include "IndividualEventContext.h"
#include "NodeEventContext.h"

SETUP_LOGGING( "SimpleBednet" )

namespace Kernel
{
    // ------------------------------------------------------------------------
    // --- AbstractBednet
    // ------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY( AbstractBednet )
        HANDLE_INTERFACE( IConfigurable )
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY( AbstractBednet )

    AbstractBednet::AbstractBednet()
        : BaseIntervention()
        , m_pEffectKilling( nullptr )
        , m_pEffectBlocking( nullptr )
        , m_pConsumer( nullptr )
    {
        initSimTypes( 2, "MALARIA_SIM", "VECTOR_SIM" );
    }

    AbstractBednet::AbstractBednet( const AbstractBednet& master )
        : BaseIntervention( master )
        , m_pEffectKilling( nullptr )
        , m_pEffectBlocking( nullptr )
        , m_pConsumer( nullptr )
    {
        if(master.m_pEffectKilling)
        {
            m_pEffectKilling = master.m_pEffectKilling->Clone();
        }
        if(master.m_pEffectBlocking)
        {
            m_pEffectBlocking = master.m_pEffectBlocking->Clone();
        }
    }

    AbstractBednet::~AbstractBednet()
    {
        delete m_pEffectKilling;
        delete m_pEffectBlocking;

        m_pEffectKilling = nullptr;
        m_pEffectBlocking = nullptr;
    }

    bool AbstractBednet::Configure(const Configuration* inputJson)
    {
        bool configured = true;

        initConfigTypeMap( "Cost_To_Consumer", &cost_per_unit, SB_Cost_To_Consumer_DESC_TEXT, 0, 999999, 3.75 );

        if( configured || JsonConfigurable::_dryrun )
        {
            configured = ConfigureBlockingAndKilling( inputJson );
        }
        if( configured || JsonConfigurable::_dryrun )
        {
            configured = ConfigureUsage( inputJson );
        }
        if( configured || JsonConfigurable::_dryrun )
        {
            configured = ConfigureEvents( inputJson );
        }

        return configured;
    }

    bool AbstractBednet::ConfigureBlockingAndKilling(const Configuration* inputJson)
    {
        m_pEffectKilling  = WaningEffectFactory::CreateInstance();
        m_pEffectBlocking = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",    m_pEffectKilling->GetConfigurable(),    SB_Killing_Config_DESC_TEXT);
        initConfigTypeMap("Blocking_Config",   m_pEffectBlocking->GetConfigurable(),   SB_Blocking_Config_DESC_TEXT);

        bool configured = BaseIntervention::Configure( inputJson );

        return configured;
    }

    bool AbstractBednet::Distribute(IIndividualHumanInterventionsContext* context, ICampaignCostObserver* const pCCO )
    {
        if( AbortDueToDisqualifyingInterventionStatus( context->GetParent() ) )
        {
            return false;
        }

        std::list<IDistributableIntervention*> net_list = context->GetInterventionsByName( GetName() );
        for( IDistributableIntervention* p_bednet : net_list )
        {
            p_bednet->SetExpired( true );
        }

        bool distributed = BaseIntervention::Distribute( context, pCCO );
        if( distributed )
        {
            SetContextTo( context->GetParent() );
        }
        return distributed;
    }

    void AbstractBednet::Update( float dt )
    {
        if( Expired() ) return;

        if( !BaseIntervention::UpdateIndividualsInterventionStatus() ) return;

        UpdateUsage( dt );
        UpdateBlockingAndKilling( dt );

        if( IsUsingBednet() )
        {
            UseBednet();
        }

        if( CheckExpiration( dt ) )
        {
            SetExpired( true );
        }
    }

    float AbstractBednet::GetEffectKilling() const
    {
        float effect = 0.0f;
        if(m_pEffectKilling)
        {
            effect = m_pEffectKilling->Current();
        }
        return effect;
    }

    float AbstractBednet::GetEffectBlocking() const
    {
        float effect = 0.0f;
        if(m_pEffectBlocking)
        {
            effect = m_pEffectBlocking->Current();
        }
        return effect;
    }

    void AbstractBednet::UseBednet()
    {
        if(m_pEffectKilling)
        {
            m_pConsumer->UpdateProbabilityOfKilling( GetEffectKilling() );
        }

        if(m_pEffectBlocking)
        {
            m_pConsumer->UpdateProbabilityOfBlocking( GetEffectBlocking()  );
        }
    }

    void AbstractBednet::UpdateBlockingAndKilling( float dt )
    {
        if(m_pEffectKilling)
        {
            m_pEffectKilling->Update( dt );
            if(m_pEffectKilling->Expired())
            {
                delete m_pEffectKilling;
                m_pEffectKilling = nullptr;
            }
        }
        if(m_pEffectBlocking)
        {
            m_pEffectBlocking->Update( dt );
            if(m_pEffectBlocking->Expired())
            {
                delete m_pEffectBlocking;
                m_pEffectBlocking = nullptr;
            }
        }
    }

    void AbstractBednet::SetContextTo( IIndividualHumanContext *context )
    {
        BaseIntervention::SetContextTo( context );

        if(m_pEffectKilling)
        {
            m_pEffectKilling->SetContextTo( context );
        }
        if(m_pEffectBlocking)
        {
            m_pEffectBlocking->SetContextTo( context );
        }

        if( s_OK != context->GetInterventionsContext()->QueryInterface( GET_IID( IBednetConsumer ), (void**)&m_pConsumer ) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IBednetConsumer", "IIndividualHumanContext" );
        }
    }

    void AbstractBednet::BroadcastEvent( const EventTrigger::Enum& trigger ) const
    {
        if( trigger != EventTrigger::NoTrigger )
        {
            IIndividualEventBroadcaster* broadcaster = parent->GetEventContext()->GetNodeEventContext()->GetIndividualEventBroadcaster();
            broadcaster->TriggerObservers( parent->GetEventContext(), trigger );
        }
    }

    void AbstractBednet::serialize( IArchive& ar, AbstractBednet* obj )
    {
        BaseIntervention::serialize( ar, obj );
        AbstractBednet& bednet = *obj;
        ar.labelElement( "m_pEffectBlocking" ) & bednet.m_pEffectBlocking;
        ar.labelElement( "m_pEffectKilling" ) & bednet.m_pEffectKilling;
    }

    // ------------------------------------------------------------------------
    // --- SimpleBednet
    // ------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_DERIVED( SimpleBednet, AbstractBednet )
    END_QUERY_INTERFACE_DERIVED( SimpleBednet, AbstractBednet )

    IMPLEMENT_FACTORY_REGISTERED(SimpleBednet)
    
    SimpleBednet::SimpleBednet()
        : AbstractBednet()
        , m_pEffectUsage( nullptr )
    {
    }

    SimpleBednet::SimpleBednet( const SimpleBednet& master )
        : AbstractBednet( master )
        , m_pEffectUsage( nullptr )
    {
        if(master.m_pEffectUsage)
        {
            m_pEffectUsage = master.m_pEffectUsage->Clone();
        }
    }

    SimpleBednet::~SimpleBednet()
    {
        delete m_pEffectUsage;
        m_pEffectUsage = nullptr;
    }

    bool SimpleBednet::ConfigureUsage( const Configuration * inputJson )
    {
        m_pEffectUsage = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Usage_Config",  m_pEffectUsage->GetConfigurable(),  SB_Usage_Config_DESC_TEXT);

        bool configured = JsonConfigurable::Configure( inputJson ); // AbstractBednet is responsible for calling BaseIntervention::Configure()

        return configured;
    }

    bool SimpleBednet::IsUsingBednet() const
    {
        // -----------------------------------------------------------------------------------
        // --- true because we use the usage effect to adjust the killing and blocking effects
        // -----------------------------------------------------------------------------------
        return true;
    }

    void SimpleBednet::UpdateUsage( float dt )
    {
        if(m_pEffectUsage)
        {
            m_pEffectUsage->Update( dt );
            if(m_pEffectUsage->Expired())
            {
                delete m_pEffectUsage;
                m_pEffectUsage = nullptr;
            }
        }
    }

    float SimpleBednet::GetEffectKilling() const
    {
        return AbstractBednet::GetEffectKilling() * GetEffectUsage();
    }

    float SimpleBednet::GetEffectBlocking() const
    {
        return AbstractBednet::GetEffectBlocking() * GetEffectUsage();
    }

    float SimpleBednet::GetEffectUsage() const
    {
        float effect = 0.0f;
        if(m_pEffectUsage)
        {
            effect = m_pEffectUsage->Current();
        }
        return effect;
    }

    bool SimpleBednet::CheckExpiration( float dt )
    {
        bool is_expired = true;
        if(m_pEffectUsage)
        {
            is_expired = m_pEffectUsage->Expired();
        }
        return is_expired;
    }

    void SimpleBednet::SetContextTo( IIndividualHumanContext *context )
    {
        AbstractBednet::SetContextTo( context );

        if(m_pEffectUsage)
        {
            m_pEffectUsage->SetContextTo( context );
        }
    }

    REGISTER_SERIALIZABLE(SimpleBednet);

    void SimpleBednet::serialize(IArchive& ar, SimpleBednet* obj)
    {
        AbstractBednet::serialize( ar, obj );
        SimpleBednet& bednet = *obj;
        ar.labelElement("m_pEffectUsage") & bednet.m_pEffectUsage;
    }
}
