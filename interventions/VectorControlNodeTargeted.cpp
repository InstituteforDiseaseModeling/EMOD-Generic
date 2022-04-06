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

    //IMPLEMENT_FACTORY_REGISTERED(SimpleVectorControlNode) // don't register unusable base class
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
        , m_LarvalKillingConfig()
        , m_RepellingConfig()
        , m_KillingConfig()
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
        , m_LarvalKillingConfig() //shouldn't need to copy
        , m_RepellingConfig()     //shouldn't need to copy
        , m_KillingConfig()       //shouldn't need to copy
        , m_pINVIC( nullptr )
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

    SimpleVectorControlNode::~SimpleVectorControlNode()
    {
        delete killing_effect;
        delete blocking_effect;
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

        if( configured && !JsonConfigurable::_dryrun )
        {
            WaningConfig empty_config;

            // try to get rid of memory no longer needed
            m_LarvalKillingConfig = empty_config;
            m_RepellingConfig     = empty_config;
            m_KillingConfig       = empty_config;
        }
        return configured;
    }

    void SimpleVectorControlNode::SetContextTo( INodeEventContext *context )
    {
        BaseNodeIntervention::SetContextTo( context );

        // NOTE: Can't use WaningEffects that need SetConextTo() - i.e. aging.  Should be able to use calendar ones.
        //m_pInsecticideWaningEffect->SetContextTo( context );

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

        if( killing_effect != nullptr )
        {
            killing_effect->Update(dt);
            killing  = killing_effect->Current();
        }
        if( blocking_effect != nullptr )
        {
            blocking_effect->Update(dt);
            reduction = blocking_effect->Current();
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
            //for( int i = 0; !found && (i < p_vp->vector_species.Size()); ++i )
            //{
            //    VectorSpeciesParameters* p_vsp = p_vp->vector_species[ i ];
            //    const std::vector<IVectorHabitat*>& r_habitats = p_vsp->habitat_params.GetHabitats();
            //    for( int j = 0; !found && (j < r_habitats.size()); ++j )
            //    {
            //        const char* p_habitat_name = VectorHabitatType::pairs::lookup_key( r_habitats[ j ]->GetVectorHabitatType() );
            //        ss_config_habitats << p_vsp->name << " : " << p_habitat_name << "\n";
            //        found |= (r_habitats[ j ]->GetVectorHabitatType() == habitatType);
            //    }
            //}
            //if( !found )
            //{
            //    const char* p_habitat_name = VectorHabitatType::pairs::lookup_key( habitatType );

            //    std::stringstream ss;
            //    ss << "Invalid parameter value: '" << pParameterName << "' = '" << p_habitat_name << "'\n";
            //    ss << "This habitat type is not configured as a type in 'Vector_Species_Params.Habitats'.\n";
            //    ss << "Please select from one of the configured types:\n";
            //    ss << ss_config_habitats.str();
            //    throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            //}
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
        WaningConfig killing_config;
        WaningConfig blocking_config;

        initConfig( "Habitat_Target", m_HabitatTarget, inputJson, MetadataDescriptor::Enum("Habitat_Target", LV_Habitat_Target_DESC_TEXT, MDD_ENUM_ARGS(VectorHabitatType)) );
        initConfigComplexType("Killing_Config",  &killing_config,  LV_Killing_Config_DESC_TEXT );
        initConfigComplexType("Blocking_Config", &blocking_config, LV_Blocking_Config_DESC_TEXT );

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            killing_effect  = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json,  inputJson->GetDataLocation(), "Killing_Config" );
            blocking_effect = WaningEffectFactory::getInstance()->CreateInstance( blocking_config._json, inputJson->GetDataLocation(), "Blocking_Config" );
        }
        return configured;
    }

    void Larvicides::initConfigKilling()
    {
    }

    void Larvicides::ApplyEffects( float dt )
    {
        release_assert( m_pINVIC != nullptr );

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
        WaningConfig killing_config;
        WaningConfig blocking_config;

        initConfig( "Habitat_Target", m_HabitatTarget, inputJson, MetadataDescriptor::Enum("Habitat_Target", SS_Habitat_Target_DESC_TEXT, MDD_ENUM_ARGS(VectorHabitatType)) );
        initConfig( "Spray_Kill_Target", kill_target, inputJson, MetadataDescriptor::Enum("Spray_Kill_Target", SS_Kill_Target_DESC_TEXT, MDD_ENUM_ARGS(SpaceSprayTarget)) );
        initConfigComplexType( "Killing_Config",   &killing_config,  SS_Killing_Config_DESC_TEXT   );
        initConfigComplexType( "Reduction_Config", &blocking_config, SS_Reduction_Config_DESC_TEXT );

        bool configured = SimpleVectorControlNode::Configure( inputJson );

        if( configured && !JsonConfigurable::_dryrun )
        {
            killing_effect  = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json,  inputJson->GetDataLocation(), "Killing_Config" );
            blocking_effect = WaningEffectFactory::getInstance()->CreateInstance( blocking_config._json, inputJson->GetDataLocation(), "Blocking_Config" );
        }
        return configured;
    }

    void SpaceSpraying::ApplyEffects( float dt )
    {
        release_assert( m_pINVIC != nullptr );

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
        release_assert( m_pINVIC != nullptr );

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
        WaningConfig blocking_config;

        initConfigComplexType("Repellency_Config", &blocking_config, SR_Repellency_Config_DESC_TEXT );

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            blocking_effect = WaningEffectFactory::getInstance()->CreateInstance( blocking_config._json, inputJson->GetDataLocation(), "Repellency_Config" );
        }
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
        release_assert( m_pINVIC != nullptr );

        m_pINVIC->UpdateVillageSpatialRepellent( GetReduction() );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- ArtificialDiet ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    ArtificialDiet::ArtificialDiet()
        : SimpleVectorControlNode()
        , m_AttractionTarget( ArtificialDietTarget::AD_OutsideVillage )
        , m_pAttractionEffect( nullptr )
    {
    }

    ArtificialDiet::ArtificialDiet( const ArtificialDiet& rMaster )
        : SimpleVectorControlNode( rMaster )
        , m_AttractionTarget( rMaster.m_AttractionTarget )
        , m_pAttractionEffect( nullptr )
    {
        if( rMaster.m_pAttractionEffect != nullptr )
        {
            this->m_pAttractionEffect = rMaster.m_pAttractionEffect->Clone();
        }
    }

    ArtificialDiet::~ArtificialDiet()
    {
        delete m_pAttractionEffect;
    }

    bool ArtificialDiet::Configure( const Configuration * inputJson )
    {
        WaningConfig attraction_config;

        initConfig( "Artificial_Diet_Target", m_AttractionTarget, inputJson, MetadataDescriptor::Enum("Artificial_Diet_Target", AD_Target_DESC_TEXT, MDD_ENUM_ARGS(ArtificialDietTarget)) );
        initConfigComplexType("Attraction_Config", &attraction_config, AD_Attraction_Config_DESC_TEXT );

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            blocking_effect     = WaningEffectFactory::getInstance()->CreateInstance( attraction_config._json,
                                                                                      inputJson->GetDataLocation(),
                                                                                      "Attraction_Config" );
        }
        return configured;
    }

    bool ArtificialDiet::ConfigureKilling( const Configuration* config )
    {
        // skip other stuff in base class
        return BaseNodeIntervention::Configure( config );
    }

    void ArtificialDiet::ApplyEffects( float dt )
    {
        release_assert( m_pINVIC != nullptr );

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
        WaningConfig killing_config;
        initConfigComplexType("Killing_Config",  &killing_config, VCN_Killing_DESC_TEXT );

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            killing_effect = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json, inputJson->GetDataLocation(), "Killing_Config" );
            bool found = false;
            VectorParameters* p_vp = GET_CONFIGURABLE( SimulationConfig )->vector_params;
            //for( int i = 0; !found && (i < p_vp->vector_species.Size()); ++i )
            //{
            //    found |= (p_vp->vector_species[ i ]->vector_sugar_feeding != VectorSugarFeeding::VECTOR_SUGAR_FEEDING_NONE);
            //}
            //if( !found )
            //{
            //    std::stringstream ss;
            //    ss << "Using 'SugarTrap' intervention but 'Vector_Sugar_Feeding_Frequency' set to 'VECTOR_SUGAR_FEEDING_NONE'\n";
            //    ss << "for all the species.  'Vector_Sugar_Feeding_Frequency' must be set to something besides\n";
            //    ss << "'VECTOR_SUGAR_FEEDING_NONE' for at least one specie when using 'SugarTrap'.\n";
            //    ss << "Options are:\n";
            //    for( int i = 0; i < VectorSugarFeeding::pairs::count(); ++i )
            //    {
            //        ss << VectorSugarFeeding::pairs::get_keys()[ i ] << "\n";
            //    }
            //    throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            //}
        }
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

        if( killing_effect != nullptr )
        {
            killing_effect->Update(dt);
            killing  = killing_effect->Current();
        }
        if( blocking_effect != nullptr )
        {
            blocking_effect->Update(dt);
            reduction = blocking_effect->Current();
        }

        ApplyEffects( dt );
    }


    void SugarTrap::ApplyEffects( float dt )
    {
        release_assert( m_pINVIC != nullptr );
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

        WaningConfig killing_config;

        initConfig( "Habitat_Target", m_HabitatTarget, inputJson, MetadataDescriptor::Enum("Habitat_Target", OT_Habitat_Target_DESC_TEXT, MDD_ENUM_ARGS(VectorHabitatType)) );
        initConfigComplexType("Killing_Config",  &killing_config, OT_Killing_DESC_TEXT  );

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            killing_effect = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json, inputJson->GetDataLocation(), "Killing_Config" );
        }
        return configured;
    }

    bool OvipositionTrap::ConfigureKilling( const Configuration* config )
    {
        // skip other stuff in base class
        return BaseNodeIntervention::Configure( config );
    }

    void OvipositionTrap::ApplyEffects( float dt )
    {
        release_assert( m_pINVIC != nullptr );
        m_pINVIC->UpdateOviTrapKilling( GetHabitatTarget(), GetKilling() );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- OutdoorRestKill ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    bool OutdoorRestKill::Configure( const Configuration * inputJson )
    {
        WaningConfig killing_config;

        initConfigComplexType("Killing_Config",  &killing_config, VCN_Killing_DESC_TEXT );

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            killing_effect = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json, inputJson->GetDataLocation(), "Killing_Config" );
        }
        return configured;
    }

    void OutdoorRestKill::ApplyEffects( float dt )
    {
        release_assert( m_pINVIC != nullptr );
        m_pINVIC->UpdateOutdoorRestKilling( GetKilling() );
    }

    // ---------------------------------------------------------------------------------------------------------
    //--------------------------------------------- AnimalFeedKill ---------------------------------------------
    // ---------------------------------------------------------------------------------------------------------

    bool AnimalFeedKill::Configure( const Configuration * inputJson )
    {
        WaningConfig killing_config;

        initConfigComplexType("Killing_Config",  &killing_config, AFK_Killing_DESC_TEXT );

        bool configured = SimpleVectorControlNode::Configure( inputJson );
        if( configured && !JsonConfigurable::_dryrun )
        {
            killing_effect = WaningEffectFactory::getInstance()->CreateInstance( killing_config._json, inputJson->GetDataLocation(), "Killing_Config" );
        }
        return configured;
    }

    void AnimalFeedKill::ApplyEffects( float dt )
    {
        release_assert( m_pINVIC != nullptr );
        m_pINVIC->UpdateAnimalFeedKilling( GetKilling() );
    }
}
