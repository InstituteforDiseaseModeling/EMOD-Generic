/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportEventCounter.h"
#include "Environment.h"
#include "DllInterfaceHelper.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING( "ReportEventCounter" )

static const char* _sim_types[] = { "*", nullptr };

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
    return new Kernel::ReportEventCounter();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- ReportEventCounter Methods
// ----------------------------------------

namespace Kernel
{
    ReportEventCounter::ReportEventCounter()
        : BaseEventReport( "ReportEventCounter" )
        , channelDataMap()
        , unitsMap()
    {
        // ------------------------------------------------------------------------------------------------
        // --- Since this report will be listening for events, it needs to increment its reference count
        // --- so that it is 1.  Other objects will be AddRef'ing and Release'ing this report/observer
        // --- so it needs to start with a refcount of 1.
        // ------------------------------------------------------------------------------------------------
        AddRef();
    }

    ReportEventCounter::~ReportEventCounter()
    {
    }

    bool ReportEventCounter::Configure( const Configuration * inputJson )
    {
        bool ret = BaseEventReport::Configure( inputJson );

        if( ret )
        {
            const std::vector< EventTrigger::Enum >& trigger_list = GetEventTriggerList();
            for( auto trigger : trigger_list )
            {
                auto event_string = EventTrigger::pairs::lookup_key( trigger );
                unitsMap[ event_string ] = "" ;
                channelDataMap.AddChannel( event_string );
            }
        }
        return ret;
    }

    void ReportEventCounter::BeginTimestep()
    {
        channelDataMap.IncreaseChannelLength( 1 );
    }

    void ReportEventCounter::Reduce()
    {
        channelDataMap.Reduce();
        BaseEventReport::Reduce();
    }

    void ReportEventCounter::Finalize()
    {
        std::string output_fn = GetBaseOutputFilename() + ".json" ;
        channelDataMap.WriteOutput( output_fn, unitsMap );
        BaseEventReport::Finalize();
    }

    bool ReportEventCounter::notifyOnEvent( IIndividualHumanEventContext *context, 
                                            const EventTrigger::Enum& trigger )
    {
        if( HaveUnregisteredAllEvents() )
        {
            return false ;
        }

        channelDataMap.Accumulate( EventTrigger::pairs::lookup_key( trigger ), 1.0 );

        return true ;
    }

}
