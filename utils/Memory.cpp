/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <iostream>
#include "Memory.h"
#include "Log.h"
#include "Debug.h"

#ifdef WIN32
#include <windows.h>
#include <Psapi.h>
#endif

SETUP_LOGGING( "Memory" )

namespace Kernel
{
    void*    MemoryGauge::m_ProcessHandle       = nullptr;
    uint64_t MemoryGauge::m_WorkingSetWarningMB = 7000;
    uint64_t MemoryGauge::m_WorkingSetHaltMB    = 8000;
    uint64_t MemoryGauge::m_LastPeakSizeMB      =    0;

    GET_SCHEMA_STATIC_WRAPPER_IMPL( MemoryGauge, MemoryGauge )

    MemoryGauge::MemoryGauge()
    { }

    MemoryGauge::~MemoryGauge()
    { }

    bool MemoryGauge::Configure( const Configuration* inputJson )
    {
        if( JsonConfigurable::_dryrun || inputJson->Exist( "Memory_Usage_Warning_Threshold_Working_Set_MB" ) )
        {
            initConfigTypeMap( "Memory_Usage_Warning_Threshold_Working_Set_MB", (int*)&m_WorkingSetWarningMB, Memory_Usage_Warning_Threshold_Working_Set_MB_DESC_TEXT, 0, 1000000, 7000 );
        }

        if( JsonConfigurable::_dryrun || inputJson->Exist( "Memory_Usage_Halting_Threshold_Working_Set_MB" ) )
        {
            initConfigTypeMap( "Memory_Usage_Halting_Threshold_Working_Set_MB", (int*)&m_WorkingSetHaltMB, Memory_Usage_Halting_Threshold_Working_Set_MB_DESC_TEXT, 0, 1000000, 8000 );
        }

        bool retValue = JsonConfigurable::Configure( inputJson );
        if( retValue )
        {
            if( m_WorkingSetHaltMB < m_WorkingSetWarningMB )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                    "Memory_Usage_Warning_Threshold_Working_Set_MB", (unsigned long)(m_WorkingSetWarningMB),
                    "Memory_Usage_Halting_Threshold_Working_Set_MB", (unsigned long)(m_WorkingSetHaltMB),
                    "\nThe Warning WorkingSet threshold must be smaller than the Halting WorkingSet threshold." );
            }
        }
        return retValue;
    }

    void MemoryGauge::CheckMemoryFailure( bool onlyCheckForFailure )
    {
        uint64_t current_working_set_mb      = 0;
        uint64_t current_peak_working_set_mb = 0;

#ifdef WIN32
        // Windows syntax
        if( m_ProcessHandle == nullptr )
        {
            m_ProcessHandle = GetCurrentProcess();
        }
        release_assert( m_ProcessHandle );

        PROCESS_MEMORY_COUNTERS_EX meminfo;
        if( !GetProcessMemoryInfo( m_ProcessHandle, (PROCESS_MEMORY_COUNTERS*)&meminfo, sizeof( meminfo ) ) )
        {
            LOG_WARN( "Unable to get memory information.  No checking will be done.\n" );
            return;
        }

        MEMORYSTATUSEX memstatus;
        memstatus.dwLength = sizeof( memstatus );

        if( !GlobalMemoryStatusEx( &memstatus ) )
        {
            LOG_WARN( "Unable to get memory status.  No checking will be done.\n" );
            return;
        }

        current_working_set_mb      = meminfo.WorkingSetSize     >> 20;
        current_peak_working_set_mb = meminfo.PeakWorkingSetSize >> 20;
#else
        // Linux syntax: https://stackoverflow.com/questions/669438
        uint64_t tsize_val, res_val;

        ifstream buffer("/proc/self/statm");
        buffer >> tsize_val >> res_val;
        buffer.close();

        uint64_t page_size_kb = sysconf(_SC_PAGE_SIZE) >> 10;
        uint64_t rss_kb       = res_val * page_size_kb;

        current_working_set_mb      = rss_kb >> 10;
        if(current_working_set_mb > m_LastPeakSizeMB)
        {
            m_LastPeakSizeMB = current_working_set_mb;
        }
        current_peak_working_set_mb = m_LastPeakSizeMB;
#endif

        // Check if debug logging enabled
        bool           log_data  = false;
        Logger::tLevel log_level = Logger::DEBUG;
        if(!onlyCheckForFailure)
        {
            log_data = EnvPtr->Log->IsLoggingEnabled( log_level, _module, _log_level_enabled_array );
        }

        // Check if warning logging required
        if( m_WorkingSetWarningMB <= current_working_set_mb )
        {
            log_data = true;
            log_level = Logger::WARNING;
        }

        if( log_data )
        {
            EnvPtr->Log->Log( log_level, _module, "Working-set              : %uMB\n", current_working_set_mb );
            EnvPtr->Log->Log( log_level, _module, "Peak working-set         : %uMB\n", current_peak_working_set_mb );
        }

#ifdef WIN32
        // Extra Windows logging
        if( log_data )
        {
            EnvPtr->Log->Log( log_level, _module, "Pagefile Usage           : %uMB\n", meminfo.PagefileUsage      >> 20 );
            EnvPtr->Log->Log( log_level, _module, "Peak Pagefile Usage      : %uMB\n", meminfo.PeakPagefileUsage  >> 20 );
            EnvPtr->Log->Log( log_level, _module, "Private Usage            : %uMB\n", meminfo.PrivateUsage       >> 20 );
            EnvPtr->Log->Log( log_level, _module, "Page Fault Count         : %u\n",   meminfo.PageFaultCount );
            EnvPtr->Log->Log( log_level, _module, "Physical memory load     : %u%%\n", memstatus.dwMemoryLoad );
            EnvPtr->Log->Log( log_level, _module, "Available physical memory: %uMB\n", memstatus.ullAvailPhys     >> 20 );
            EnvPtr->Log->Log( log_level, _module, "Total physical memory    : %uMB\n", memstatus.ullTotalPhys     >> 20 );
            EnvPtr->Log->Log( log_level, _module, "Total Page File          : %uMB\n", memstatus.ullTotalPageFile >> 20 );
            EnvPtr->Log->Log( log_level, _module, "Available Page File      : %uMB\n", memstatus.ullAvailPageFile >> 20 );
        }
#endif

        // Check for memory error
        if( m_WorkingSetHaltMB <= current_working_set_mb )
        {
            std::stringstream ss;
            ss << "Current memory usage (WorkingSet = " << current_working_set_mb << " MB) exceeds limit of " << m_WorkingSetHaltMB << " MB.\n";
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        return;
    }

}
