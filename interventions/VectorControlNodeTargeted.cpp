/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "VectorControlNodeTargeted.h"

#include "Exceptions.h"
#include "InterventionFactory.h"
#include "NodeVectorEventContext.h" // for INodeVectorInterventionEffectsApply methods
#include "SimulationConfig.h"
#include "VectorParameters.h"
#include "DistributionFactory.h"

SETUP_LOGGING( "VectorControlNodeTargeted" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(SimpleVectorControlNode)
        HANDLE_INTERFACE( IConfigurable )
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleVectorControlNode)

    IMPLEMENT_FACTORY_REGISTERED(Larvicides)
    IMPLEMENT_FACTORY_REGISTERED(SpaceSpraying)
    IMPLEMENT_FACTORY_REGISTERED(MultiInsecticideSpaceSpraying)
    IMPLEMENT_FACTORY_REGISTERED(IndoorSpaceSpraying)
    IMPLEMENT_FACTORY_REGISTERED(MultiInsecticideIndoorSpaceSpraying)
    IMPLEMENT_FACTORY_REGISTERED(SpatialRepellent)
    IMPLEMENT_FACTORY_REGISTERED(ArtificialDiet)
    IMPLEMENT_FACTORY_REGISTERED(SugarTrap)
    IMPLEMENT_FACTORY_REGISTERED(OvipositionTrap)
    IMPLEMENT_FACTORY_REGISTERED(OutdoorRestKill)
    IMPLEMENT_FACTORY_REGISTERED(AnimalFeedKill)

    SimpleVectorControlNode::SimpleVectorControlNode()
        : BaseNodeIntervention()
        , killing( 0.0f )
        , reduction( 0.0f )
        , m_HabitatTarget(VectorHabitatType::ALL_HABITATS)
        , killing_effect( nullptr )
        , blocking_effect( nullptr ) 
        , m_pINVIC(nullptr)
    {
        initSimTypes( 3, "VECTOR_SIM", "MALARIA_SIM", "DENGUE_SIM" );
    }

    SimpleVectorControlNode::SimpleVectorControlNode( const SimpleVectorControlNode& master )
        : BaseNodeIntervention( master )
        , killing( master.killing )
        , reduction( master.reduction )
        , m_HabitatTarget( master.m_HabitatTarget )
        , killing_effect( nullptr )
        , blocking_effect( nullptr )
        , m_pINVIC( nullptr )
    {
        if(master.blocking_effect)
        {
            blocking_effect = master.blocking_effect->Clone();
        }
        if(master.killing_effect)
        {
            killing_effect = master.killing_effect->Clone();
        }
    }

    SimpleVectorControlNode::~SimpleVectorControlNode()
    {
        delete killing_effect;
        delete blocking_effect;

        killing_effect  = nullptr;
        blocking_effect = nullptr;
    }

    bool SimpleVectorControlNode::Configure( const Configuration * inputJson )
    {
        // TODO: consider to what extent we want to pull the decay constants out of here as well
        //       in particular, for spatial repellents where there is reduction but not killing,
        //       the primary constant is un-used in BOX and DECAY (but not BOXDECAY)
        //       whereas, oviposition traps only have a killing effect.  (ERAD-599)
        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, VCN_Cost_To_Consumer_DESC_TEXT, 0, 999999, 10.0);

        bool configured = ConfigureKilling( inputJson );

        return configured;
    }

    void SimpleVectorControlNode::initConfigKilling()
    {
    }

    void SimpleVectorControlNode::initConfigRepelling()
    {
    }

    bool SimpleVectorControlNode::ConfigureKilling( const Configuration* config )
    {
        bool configured = BaseNodeIntervention::Configure( config );

        return configured;
    }

    void SimpleVectorControlNode::SetContextTo( INodeEventContext *context )
    {
        BaseNodeIntervention::SetContextTo( context );

        if(killing_effect)
        {
            killing_effect->SetContextTo( context );
        }
        if(blocking_effect)
        {
            blocking_effect->SetContextTo( context );
        }

        if (s_OK != context->QueryInterface(GET_IID(INodeVectorInterventionEffectsApply), (void**)&m_pINVIC) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "INodeVectorInterventionEffectsApply", "INodeEventContext" );
        }
    }

    bool SimpleVectorControlNode::Distribute( INodeEventContext *pNodeContext, IEventCoordinator2 *pEC )
    {
        // Just one of each of these allowed
        pNodeContext->PurgeExisting( typeid(*this).name() ); // hmm?  let's come back to this and query the right interfaces everywhere.
        return BaseNodeIntervention::Distribute( pNodeContext, pEC );
    }
    
    float SimpleVectorControlNode::GetKilling() const
    {
        return killing;
    }

    float SimpleVectorControlNode::GetReduction() const
    {
        return reduction;
    }

    VectorHabitatType::Enum SimpleVectorControlNode::GetHabitatTarget() const
    {
        return m_HabitatTarget;
    }

    void SimpleVectorControlNode::Update( float dt )
    {
        if( !BaseNodeIntervention::UpdateNodesInterventionStatus() ) return;

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
                killing = killing_effect->Current();
            }
        }

        if(blocking_effect)
        {
            blocking_effect->Update(dt);
            if(blocking_effect->Expired())
            {
                delete blocking_effect;
                blocking_effect = nullptr;
            }
            else
            {
                reduction = blocking_effect->Current();
            }
        }

        ApplyEffects( dt );
    }

    void SimpleVectorControlNode::CheckHabitatTarget( VectorHabitatType::Enum habitatType,
                                                      const char* pParameterName )
    {
        if( habitatType == VectorHabitatType::NONE )
        {
            std::stringstream ss;
            ss << "Invalid parameter value: '" << pParameterName << "' = 'NONE'\n";
            ss << "Please select one of the following types:\n";
            // start at 1 to skip NONE
            for( int i = 1; i < VectorHabitatType::pairs::count(); ++i )
            {
                ss << VectorHabitatType::pairs::get_keys()[ i ] << "\n";
            }
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }
        else if( habitatType != VectorHabitatType::ALL_HABITATS )
        {
            VectorParameters* p_vp = GET_CONFIGURABLE( SimulationConfig )->vector_params;

            std::stringstream ss_config_habitats;
            bool found = false;
        }
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- Larvicides ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    Larvicides::Larvicides()
        : SimpleVectorControlNode()
        , m_Coverage(1.0f)
    {
    }

    Larvicides::Larvicides( const Larvicides& rMaster )
        : SimpleVectorControlNode( rMaster )
        , m_Coverage( rMaster.m_Coverage )
    {
    }

    Larvicides::~Larvicides()
    {
    }

    bool Larvicides::Configure( const Configuration * inputJson )
    {
        initConfig( "Habitat_Target", m_HabitatTarget, inputJson, MetadataDescriptor::Enum("Habitat_Target", LV_Habitat_Target_DESC_TEXT, MDD_ENUM_ARGS(VectorHabitatType)) );

        killing_effect   = WaningEffectFactory::CreateInstance();
        blocking_effect  = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",    killing_effect->GetConfigurable(),    LV_Killing_Config_DESC_TEXT);
        initConfigTypeMap("Blocking_Config",   blocking_effect->GetConfigurable(),   LV_Blocking_Config_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    void Larvicides::initConfigKilling()
    {
    }

    void Larvicides::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);

        m_pINVIC->UpdateLarvalKilling( GetHabitatTarget(), GetKilling() );
        m_pINVIC->UpdateLarvalHabitatReduction( GetHabitatTarget(), GetReduction() );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- SpaceSpraying ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    SpaceSpraying::SpaceSpraying()
        : SimpleVectorControlNode()
        , m_Coverage(1.0f)
        , kill_target( SpaceSprayTarget::SpaceSpray_FemalesAndMales )
    {
    }

    SpaceSpraying::SpaceSpraying( const SpaceSpraying& rMaster )
        : SimpleVectorControlNode( rMaster )
        , m_Coverage( rMaster.m_Coverage )
        , kill_target( rMaster.kill_target )
    {
    }

    SpaceSpraying::~SpaceSpraying()
    {
    }

    bool SpaceSpraying::Configure( const Configuration * inputJson )
    {
        initConfig( "Habitat_Target", m_HabitatTarget, inputJson, MetadataDescriptor::Enum("Habitat_Target", SS_Habitat_Target_DESC_TEXT, MDD_ENUM_ARGS(VectorHabitatType)) );
        initConfig( "Spray_Kill_Target", kill_target, inputJson, MetadataDescriptor::Enum("Spray_Kill_Target", SS_Kill_Target_DESC_TEXT, MDD_ENUM_ARGS(SpaceSprayTarget)) );

        killing_effect  = WaningEffectFactory::CreateInstance();
        blocking_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",    killing_effect->GetConfigurable(),    SS_Killing_Config_DESC_TEXT);
        initConfigTypeMap("Reduction_Config",  blocking_effect->GetConfigurable(),   SS_Reduction_Config_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    void SpaceSpraying::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);

        // TODO - consider spatial (node-wide) indoor spraying as a separate intervention?
        if ( GetKillTarget() != SpaceSprayTarget::SpaceSpray_Indoor )
        {
            m_pINVIC->UpdateLarvalHabitatReduction( GetHabitatTarget(), GetReduction() );
        }

        switch( GetKillTarget() )
        {
            case SpaceSprayTarget::SpaceSpray_FemalesOnly:
                m_pINVIC->UpdateOutdoorKilling( GetKilling() );
                break;

            case SpaceSprayTarget::SpaceSpray_MalesOnly:
                m_pINVIC->UpdateOutdoorKillingMale( GetKilling() );
                break;

            case SpaceSprayTarget::SpaceSpray_FemalesAndMales:
                m_pINVIC->UpdateOutdoorKilling( GetKilling() );
                m_pINVIC->UpdateOutdoorKillingMale( GetKilling() );
                break;

            case SpaceSprayTarget::SpaceSpray_Indoor:
                m_pINVIC->UpdateIndoorKilling( GetKilling() );
                break;

            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "GetKillTarget()", GetKillTarget(), SpaceSprayTarget::pairs::lookup_key(GetKillTarget()) );
                // break;
        }
    }

    SpaceSprayTarget::Enum SpaceSpraying::GetKillTarget() const
    {
        return kill_target;
    }

    // ---------------------------------------------------------------------------------------------------------
    //------------------------------ MultiInsecticideSpaceSpraying ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    MultiInsecticideSpaceSpraying::MultiInsecticideSpaceSpraying()
        : SpaceSpraying()
    {
    }

    MultiInsecticideSpaceSpraying::MultiInsecticideSpaceSpraying( const MultiInsecticideSpaceSpraying& rMaster )
        : SpaceSpraying( rMaster )
    {
    }

    MultiInsecticideSpaceSpraying::~MultiInsecticideSpaceSpraying()
    {
    }

    bool MultiInsecticideSpaceSpraying::ConfigureKilling( const Configuration* config )
    {
        bool configured = JsonConfigurable::Configure( config );
        if( !JsonConfigurable::_dryrun && configured )
        {
        }
        return configured;
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- IndoorSpaceSpraying ----------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    IndoorSpaceSpraying::IndoorSpaceSpraying()
        : SimpleVectorControlNode()
        , m_Coverage(1.0f)
    {
    }

    IndoorSpaceSpraying::IndoorSpaceSpraying( const IndoorSpaceSpraying& rMaster )
        : SimpleVectorControlNode( rMaster )
        , m_Coverage( rMaster.m_Coverage )
    {
    }

    IndoorSpaceSpraying::~IndoorSpaceSpraying()
    {
    }

    bool IndoorSpaceSpraying::Configure( const Configuration * inputJson )
    {
        initConfigTypeMap("Spray_Coverage", &m_Coverage, ISS_Spray_Coverage_DESC_TEXT, 0.0f, 1.0f, 1.0f);

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        return configured;
    }

    void IndoorSpaceSpraying::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);

        m_pINVIC->UpdateIndoorKilling( killing );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- MultiInsecticideIndoorSpaceSpraying ------------------------
    // ---------------------------------------------------------------------------------------------------------

    MultiInsecticideIndoorSpaceSpraying::MultiInsecticideIndoorSpaceSpraying()
        : IndoorSpaceSpraying()
    {
    }

    MultiInsecticideIndoorSpaceSpraying::MultiInsecticideIndoorSpaceSpraying( const MultiInsecticideIndoorSpaceSpraying& rMaster )
        : IndoorSpaceSpraying( rMaster )
    {
    }

    MultiInsecticideIndoorSpaceSpraying::~MultiInsecticideIndoorSpaceSpraying()
    {
    }

    bool MultiInsecticideIndoorSpaceSpraying::ConfigureKilling( const Configuration* config )
    {
        bool configured = JsonConfigurable::Configure( config );
        if( !JsonConfigurable::_dryrun && configured )
        {
        }
        return configured;
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- SpatialRepellent ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    SpatialRepellent::SpatialRepellent()
        : SimpleVectorControlNode()
        , m_Coverage(1.0)
    {
    }

    SpatialRepellent::SpatialRepellent( const SpatialRepellent& rMaster )
        : SimpleVectorControlNode( rMaster )
        , m_Coverage( rMaster.m_Coverage )
    {
    }

    SpatialRepellent::~SpatialRepellent()
    {
    }

    bool SpatialRepellent::Configure( const Configuration * inputJson )
    {
        blocking_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Repellency_Config",   blocking_effect->GetConfigurable(),   SR_Repellency_Config_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    void SpatialRepellent::initConfigRepelling()
    {
    }

    void SpatialRepellent::initConfigKilling()
    {
    }

    void SpatialRepellent::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);

        m_pINVIC->UpdateVillageSpatialRepellent( GetReduction() );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- ArtificialDiet ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    ArtificialDiet::ArtificialDiet()
        : SimpleVectorControlNode()
        , m_AttractionTarget( ArtificialDietTarget::AD_OutsideVillage )
    {
    }

    ArtificialDiet::ArtificialDiet( const ArtificialDiet& rMaster )
        : SimpleVectorControlNode( rMaster )
        , m_AttractionTarget( rMaster.m_AttractionTarget )
    {
    }

    ArtificialDiet::~ArtificialDiet()
    {
    }

    bool ArtificialDiet::Configure( const Configuration * inputJson )
    {
        initConfig( "Artificial_Diet_Target", m_AttractionTarget, inputJson, MetadataDescriptor::Enum("Artificial_Diet_Target", AD_Target_DESC_TEXT, MDD_ENUM_ARGS(ArtificialDietTarget)) );

        blocking_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Attraction_Config",  blocking_effect->GetConfigurable(),  AD_Attraction_Config_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    bool ArtificialDiet::ConfigureKilling( const Configuration* config )
    {
        // skip other stuff in base class
        return BaseNodeIntervention::Configure( config );
    }

    void ArtificialDiet::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);

        switch( m_AttractionTarget )
        {
            case ArtificialDietTarget::AD_WithinVillage:
                m_pINVIC->UpdateADIVAttraction( GetReduction() );
                break;

            case ArtificialDietTarget::AD_OutsideVillage:
                m_pINVIC->UpdateADOVAttraction( GetReduction() );
                break;

            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__,
                                                            "GetAttractionTarget()",
                                                            m_AttractionTarget,
                                                            ArtificialDietTarget::pairs::lookup_key(m_AttractionTarget) );
        }
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- SugarTrap ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    SugarTrap::SugarTrap()
        : SimpleVectorControlNode()
        , m_pExpirationDistribution( nullptr )
        , m_ExpirationTimer( 0.0 )
        , m_TimerHasExpired( false )
    {
        m_ExpirationTimer.handle = std::bind( &SugarTrap::Callback, this, std::placeholders::_1 );
    }

    SugarTrap::SugarTrap( const SugarTrap& master )
        : SimpleVectorControlNode( master )
        , m_pExpirationDistribution( nullptr )
        , m_ExpirationTimer( master.m_ExpirationTimer )
        , m_TimerHasExpired( master.m_TimerHasExpired )
    {
        if( master.m_pExpirationDistribution != nullptr )
        {
            this->m_pExpirationDistribution = master.m_pExpirationDistribution->Clone();
        }
        m_ExpirationTimer.handle = std::bind( &SugarTrap::Callback, this, std::placeholders::_1 );
    }

    SugarTrap::~SugarTrap()
    {
        delete m_pExpirationDistribution;
    }

    bool SugarTrap::Configure( const Configuration * inputJson )
    {
        killing_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",  killing_effect->GetConfigurable(),  VCN_Killing_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    bool SugarTrap::Distribute( INodeEventContext *pNodeContext, IEventCoordinator2 *pEC )
    {
        bool distributed =  SimpleVectorControlNode::Distribute( pNodeContext, pEC );
        if( distributed )
        {
        }
        return distributed;
    }

    void SugarTrap::Update( float dt )
    {
        if( !BaseNodeIntervention::UpdateNodesInterventionStatus() ) return;

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
                killing = killing_effect->Current();
            }
        }

        if(blocking_effect)
        {
            blocking_effect->Update(dt);
            if(blocking_effect->Expired())
            {
                delete blocking_effect;
                blocking_effect = nullptr;
            }
            else
            {
                reduction = blocking_effect->Current();
            }
        }

        ApplyEffects( dt );
    }


    void SugarTrap::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);
        m_pINVIC->UpdateSugarFeedKilling( GetKilling() );
    }

    void SugarTrap::Callback( float dt )
    {
        m_TimerHasExpired = true;
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- OvipositionTrap ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    bool OvipositionTrap::Configure( const Configuration * inputJson )
    {
        if(!JsonConfigurable::_dryrun && MatchesDependency(inputJson, "Vector_Sampling_Type", "VECTOR_COMPARTMENTS_NUMBER,VECTOR_COMPARTMENTS_PERCENT"))
        {
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Explicit oviposition only implemented in individual-mosquito model, not in cohort model." );
        }

        initConfig( "Habitat_Target", m_HabitatTarget, inputJson, MetadataDescriptor::Enum("Habitat_Target", OT_Habitat_Target_DESC_TEXT, MDD_ENUM_ARGS(VectorHabitatType)) );

        killing_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",  killing_effect->GetConfigurable(),  OT_Killing_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    bool OvipositionTrap::ConfigureKilling( const Configuration* config )
    {
        // skip other stuff in base class
        return BaseNodeIntervention::Configure( config );
    }

    void OvipositionTrap::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);
        m_pINVIC->UpdateOviTrapKilling( GetHabitatTarget(), GetKilling() );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- OutdoorRestKill ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    bool OutdoorRestKill::Configure( const Configuration * inputJson )
    {
        killing_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",  killing_effect->GetConfigurable(),  VCN_Killing_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    void OutdoorRestKill::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);
        m_pINVIC->UpdateOutdoorRestKilling( GetKilling() );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- AnimalFeedKill ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    bool AnimalFeedKill::Configure( const Configuration * inputJson )
    {
        killing_effect = WaningEffectFactory::CreateInstance();
        initConfigTypeMap("Killing_Config",  killing_effect->GetConfigurable(),  AFK_Killing_DESC_TEXT);

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        return configured;
    }

    void AnimalFeedKill::ApplyEffects( float dt )
    {
        release_assert(m_pINVIC);
        m_pINVIC->UpdateAnimalFeedKilling( GetKilling() );
    }
}
