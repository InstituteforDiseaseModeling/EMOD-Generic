/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <iostream>
#include <queue>
#include <iomanip> //setw(), setfill()
#include <algorithm>
#include <deque>
#include <functional>
#include <memory>

#include "FileSystem.h"
#include "Debug.h"
#include "Log.h"
#include "suids.hpp"
#include "ConfigParams.h"
#include "SimulationFactory.h"
#include "Simulation.h"
#include "IdmMpi.h"

#include "StatusReporter.h"

#ifndef _DLLS_
#include "SimulationMalaria.h"
#ifdef ENABLE_POLIO
#include "SimulationPolio.h"
#endif

#include "SimulationTBHIV.h"
#endif // _DLLS_
#include "ControllerFactory.h"

#include "SerializedPopulation.h"
#include "SerializationParameters.h"

#pragma warning(disable : 4244)

using namespace Kernel;

SETUP_LOGGING( "Controller" )


void CheckMissingParameters()
{
    if( !JsonConfigurable::missing_parameters_set.empty() )
    {
        std::stringstream errMsg;
        errMsg << "The following necessary parameters were not specified" << std::endl;
        for (auto& key : JsonConfigurable::missing_parameters_set)
        {
            errMsg << "\t \"" << key.c_str() << "\"" << std::endl;
        }
        //LOG_ERR( errMsg.str().c_str() );
        throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, errMsg.str().c_str() );
    }
}


// Basic simulation main loop with reporting
template <class SimulationT> void RunSimulation(SimulationT &sim, int steps)
{
    LOG_DEBUG( "RunSimulation\n" );

    bool use_full_precision = (SerializationParameters::GetInstance()->GetPrecision() == SerializationPrecision::FULL);

    float start_time = sim.GetParams()->sim_time_start;
    float step_size  = sim.GetParams()->sim_time_delta;

    // Calculate the sorted set of time steps to serialize
    std::deque< int32_t > serialization_time_steps = SerializationParameters::GetInstance()->GetSerializedTimeSteps(steps, start_time, step_size);

    for (int t = 0; t < steps; t++)
    {
        if (!serialization_time_steps.empty() && t == serialization_time_steps.front())
        {
            SerializedState::SaveSerializedSimulation(dynamic_cast<Simulation*>(&sim), t, true);
            serialization_time_steps.pop_front();
        }

        sim.Update();
        EnvPtr->Log->Flush();

        if (EnvPtr->MPI.Rank == 0)
        {
            EnvPtr->getStatusReporter()->ReportProgress(t+1, steps);
        }

        if(sim.TimeToStop())
        {
            break;
        }
    }

    // Serialize the final state if it was requested
    bool final_in_list = std::find(serialization_time_steps.begin(), serialization_time_steps.end(), steps) != serialization_time_steps.end();
    if (!serialization_time_steps.empty() && final_in_list)
    {
        SerializedState::SaveSerializedSimulation(dynamic_cast<Simulation*>(&sim), steps, true);
    }
}


bool DefaultController::execute_internal()
{
    using namespace Kernel;
    list<string> serialization_test_state_filenames;

    LOG_INFO("DefaultController::execute_internal()...\n");

    JsonConfigurable::_useDefaults = false;
    JsonConfigurable::_track_missing = true;
    SerializationParameters::GetInstance()->Configure( EnvPtr->Config );  // Has to be configured before CreateSimulation()
    CheckMissingParameters();

#ifdef _DLLS_
    ISimulation* sim = SimulationFactory::CreateSimulation(); 
    release_assert(sim);
#else
    std::unique_ptr<ISimulation> sim(SimulationFactory::CreateSimulation());
    if(!sim.get())
    {
        throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "sim.get() returned NULL after call to CreateSimulation.\n" );
    }
#endif // End of _DLLS_

    if (EnvPtr->MPI.Rank==0) { ostringstream oss; oss << "Beginning Simulation...";  EnvPtr->getStatusReporter()->ReportStatus(oss.str()); }

    // populate it
    LOG_INFO("DefaultController::execute_internal() populate simulation...\n");
    // Confusing variable name (JC::useDefaults); we want to collect all defaults for reporting. It's up to us as the calling function
    JsonConfigurable::_track_missing = true;
    if(sim->Populate())
    {
        // Need to reset back to false; will be set as appropriate by campaign related code after this based on
        // "Use_Defaults" in campaign.json.
        JsonConfigurable::_useDefaults = false;
        CheckMissingParameters();
        // now try to run it
        // divide the simulation into stages according to requesting number of serialization test cycles
        int simulation_steps = static_cast<int>( sim->GetParams()->sim_time_total / sim->GetParams()->sim_time_delta );

#ifndef _DLLS_
        int remaining_steps = simulation_steps;

        for (int k= 0; remaining_steps > 0; k++)
        {
            int cycle_steps = min(remaining_steps, max(1, simulation_steps));
            if (cycle_steps > 0)
                RunSimulation(*sim, cycle_steps);

            remaining_steps -= cycle_steps;
        }
#else // _DLLS_
        LOG_INFO( "Execute_internal(): Calling RunSimulation.\n" );
        RunSimulation(*sim, simulation_steps);
#endif
        sim->WriteReportsData();

        if (EnvPtr->MPI.Rank==0)
        {
            LogTimeInfo lti;
            EnvPtr->Log->GetLogInfo(lti);

            ostringstream oss;
            oss << "Done - " << lti.hours << ":" << setw(2) << setfill('0') << lti.mins << ":" << setw(2) << setfill('0') << lti.secs;
            EnvPtr->getStatusReporter()->ReportStatus(oss.str());
        }

        // cleanup serialization test state files
        for (auto& filename : serialization_test_state_filenames)
        {
            if( FileSystem::FileExists( filename ) )
                FileSystem::RemoveFile( filename );
        }

        LOG_INFO_F( "Exiting %s\n", __FUNCTION__ );

        return true;
    }
    return false;
}


bool DefaultController::Execute()
{
    return execute_internal();
}
