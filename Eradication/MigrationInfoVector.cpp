/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"
#include "MigrationInfoVector.h"
#include "INodeContext.h"
#include "VectorContexts.h"


namespace Kernel
{
    // ------------------------------------------------------------------------
    // --- MigrationInfoNullVector
    // ------------------------------------------------------------------------

    BEGIN_QUERY_INTERFACE_DERIVED( MigrationInfoNullVector, MigrationInfoNull )
    END_QUERY_INTERFACE_DERIVED( MigrationInfoNullVector, MigrationInfoNull )

    MigrationInfoNullVector::MigrationInfoNullVector()
    : MigrationInfoNull()
    {
    }

    MigrationInfoNullVector::~MigrationInfoNullVector()
    {
    }


    // ------------------------------------------------------------------------
    // --- MigrationInfoVector
    // ------------------------------------------------------------------------

    BEGIN_QUERY_INTERFACE_DERIVED(MigrationInfoVector, MigrationInfoFixedRate)
    END_QUERY_INTERFACE_DERIVED(MigrationInfoVector, MigrationInfoFixedRate)

    MigrationInfoVector::MigrationInfoVector( INodeContext * _parent,
                                              ModiferEquationType::Enum equation,
                                              float habitatModifier,
                                              float foodModifier,
                                              float stayPutModifier ) 
    : MigrationInfoFixedRate( _parent ) 
    , m_RawMigrationRate()
    , m_ThisNodeId(suids::nil_suid())
    , m_ModifierEquation(equation)
    , m_ModifierHabitat(habitatModifier)
    , m_ModifierFood(foodModifier)
    , m_ModifierStayPut(stayPutModifier)
    {
    }

    MigrationInfoVector::~MigrationInfoVector() 
    {
    }

