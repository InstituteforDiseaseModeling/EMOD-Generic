/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "UsageDependentBednet.h"
#include "InterventionFactory.h"
#include "Log.h"
#include "IndividualEventContext.h"
#include "IIndividualHumanContext.h"
#include "RANDOM.h"
#include "DistributionFactory.h"

SETUP_LOGGING( "UsageDependentBednet" )

namespace Kernel
{
    // ------------------------------------------------------------------------
    // --- UsageDependentBednet
    // ------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_DERIVED( UsageDependentBednet, AbstractBednet )
    END_QUERY_INTERFACE_DERIVED( UsageDependentBednet, AbstractBednet )

    IMPLEMENT_FACTORY_REGISTERED( UsageDependentBednet )

    UsageDependentBednet::UsageDependentBednet()
        : AbstractBednet()
        , m_pEffectUsage(nullptr)
        , m_TriggerReceived()
        , m_TriggerUsing()
        , m_TriggerDiscard()
        , m_ExpirationDuration(nullptr)
        , m_ExpirationTimer(0.0)
        , m_TimerHasExpired(false)
    {
        m_ExpirationTimer.handle = std::bind( &UsageDependentBednet::Callback, this, std::placeholders::_1 );
    }

    UsageDependentBednet::UsageDependentBednet( const UsageDependentBednet& master )
        : AbstractBednet( master )
        , m_pEffectUsage(nullptr)
        , m_TriggerReceived( master.m_TriggerReceived )
        , m_TriggerUsing( master.m_TriggerUsing )
        , m_TriggerDiscard( master.m_TriggerDiscard )
        , m_ExpirationDuration(nullptr)
        , m_ExpirationTimer(master.m_ExpirationTimer)
        , m_TimerHasExpired(master.m_TimerHasExpired)
    {
        if(master.m_pEffectUsage)
        {
            m_pEffectUsage = master.m_pEffectUsage->Clone();
        }
        if(master.m_ExpirationDuration)
        {
            m_ExpirationDuration = master.m_ExpirationDuration->Clone();
        }
        m_ExpirationTimer.handle = std::bind( &UsageDependentBednet::Callback, this, std::placeholders::_1 );
    }

    UsageDependentBednet::~UsageDependentBednet()
    {
        delete m_pEffectUsage;
        delete m_ExpirationDuration;

        m_pEffectUsage       = nullptr;
        m_ExpirationDuration = nullptr;
    }

    bool UsageDependentBednet::ConfigureUsage( const Configuration * inputJson )
    {
        m_pEffectUsage = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Usage_Config",  m_pEffectUsage->GetConfigurable(),  UDBednet_Usage_Config_List_DESC_TEXT);

        bool configured = JsonConfigurable::Configure( inputJson ); // AbstractBednet is responsible for calling BaseIntervention::Configure()

        return configured;
    }

    bool UsageDependentBednet::ConfigureEvents( const Configuration * inputJson )
    {
        DistributionFunction::Enum expiration_function( DistributionFunction::CONSTANT_DISTRIBUTION );
        initConfig("Expiration_Period_Distribution", expiration_function, inputJson, MetadataDescriptor::Enum("Expiration_Distribution_Type", UDBednet_Expiration_Distribution_Type_DESC_TEXT, MDD_ENUM_ARGS(DistributionFunction)));                 
        m_ExpirationDuration = DistributionFactory::CreateDistribution( this, expiration_function, "Expiration_Period", inputJson );

        initConfig( "Received_Event", m_TriggerReceived, inputJson, MetadataDescriptor::Enum("Received_Event", UDBednet_Received_Event_DESC_TEXT, MDD_ENUM_ARGS( EventTrigger ) ) );
        initConfig( "Using_Event",    m_TriggerUsing,    inputJson, MetadataDescriptor::Enum("Using_Event",    UDBednet_Using_Event_DESC_TEXT,    MDD_ENUM_ARGS( EventTrigger ) ) );
        initConfig( "Discard_Event",  m_TriggerDiscard,  inputJson, MetadataDescriptor::Enum("Discard_Event",  UDBednet_Discard_Event_DESC_TEXT,  MDD_ENUM_ARGS( EventTrigger ) ) );

        return JsonConfigurable::Configure( inputJson ); // AbstractBednet is responsible for calling BaseIntervention::Configure()
    }

    bool UsageDependentBednet::Distribute(IIndividualHumanInterventionsContext* context, ICampaignCostObserver* const pCCO)
    {
        bool distributed = AbstractBednet::Distribute( context, pCCO );
        if( distributed )
        {
            m_ExpirationTimer = m_ExpirationDuration->Calculate( context->GetParent()->GetRng() );
            BroadcastEvent( m_TriggerReceived );

            // ----------------------------------------------------------------------------
            // --- Assuming dt=1.0 and decrementing timer so that a timer of zero expires
            // --- when it is distributed but is not used.  A timer of one should be used
            // --- the day it is distributed but expire:
            // ---    distributed->used->expired on all same day
            // ----------------------------------------------------------------------------
            m_ExpirationTimer.Decrement( 1.0 );
        }
        return distributed;
    }

    bool UsageDependentBednet::IsUsingBednet() const
    {
        float usage_effect = GetEffectUsage();

        // Check expiratin in case it expired when it was distributed
        bool is_using = !m_TimerHasExpired && parent->GetRng()->SmartDraw( usage_effect );

        if( is_using )
        {
            BroadcastEvent( m_TriggerUsing );
        }

        return is_using;
    }

    void UsageDependentBednet::UpdateUsage( float dt )
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

    float UsageDependentBednet::GetEffectUsage() const
    {
        float effect = 0.0f;
        if(m_pEffectUsage)
        {
            effect = m_pEffectUsage->Current();
        }
        return effect;
    }

    bool UsageDependentBednet::CheckExpiration( float dt )
    {
        m_ExpirationTimer.Decrement( dt );

        return m_TimerHasExpired;
    }

    void UsageDependentBednet::Callback( float dt )
    {
        m_TimerHasExpired = true;
    }

    void UsageDependentBednet::SetExpired( bool isExpired )
    {
        AbstractBednet::SetExpired( isExpired );
        if( Expired() )
        {
            BroadcastEvent( m_TriggerDiscard );
        }
    }

    void UsageDependentBednet::SetContextTo( IIndividualHumanContext *context )
    {
        AbstractBednet::SetContextTo( context );
        if(m_pEffectUsage)
        {
            m_pEffectUsage->SetContextTo( context );
        }
    }

    REGISTER_SERIALIZABLE( UsageDependentBednet );

    void UsageDependentBednet::serialize( IArchive& ar, UsageDependentBednet* obj )
    {
        AbstractBednet::serialize( ar, obj );
        UsageDependentBednet& bednet = *obj;

        ar.labelElement( "m_pEffectUsage" ) & bednet.m_pEffectUsage;
        LOG_ERR( "TBD: Serialize enum." );
        ar.labelElement( "m_TriggerReceived" ) & (uint32_t&)bednet.m_TriggerReceived;
        ar.labelElement( "m_TriggerUsing"    ) & (uint32_t&)bednet.m_TriggerUsing;
        ar.labelElement( "m_TriggerDiscard"  ) & (uint32_t&)bednet.m_TriggerDiscard;
        ar.labelElement( "m_ExpirationTimer" ) & bednet.m_ExpirationTimer;
        ar.labelElement( "m_TimerHasExpired" ) & bednet.m_TimerHasExpired;
    }
}
