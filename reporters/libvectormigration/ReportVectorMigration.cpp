/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportVectorMigration.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h" // for DTK_DLLEXPORT

#include "VectorCohortIndividual.h"
#include "IMigrate.h"
#include "ISimulationContext.h"
#include "IdmDateTime.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING( "ReportVectorMigration" )

static const char* _sim_types[] = { "VECTOR_SIM", "MALARIA_SIM", nullptr };

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
    return new Kernel::ReportVectorMigration();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- ReportVectorMigration Methods
// ----------------------------------------

namespace Kernel
{
    ReportVectorMigration::ReportVectorMigration()
        : BaseTextReport( "ReportVectorMigration.csv" )
    {
        // ------------------------------------------------------------------------------------------------
        // --- Since this report will be listening for events, it needs to increment its reference count
        // --- so that it is 1.  Other objects will be AddRef'ing and Release'ing this report/observer
        // --- so it needs to start with a refcount of 1.
        // ------------------------------------------------------------------------------------------------
        AddRef();
    }

    ReportVectorMigration::~ReportVectorMigration()
    {
    }

    std::string ReportVectorMigration::GetHeader() const
    {
        std::stringstream header ;
        header << "Time"           << ", "
               << "ID"             << ", "
               << "FromNodeID"     << ", "
               << "ToNodeID"       << ", "
               << "MigrationType"  << ", "
               << "Species"        << ", "
               << "Age"
               ;

        return header.str();
    }

    void ReportVectorMigration::LogVectorMigration( ISimulationContext* pSim, float currentTime, const suids::suid& nodeSuid, IVectorCohort* pvc )
    {
        if( (currentTime <= 365.0) || (372.0 < currentTime) )
        {
            return;
        }

        IMigrate* pim = pvc->GetIMigrate();

        uint64_t vci_id = 0;
        int from_node_id = pSim->GetNodeExternalID( nodeSuid ) ;
        int to_node_id = pSim->GetNodeExternalID( pim->GetMigrationDestination() ) ;
        int mig_type = pim->GetMigrationType() ;
        VectorStateEnum::Enum state = pvc->GetState();
        std::string species = pvc->GetSpecies();
        float age = pvc->GetAge();

        std::string mig_type_str = "" ;
        if( mig_type == MigrationType::LOCAL_MIGRATION )
            mig_type_str = "local" ;
        else if( mig_type == MigrationType::AIR_MIGRATION )
            mig_type_str = "air" ;
        else if( mig_type == MigrationType::REGIONAL_MIGRATION )
            mig_type_str = "regional" ;
        else if( mig_type == MigrationType::SEA_MIGRATION )
            mig_type_str = "sea" ;
        else
            release_assert( false );

        GetOutputStream() << currentTime
                   << "," << vci_id 
                   << "," << from_node_id 
                   << "," << to_node_id 
                   << "," << mig_type_str
                   << "," << species 
                   << "," << age 
                   << endl;
    }
}
