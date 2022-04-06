/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportVectorStatsMalaria.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "NodeEventContext.h"
#include "Individual.h"
#include "VectorContexts.h"
#include "VectorPopulation.h"
#include "ReportUtilities.h"
#include "SimulationConfig.h"
#include "MalariaParameters.h"
#include "IGenomeMarkers.h"
#include "StrainIdentity.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING( "ReportVectorStatsMalaria" )

static const char* _sim_types[] = { "MALARIA_SIM", nullptr };

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
    return new Kernel::ReportVectorStatsMalaria();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- ReportVectorStatsMalaria Methods
// ----------------------------------------
namespace Kernel
{
    ReportVectorStatsMalaria::ReportVectorStatsMalaria()
        : ReportVectorStats( "ReportVectorStatsMalaria.csv" )
        , genome_marker_columns()
    {
    }

    ReportVectorStatsMalaria::~ReportVectorStatsMalaria()
    {
    }

    bool ReportVectorStatsMalaria::Configure( const Configuration * inputJson )
    {
        bool ret = ReportVectorStats::Configure( inputJson );

        if( ret )
        {
            SimulationConfig* p_sim_config = GET_CONFIGURABLE( SimulationConfig );
            std::vector<std::pair<std::string,uint64_t>> marker_combos = p_sim_config->malaria_params->pGenomeMarkers->CreatePossibleCombinations();

            for( auto& combo : marker_combos )
            {
                genome_marker_columns.push_back( ReportUtilitiesMalaria::GenomeMarkerColumn( combo.first, combo.second ) );
            }
        }
        return ret;
    }

    std::string ReportVectorStatsMalaria::GetHeader() const
    {
        std::stringstream header ;

        header << ReportVectorStats::GetHeader();

        for( auto& r_column : genome_marker_columns )
        {
            header << ", " << r_column.GetColumnName();
        }

        return header.str();
    }

    void ReportVectorStatsMalaria::ResetOtherCounters()
    {
        for( auto& r_column : genome_marker_columns )
        {
            r_column.ResetCount();
        }
    }

    void ReportVectorStatsMalaria::CollectOtherData( IVectorPopulationReporting* pIVPR )
    {
        StrainIdentity strain;
        for( auto& r_column : genome_marker_columns )
        {
            strain.SetGeneticID( r_column.GetBitMask() );
            uint32_t gm_infected = pIVPR->getInfectedCount( &strain );
            uint32_t gm_infectious = pIVPR->getInfectiousCount( &strain );
            r_column.AddCount( gm_infected + gm_infectious );
        }
    }

    void ReportVectorStatsMalaria::WriteOtherData()
    {
        for( auto& r_column : genome_marker_columns )
        {
            GetOutputStream() << "," << r_column.GetCount();
        }
    }
}
