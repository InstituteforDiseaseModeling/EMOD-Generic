/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>

#ifdef __GNUC__
namespace std
{
     using namespace __gnu_cxx;
}
#endif

#include "IMigrationInfo.h"
#include "InterpolatedValueMap.h"


#define MAX_DESTINATIONS                   (100)

#define MAX_LOCAL_MIGRATION_DESTINATIONS     (8)
#define MAX_AIR_MIGRATION_DESTINATIONS      (60)
#define MAX_REGIONAL_MIGRATION_DESTINATIONS (30)
#define MAX_SEA_MIGRATION_DESTINATIONS       (5)
#define MAX_FAMILY_MIGRATION_DESTINATIONS    (5)


namespace Kernel
{
    // ---------------------------
    // --- MigrationRateData
    // ---------------------------

    // MigrationRateData contains data about migrating to a particular node.
    // The rates can be age dependent.
    class IDMAPI MigrationRateData
    {
    public:
        MigrationRateData();
        MigrationRateData( suids::suid to_node_suid, MigrationType::Enum migType, InterpolationType::Enum interpType );

        const suids::suid       GetToNodeSuid()           const;
        MigrationType::Enum     GetMigrationType()        const;
        InterpolationType::Enum GetInterpolationType()    const;
        int                     GetNumRates()             const;
        float                   GetRate( float ageYears ) const;

        void AddRate( float ageYears, float rate );
    private:
        suids::suid             m_ToNodeSuid ;
        MigrationType::Enum     m_MigType ;
        InterpolationType::Enum m_InterpType ;
        InterpolatedValueMap    m_InterpMap;
    };

    // ---------------------------
    // --- MigrationInfoNull
    // ---------------------------

    // MigrationInfoNull is the null object in the Null Object Pattern.
    // Essentially, this object implements the IMigrationInfo interface
    // but doesn't do anything.  This object is given to nodes when migration
    // is on but the node does not have any migration away from it.
    class IDMAPI MigrationInfoNull : virtual public IMigrationInfo
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~MigrationInfoNull();

        // IMigrationInfo methods
        virtual const MigrationParams* GetParams() const;

        virtual void PickMigrationStep( RANDOMBASE* pRNG,
                                        IIndividualHumanContext * traveler, 
                                        suids::suid &destination, 
                                        MigrationType::Enum &migration_type,
                                        float &time ) override;
        virtual void SetContextTo(INodeContext* _parent) override;
        virtual float GetTotalRate() const override;
        virtual const std::vector<float>& GetCumulativeDistributionFunction() const override;
        virtual const std::vector<suids::suid>& GetReachableNodes() const override;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes() const override;

    protected:
        friend class MigrationInfoFactoryFile;
        friend class MigrationInfoFactoryDefault;

        MigrationInfoNull();

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        // We need these member variables so that GetReachableNodes() and GetMigrationTypes()
        // can return references to objects that exist.
        std::vector<float>               m_EmptyListCDF;
        std::vector<suids::suid>         m_EmptyListNodes;
        std::vector<MigrationType::Enum> m_EmptyListTypes;
#pragma warning( pop )
    };

    // ---------------------------
    // --- MigrationInfoFixedRate
    // ---------------------------

    // MigrationInfoFixedRate is an IMigrationInfo object that has fixed/constant rates.
    class IDMAPI MigrationInfoFixedRate : virtual public IMigrationInfo
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~MigrationInfoFixedRate();

        // IMigrationInfo methods
        virtual const MigrationParams* GetParams() const;

        virtual void PickMigrationStep( RANDOMBASE* pRNG,
                                        IIndividualHumanContext * traveler, 
                                        suids::suid &destination, 
                                        MigrationType::Enum &migration_type,
                                        float &time ) override;
        virtual void SetContextTo(INodeContext* _parent) override;
        virtual float GetTotalRate() const override;
        virtual const std::vector<float>& GetCumulativeDistributionFunction() const override;
        virtual const std::vector<suids::suid>& GetReachableNodes() const override;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes() const override;

    protected:
        friend class MigrationInfoFactoryFile;
        friend class MigrationInfoFactoryDefault;

        MigrationInfoFixedRate( INodeContext* _parent );

        virtual void Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData );
        virtual void CalculateRates( Gender::Enum gender, float ageYears );
        virtual void NormalizeRates( std::vector<float>& r_rate_cdf, float& r_total_rate );
        virtual void SaveRawRates( std::vector<float>& r_rate_cdf );

        virtual const std::vector<suids::suid>& GetReachableNodes( Gender::Enum gender ) const;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes( Gender::Enum gender ) const;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        INodeContext * m_Parent;
        std::vector<suids::suid>         m_ReachableNodes;
        std::vector<MigrationType::Enum> m_MigrationTypes;
        std::vector<float>               m_RateCDF;
        float                            m_TotalRate;
