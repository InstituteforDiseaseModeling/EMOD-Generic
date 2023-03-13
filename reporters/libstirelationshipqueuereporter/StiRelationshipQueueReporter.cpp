/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <string>
#include "StiRelationshipQueueReporter.h"
#include "Exceptions.h"
#include "INodeSTI.h"
#include "NodeEventContext.h"
#include "IPairFormationAgent.h"
#include "INodeContext.h"

#include "Environment.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

//******************************************************************************

using namespace std;

//******************************************************************************

SETUP_LOGGING( "StiRelationshipQueueReporter" )

static const char* _sim_types[] = { "STI_SIM", "HIV_SIM", nullptr };

Kernel::DllInterfaceHelper DLL_HELPER( _module, _sim_types );

//******************************************************************************
// DLL Methods
//******************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

DTK_DLLEXPORT char*
__cdecl GetEModuleVersion(char* sVer, const Environment* pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void
__cdecl GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char*
__cdecl GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT Kernel::IReport*
__cdecl GetReportInstantiator()
{
    return new Kernel::StiRelationshipQueueReporter();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- StiRelationshipQueueReporter Methods
// ----------------------------------------

namespace Kernel
{
    StiRelationshipQueueReporter::StiRelationshipQueueReporter()
        : BaseTextReport("RelationshipQueueReporter.csv")
        , m_FirstTime(true)
    {
    }

    StiRelationshipQueueReporter::~StiRelationshipQueueReporter()
    {
    }

    std::string StiRelationshipQueueReporter::GetHeader() const
    {
        return "";
    }

    void StiRelationshipQueueReporter::UpdateEventRegistration( float currentTime, 
                                                                float dt, 
                                                                std::vector<INodeEventContext*>& rNodeEventContextList,
                                                                ISimulationEventContext* pSimEventContext )
    {
        if( m_FirstTime )
        {
            m_FirstTime = false ;

            // ----------------------------------------------------------------------------
            // --- Since we won't have the age bin information until we have entered
            // --- the Simulation::Update(), we can't determine the file header in
            // --- the normal GetHeader().  Hence, the first time we enter this method
            // --- we can determine the maximum number bins and create the correct number
            // --- of columns
            // ----------------------------------------------------------------------------
            int max_bins_m = 0 ;
            int max_bins_f = 0 ;
            for( auto p_NEC : rNodeEventContextList )
            {
                INodeSTI * p_node_sti = p_NEC->GetNodeContext()->GetNodeSTI();
                release_assert(p_node_sti);
                ISociety* p_society = p_node_sti->GetSociety();
                for( int irel = 0 ; irel < RelationshipType::COUNT ; irel++ )
                {
                    RelationshipType::Enum rel_type = (RelationshipType::Enum)irel ;
                    int num_bins_m = p_society->GetPFA( rel_type )->GetAgeBins().at( Gender::MALE   ).size();
                    int num_bins_f = p_society->GetPFA( rel_type )->GetAgeBins().at( Gender::FEMALE ).size();

                    if( max_bins_m < num_bins_m )
                    {
                        max_bins_m = num_bins_m ;
                    }
                    if( max_bins_f < num_bins_f )
                    {
                        max_bins_f = num_bins_f ;
                    }
                }
            }

            std::stringstream header ;
            header 
                << "time,"
                << "Node_ID,"
                << "Before/After,"
                << "RelationshipType,"
                << "# of Males,"
                << "# of Females," ;
            for( int i = 0 ; i < max_bins_m ; i++ )
            {
                header << "Male_Bin_Index_" << (i+1) << "," ;
            }
            for( int i = 0 ; i < max_bins_f ; i++ )
            {
                header << "Female_Bin_Index_" << (i+1) ;

                // avoid putting last comma
                if( (i+1) < max_bins_f )
                {
                    header << "," ;
                }
            }
            GetOutputStream() << header.str() << std::endl ;
        }
    }

    void StiRelationshipQueueReporter::LogNodeData( INodeContext* pNC )
    {
        // --------------------------------
        // --- Get access to the queue data
        // --------------------------------

        INodeSTI* p_node_sti = pNC->GetNodeSTI();
        release_assert(p_node_sti);
        ISociety* p_society = p_node_sti->GetSociety();

        for( int irel = 0 ; irel < RelationshipType::COUNT ; irel++ )
        {
            RelationshipType::Enum rel_type = (RelationshipType::Enum)irel ;
            std::string p_rel_type_str = p_node_sti->GetRelationshipName( irel );

            std::stringstream line_before ;
            line_before << pNC->GetTime().time  << ","
                        << pNC->GetExternalID() << "," 
                        << "BEFORE"             << ","
                        << p_rel_type_str       << "," ;

            std::stringstream line_after ;
            line_after  << pNC->GetTime().time  << ","
                        << pNC->GetExternalID() << "," 
                        << "AFTER"             << ","
                        << p_rel_type_str       << "," ;

            std::string lengths_str_before = CreateLengthsString( p_society->GetPFA( rel_type )->GetQueueLengthsBefore() );
            std::string lengths_str_after  = CreateLengthsString( p_society->GetPFA( rel_type )->GetQueueLengthsAfter()  );

            GetOutputStream() << line_before.str() << lengths_str_before << std::endl ;
            GetOutputStream() << line_after.str()  << lengths_str_after  << std::endl ;
        }
    }

    std::string StiRelationshipQueueReporter::CreateLengthsString( const map<int, vector<int>>& rQueueLengthMap )
    {
        int total_m = 0 ;
        int total_f = 0 ;
        std::string bin_str_m = CreateBinString( rQueueLengthMap.at( Gender::MALE   ), &total_m );
        std::string bin_str_f = CreateBinString( rQueueLengthMap.at( Gender::FEMALE ), &total_f );

        std::stringstream ss_length_str ;
        ss_length_str << total_m   << ","
                      << total_f   << ","
                      << bin_str_m << ","
                      << bin_str_f ;

        return ss_length_str.str() ;
    }

    std::string StiRelationshipQueueReporter::CreateBinString( const std::vector<int>& rQueue, int* pTotalCount )
    {
        std::stringstream line_bins ;
        for( int i = 0 ; i < rQueue.size() ; i++ )
        {
            int num = rQueue[i] ;
            *pTotalCount += num ;
            line_bins << num ;
            if( (i+1) < rQueue.size() )
            {
                line_bins << "," ;
            }
        }
        return line_bins.str();
    }
}