    void MigrationInfoVector::Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData )
    {
        MigrationInfoFixedRate::Initialize( rRateData );
    }

    void MigrationInfoVector::SaveRawRates( std::vector<float>& r_rate_cdf )
    {
        // ---------------------------------------------------------
        // --- Keep the un-normalized rates so we can multiply them
        // --- times our food adjusted rates.
        // ---------------------------------------------------------
        m_RawMigrationRate.clear();
        for( int i = 0; i < r_rate_cdf.size(); i++)
        {
            m_RawMigrationRate.push_back( r_rate_cdf[i] );
        }
    }

    void MigrationInfoVector::PickMigrationStep( RANDOMBASE* pRNG,
                                                 IIndividualHumanContext *traveler, 
                                                 suids::suid& destination, 
                                                 MigrationType::Enum& migration_type, 
                                                 float& timeUntilTrip ) 
    {
        MigrationInfoFixedRate::PickMigrationStep( pRNG, traveler, destination, migration_type, timeUntilTrip );

        // ------------------------------------------------------------------
        // --- if the destination is the current node, then the selection
        // --- was to stay put.  If this is the choice, then we don't want to
        // --- return anything.
        // ------------------------------------------------------------------
        if( destination == m_ThisNodeId )
        {
            destination = suids::nil_suid();
            migration_type = MigrationType::NO_MIGRATION;
            timeUntilTrip = -1.0;
        }
    }

    int GetNodePopulation( const suids::suid& rNodeId, 
                           const std::string& rSpeciesID, 
                           IVectorSimulationContext* pivsc )
    {
        return pivsc->GetNodePopulation( rNodeId ) ;
    }

    int GetAvailableLarvalHabitat( const suids::suid& rNodeId, 
                                   const std::string& rSpeciesID, 
                                   IVectorSimulationContext* pivsc )
    {
        return pivsc->GetAvailableLarvalHabitat( rNodeId, rSpeciesID );
    }

    std::vector<float> MigrationInfoVector::GetRatios( const std::vector<suids::suid>& rReachableNodes, 
                                                       const std::string& rSpeciesID, 
                                                       IVectorSimulationContext* pivsc, 
                                                       tGetValueFunc getValueFunc )
    {
        // -----------------------------------
        // --- Find the total number of people
        // --- Find the total reachable and available larval habitat
        // -----------------------------------
        float total = 0.0 ;
        for( auto node_id : rReachableNodes )
        {
            total += getValueFunc( node_id, rSpeciesID, pivsc ) ;
        }

        std::vector<float> ratios ;
        for( auto node_id : rReachableNodes )
        {
            float pr = 0.0 ;
            if( total > 0.0 )
            {
                pr = getValueFunc( node_id, rSpeciesID, pivsc ) / total ;
            }
            ratios.push_back( pr );
        }
        return ratios;
    }

    void MigrationInfoVector::UpdateRates( const suids::suid& rThisNodeId, 
                                           const std::string& rSpeciesID, 
                                           IVectorSimulationContext* pivsc )
    {
        // ---------------------------------------------------------------------------------
        // --- If we want to factor in the likelihood that a vector will decide that
        // --- the grass is not greener on the other side, then we need to add "this/current"
        // --- node as a possible node to go to.
        // ---------------------------------------------------------------------------------
        if( (m_ModifierStayPut > 0.0) && (m_ReachableNodes.size() >0) && (m_ReachableNodes[0] != rThisNodeId) )
        {
            m_ThisNodeId = rThisNodeId ;
            m_ReachableNodes.insert( m_ReachableNodes.begin(), rThisNodeId );
            m_MigrationTypes.insert( m_MigrationTypes.begin(), MigrationType::LOCAL_MIGRATION );
            m_RawMigrationRate.insert( m_RawMigrationRate.begin(), 0.0 );
            m_RateCDF.insert( m_RateCDF.begin(), 0.0 );
        }

        // -------------------------------------------------------------------
        // --- Find the ratios of population and larval habitat (i.e. things
        // --- that influence the vectors migration).  These ratios will be used
        // --- in the equations that influence which node the vectors go to.
        // -------------------------------------------------------------------
        std::vector<float> pop_ratios     = GetRatios( m_ReachableNodes, rSpeciesID, pivsc, GetNodePopulation         );
        std::vector<float> habitat_ratios = GetRatios( m_ReachableNodes, rSpeciesID, pivsc, GetAvailableLarvalHabitat );

        // --------------------------------------------------------------------------
        // --- Determine the new rates by adding the rates from the files times
        // --- to the food and habitat adjusted rates.
        // --------------------------------------------------------------------------
        release_assert( m_RawMigrationRate.size() == m_ReachableNodes.size() );
        release_assert( m_RateCDF.size()          == m_ReachableNodes.size() );
        release_assert( m_RateCDF.size()          == pop_ratios.size()      );
        release_assert( m_RateCDF.size()          == habitat_ratios.size()  );

        float tmp_totalrate = 0.0;
        for( int i = 0; i < m_RateCDF.size(); i++)
        {
            tmp_totalrate += m_RawMigrationRate[i] ; // need this to be the raw rate

            m_RateCDF[i] = CalculateModifiedRate( m_ReachableNodes[i], 
                                                  m_RawMigrationRate[i], 
                                                  pop_ratios[i], 
                                                  habitat_ratios[i] ) ;
        }

        NormalizeRates( m_RateCDF, m_TotalRate );

        // -----------------------------------------------------------------------------------
        // --- We want to use the rate from the files instead of the value changed due to the 
        // --- food modifier.  If we don't do this we get much less migration than desired.
        // -----------------------------------------------------------------------------------
        m_TotalRate = tmp_totalrate;
    }

    float MigrationInfoVector::CalculateModifiedRate( const suids::suid& rNodeId,
                                                      float rawRate, 
                                                      float populationRatio, 
                                                      float habitatRatio )
    {
        // --------------------------------------------------------------------------
        // --- Determine the probability that the mosquito will not migrate because
        // --- there is enough food or habitat in there current node
        // --------------------------------------------------------------------------
        float sp = 1.0 ;
        if( (m_ModifierStayPut > 0.0) && (rNodeId == m_ThisNodeId) )
        {
            sp = m_ModifierStayPut ;
        }

        // ---------------------------------------------------------------------------------
        // --- 10/16/2015 Jaline says that research shows that vectors don't necessarily go
        // --- to houses with more people, but do go to places with people versus no people.
        // --- Hence, 1 => go to node with people, 0 => avoid nodes without people.
        // ---------------------------------------------------------------------------------
        float pr = populationRatio ;
        if( pr > 0.0 )
        {
            pr = 1.0 ;
        }

        float rate = 0.0 ;
        switch( m_ModifierEquation )
        {
            case ModiferEquationType::LINEAR:
                rate = rawRate + (sp * m_ModifierFood * pr) + (sp * m_ModifierHabitat * habitatRatio) ;
                break;
            case ModiferEquationType::EXPONENTIAL:
                {
                    // ------------------------------------------------------------
                    // --- The -1 allows for values between 0 and 1.  Otherwise,
                    // --- the closer we got to zero the more our get closer to 1.
                    // ------------------------------------------------------------
                    float fm = 0.0 ;
                    if( m_ModifierFood > 0.0 )
                    {
                        fm =  exp( sp * m_ModifierFood * pr ) - 1.0f;
                    }
                    float hm = 0.0 ;
                    if( m_ModifierHabitat > 0.0 )
                    {
                        hm = exp( sp * m_ModifierHabitat * habitatRatio ) - 1.0f ;
                    }
                    rate = rawRate + fm + hm ;
                }
                break;
            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "Vector_Migration_Modifier_Equation", m_ModifierEquation, ModiferEquationType::pairs::lookup_key( m_ModifierEquation ) );
        }

        return rate ;
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoFactoryVector
    // ------------------------------------------------------------------------

    MigrationInfoFactoryVector::MigrationInfoFactoryVector()
    : MigrationInfoFactoryFile()
    , m_InfoFileListVector()
    { }

    MigrationInfoFactoryVector::~MigrationInfoFactoryVector()
    {
        for( auto mig_file : m_InfoFileListVector )
        {
            delete mig_file;
        }
        m_InfoFileListVector.clear();
    }

    void MigrationInfoFactoryVector::Initialize( const string& idreference )
    {
        MigrationInfoFactoryFile::Initialize( idreference );

        const MigrationParams* mp = MigrationConfig::GetMigrationParams();

        m_InfoFileListVector.push_back( new MigrationInfoFile( MigrationType::LOCAL_MIGRATION,    MAX_LOCAL_MIGRATION_DESTINATIONS,    mp->enable_mig_vec_local,    mp->mig_file_vec_local,    mp->mig_mult_vec_local ) );
        m_InfoFileListVector.push_back( nullptr );
        m_InfoFileListVector.push_back( new MigrationInfoFile( MigrationType::REGIONAL_MIGRATION, MAX_REGIONAL_MIGRATION_DESTINATIONS, mp->enable_mig_vec_regional, mp->mig_file_vec_regional, mp->mig_mult_vec_regional ) );
        m_InfoFileListVector.push_back( nullptr );
        m_InfoFileListVector.push_back( nullptr );

        for( int i = 0 ; i < m_InfoFileListVector.size() ; i++ )
        {
            if( m_InfoFileListVector[i] && m_InfoFileListVector[i]->FileEnabled() )
            {
                m_InfoFileListVector[i]->Initialize( idreference );
            }
        }
    }

    IMigrationInfoVector* MigrationInfoFactoryVector::CreateMigrationInfoVector( INodeContext *pParentNode )
    {
        bool is_fixed_rate = true ;

        std::vector<std::vector<MigrationRateData>> rate_data = GetRateData( pParentNode, m_InfoFileListVector, &is_fixed_rate );

        IMigrationInfoVector* p_new_migration_info;

        const MigrationParams* mp = MigrationConfig::GetMigrationParams();

        if( rate_data.size() > 0 )
        {
            MigrationInfoVector* p_miv = _new_ MigrationInfoVector( pParentNode, mp->vec_mod_equ, mp->vec_mod_habitat, mp->vec_mod_food, mp->vec_mod_stayput);
            p_miv->Initialize( rate_data ); 
            p_new_migration_info = p_miv;
        }
        else
        {
            p_new_migration_info = new MigrationInfoNullVector();
        }

        return p_new_migration_info;
    }

    // ------------------------------------------------------------------------
    // --- MigrationInfoFactoryVectorDefault
    // ------------------------------------------------------------------------

    MigrationInfoFactoryVectorDefault::MigrationInfoFactoryVectorDefault( int torusSize )
    : MigrationInfoFactoryDefault( torusSize )
    { }

    MigrationInfoFactoryVectorDefault::MigrationInfoFactoryVectorDefault()
    : MigrationInfoFactoryDefault( 10 )
    { }

    MigrationInfoFactoryVectorDefault::~MigrationInfoFactoryVectorDefault()
    { }

    IMigrationInfoVector* MigrationInfoFactoryVectorDefault::CreateMigrationInfoVector( INodeContext *pParentNode )
    {
        IMigrationInfoVector* p_new_migration_info_vec;

        const MigrationParams* mp = MigrationConfig::GetMigrationParams();

        if( mp->enable_mig_vec_local)
        {
            std::vector<std::vector<MigrationRateData>> rate_data = GetRateData( pParentNode, mp->mig_mult_vec_local );

            MigrationInfoVector* p_miv = _new_ MigrationInfoVector( pParentNode, ModiferEquationType::LINEAR, 1.0f, 1.0f, 1.0f );
            p_miv->Initialize( rate_data );
            p_new_migration_info_vec = p_miv;
        }
        else
        {
            p_new_migration_info_vec = new MigrationInfoNullVector();
        }

        return p_new_migration_info_vec;
    }

}