#pragma warning( pop )
    };

    // -----------------------------
    // --- MigrationInfoAgeAndGender
    // -----------------------------

    // MigrationInfoAgeAndGender is an IMigrationInfo object that is used when the rates
    // between nodes is dependent on gender and age.  The reachable/"to nodes" can be different
    // for each gender.
    class IDMAPI MigrationInfoAgeAndGender : public MigrationInfoFixedRate
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~MigrationInfoAgeAndGender();

    protected:
        friend class MigrationInfoFactoryFile;
        friend class MigrationInfoFactoryDefault;

        MigrationInfoAgeAndGender( INodeContext* _parent );

        virtual void Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData );
        virtual void CalculateRates( Gender::Enum gender, float ageYears );

        virtual const std::vector<suids::suid>& GetReachableNodes( Gender::Enum gender ) const override;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes( Gender::Enum gender ) const override;


#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::vector<std::vector<MigrationRateData>> m_RateData;
        std::vector<suids::suid>         m_ReachableNodesFemale;
        std::vector<MigrationType::Enum> m_MigrationTypesFemale;
#pragma warning( pop )
    };

    // ----------------------
    // --- MigrationInfoFile
    // ----------------------

    // MigrationInfoFile is responsible for reading the migration data from files.
    // This object assumes that for one file name there is one binary file containing the
    // "to-node" and rate data while json-formatted metadata file contains extra information
    // about the data in the binary file.  The factory uses this object to create
    // the IMigrationInfo objects.
    class IDMAPI MigrationInfoFile
    {
    public:
        MigrationInfoFile( MigrationType::Enum migType,
                           int defaultDestinationsPerNode,
                           bool enable_migration = false,
                           std::string mig_filename = "",
                           float mig_modifier = 1.0f );
        virtual ~MigrationInfoFile();

        virtual void Initialize( const std::string& idreference );

        virtual bool ReadData( INodeContext* from_node_ptr,
                               std::vector<std::vector<MigrationRateData>>& rRateData );

        MigrationType::Enum GetMigrationType() const { return m_MigrationType; }
        bool                FileEnabled()      const { return m_IsEnabled; }
        bool                FilenameEmpty()    const { return m_Filename.empty(); }


    protected:
        // Returns the expected size of the binary file
        virtual uint32_t ParseMetadataForFile( const std::string& data_filepath, const std::string& idreference );
        virtual void OpenMigrationFile( const std::string& filepath, uint32_t expected_binary_file_size );
        virtual uint32_t GetNumGenderDataChunks() const;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::string             m_Filename;
        bool                    m_IsEnabled;
        float                   m_xModifier;
        int                     m_DestinationsPerNode;
        MigrationType::Enum     m_MigrationType;
        GenderDataType::Enum    m_GenderDataType;
        InterpolationType::Enum m_InterpolationType;
        std::vector<float>      m_AgesYears;
        uint32_t                m_GenderDataSize;
        uint32_t                m_AgeDataSize;
        std::ifstream           m_FileStream;

        std::unordered_map< ExternalNodeId_t, uint32_t > m_Offsets;
#pragma warning( pop )
    };

    // ----------------------------------
    // --- MigrationInfoFactoryFile
    // ----------------------------------

    // MigrationInfoFactoryFile is an IMigrationInfoFactory that creates IMigrationInfo objects based
    // on data found in migration input files.  It can create one IMigrationInfo object for each node
    // in the simulation.
    class IDMAPI MigrationInfoFactoryFile : virtual public IMigrationInfoFactory
    {
    public:
        MigrationInfoFactoryFile();
        virtual ~MigrationInfoFactoryFile();

        // IMigrationInfoFactory methods
        virtual const MigrationParams* GetParams() const;

        virtual void Initialize( const std::string& idreference ) override;
        virtual IMigrationInfo* CreateMigrationInfo( INodeContext *parent_node ) override;
        virtual bool IsAtLeastOneTypeConfiguredForIndividuals() const override;
        virtual bool IsEnabled( MigrationType::Enum mt ) const override;
    protected:
        static std::vector<std::vector<MigrationRateData>> GetRateData( INodeContext *parent_node, 
                                                                        std::vector<MigrationInfoFile*>& infoFileList,
                                                                        bool* pIsFixedRate );

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::vector<MigrationInfoFile*> m_InfoFileList ;
#pragma warning( pop )
    private:
    };

    // ----------------------------------
    // --- MigrationInfoFactoryDefault
    // ----------------------------------

    // MigrationInfoFactoryDefault is used when the user is running the default/internal scenario.
    // This assumes that there are at least 3-rows and 3-columns of nodes and that the set of nodes is square.
    class IDMAPI MigrationInfoFactoryDefault : virtual public IMigrationInfoFactory
    {
    public:
        MigrationInfoFactoryDefault( int torusSize );
        MigrationInfoFactoryDefault();
        virtual ~MigrationInfoFactoryDefault();

        // IMigrationInfoFactory methods
        virtual const MigrationParams* GetParams() const;

        virtual void Initialize( const std::string& idreference ) override;
        virtual IMigrationInfo* CreateMigrationInfo( INodeContext *parent_node ) override;
        virtual bool IsAtLeastOneTypeConfiguredForIndividuals() const override;
        virtual bool IsEnabled( MigrationType::Enum mt ) const override;
    protected:
        std::vector<std::vector<MigrationRateData>> GetRateData( INodeContext *parent_node, float modifier );

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        int   m_TorusSize;
#pragma warning( pop )
    private:
    };
}
