/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportMalariaFiltered.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "NodeEventContext.h"
#include "Individual.h"
#include "VectorContexts.h"
#include "VectorPopulation.h"
#include "IMigrate.h"
#include "INodeContext.h"
#include "ISimulationContext.h"
#include "ConfigParams.h"

//******************************************************************************

#define DEFAULT_NAME ("ReportMalariaFiltered.json")

//******************************************************************************

SETUP_LOGGING( "ReportMalariaFiltered" )

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
    return new Kernel::ReportMalariaFiltered();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- ReportMalariaFiltered Methods
// ----------------------------------------

namespace Kernel
{
    ReportMalariaFiltered::ReportMalariaFiltered()
        : ReportMalaria()
        , m_NodesToInclude()
        , m_StartDay(0.0)
        , m_EndDay(FLT_MAX)
    {
        report_name = DEFAULT_NAME;
    }

    ReportMalariaFiltered::~ReportMalariaFiltered()
    {
    }

    bool ReportMalariaFiltered::Configure( const Configuration * inputJson )
    {
        std::vector<int> valid_external_node_id_list;

        initConfigTypeMap("Node_IDs_Of_Interest", &valid_external_node_id_list, "Data will be collected for the nodes in tis list.");
        initConfigTypeMap( "Start_Day", &m_StartDay, "Day to start collecting data", 0.0f, FLT_MAX, 0.0f );
        initConfigTypeMap( "End_Day",   &m_EndDay,   "Day to stop collecting data",  0.0f, FLT_MAX, FLT_MAX );
        initConfigTypeMap( "Report_File_Name", &report_name, "Name of the file to be written", DEFAULT_NAME );

        bool ret = JsonConfigurable::Configure( inputJson );

        for( auto node_id : valid_external_node_id_list )
        {
            m_NodesToInclude.insert( std::make_pair( node_id, true ) );
        }

        channelDataMap.SetStartTime(m_StartDay);

        return ret;
    }

    bool ReportMalariaFiltered::Validate( const ISimulationContext *parent_sim )
    {
        if( m_StartDay >= m_EndDay )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Start_Day", m_StartDay, "End_Day", m_EndDay );
        }

        float sim_time_end = parent_sim->GetParams()->sim_time_start + parent_sim->GetParams()->sim_time_total;
        if (m_StartDay > sim_time_end)
        {
            throw IncoherentConfigurationException(__FILE__, __LINE__, __FUNCTION__, "Start_Day", m_StartDay, "Start_Time + Simulation_Duration", sim_time_end);
        }

        return true;
    }

    void ReportMalariaFiltered::Initialize( unsigned int nrmSize )
    {
        ReportMalaria::Initialize( nrmSize );

        // ----------------------------------------------------------------------------------
        // --- We only want to normalize on the number of nodes included in the report.
        // --- If m_NodesToInclude.size() == 0, then we are doing all nodes in the simulation
        // ----------------------------------------------------------------------------------
        if( m_NodesToInclude.size() > 0 )
        {
            _nrmSize = m_NodesToInclude.size();
        }
        release_assert( _nrmSize );
    }

    void ReportMalariaFiltered::UpdateEventRegistration( float currentTime, 
                                                         float dt, 
                                                         std::vector<INodeEventContext*>& rNodeEventContextList,
                                                         ISimulationEventContext* pSimEventContext )
    {
        m_IsValidDay = (m_StartDay <= currentTime) && (currentTime <= m_EndDay);
    }

    void ReportMalariaFiltered::BeginTimestep()
    {
        if( m_IsValidDay )
        {
            ReportMalaria::BeginTimestep();
        }
    }

    void ReportMalariaFiltered::LogNodeData( INodeContext* pNC )
    {
        if( m_IsValidDay && IsValidNode( pNC->GetExternalID() ) )
        {
            ReportMalaria::LogNodeData( pNC );
        }
    }

    void ReportMalariaFiltered::LogIndividualData( IIndividualHuman* individual ) 
    {
        if( m_IsValidDay && IsValidNode( individual->GetParent()->GetExternalID() ) )
        {
            ReportMalaria::LogIndividualData( individual );
        }
    }

    bool ReportMalariaFiltered::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return m_IsValidDay;
    }

    bool ReportMalariaFiltered::IsValidNode( uint32_t externalNodeID ) const
    {
        bool valid = (m_NodesToInclude.size() == 0) || (m_NodesToInclude.count( externalNodeID ) > 0);
        return valid;
    }
}
