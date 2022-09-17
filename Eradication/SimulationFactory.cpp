/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#pragma warning(disable:4996)

#ifdef WIN32
#include "windows.h"
#endif
#include "Debug.h"
#include "Environment.h"
#include "Configuration.h"
#include "EventTrigger.h"
#include "ConfigParams.h"

#include "Simulation.h"
#include "SimulationFactory.h"
#include "NodeProperties.h"
#include "Properties.h"

#ifndef _DLLS_
#ifndef DISABLE_MALARIA
#include "SimulationMalaria.h"
#endif
#ifndef DISABLE_VECTOR
#include "SimulationVector.h"
#endif
#ifdef ENABLE_ENVIRONMENTAL
#include "SimulationEnvironmental.h"
#endif
#ifdef ENABLE_POLIO
#include "SimulationPolio.h"
#endif
#ifdef ENABLE_TYPHOID
#include "SimulationTyphoid.h"
#endif

#ifndef DISABLE_AIRBORNE
#include "SimulationAirborne.h"
#endif
#ifndef DISABLE_TBHIV
#include "SimulationTBHIV.h"
#endif
#ifndef DISABLE_STI
#include "SimulationSTI.h"
#endif
#ifndef DISABLE_HIV
#include "SimulationHIV.h"
#endif
#ifdef ENABLE_DENGUE
#include "SimulationDengue.h"
#endif
#endif

#ifdef ENABLE_PYTHON_FEVER
#include "SimulationPy.h"
#endif

#include "SerializedPopulation.h"

#include <chrono>
#include "EventTrigger.h"
#include "EventTriggerNode.h"
#include "EventTriggerCoordinator.h"
#include "RandomNumberGeneratorFactory.h"
#include "SerializationParameters.h"

SETUP_LOGGING( "SimulationFactory" )

namespace Kernel
{
    ISimulation * SimulationFactory::CreateSimulation()
    {
        NPFactory::CreateFactory();
        IPFactory::CreateFactory();

        ISimulation* newsim = nullptr;

        if ( SerializationParameters::GetInstance()->GetSerializedPopulationReadingType() != SerializationTypeRead::NONE )
        {
            const std::string population_filename = SerializationParameters::GetInstance()->GetSerializedPopulationFilename();

            auto t_start = std::chrono::high_resolution_clock::now();
            newsim = SerializedState::LoadSerializedSimulation( population_filename.c_str() );
            auto t_finish = std::chrono::high_resolution_clock::now();
            newsim->Initialize( EnvPtr->Config );
            double elapsed = uint64_t((t_finish - t_start).count()) * 1000 * double(std::chrono::high_resolution_clock::period::num) / double(std::chrono::high_resolution_clock::period::den);
            LOG_INFO_F( "Loaded serialized population from '%s' in %f ms\n.", population_filename.c_str(), elapsed );
            return newsim;
        }

        std::string sSimType = SimType::pairs::lookup_key(SimConfig::GetSimParams()->sim_type);

        try
        {
#ifdef _DLLS_
            // Look through disease dll directory, do LoadLibrary on each .dll,
            // do GetProcAddress on GetMimeType() and CreateSimulation
            typedef ISimulation* (*createSim)(const Environment *);
            std::map< std::string, createSim > createSimFuncPtrMap;

            // Note map operator [] will automatically initialize the pointer to NULL if not found
            DllLoader dllLoader;         
            if (!dllLoader.LoadDiseaseDlls(createSimFuncPtrMap) || !createSimFuncPtrMap[sSimType])
            {
                std::ostringstream msg;
                msg << "Failed to load disease emodules for SimType: " << sSimType << " from path: " << dllLoader.GetEModulePath(DISEASE_EMODULES).c_str() << std::endl;
                throw Kernel::DllLoadingException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str());
                return newsim;
            }
            newsim = createSimFuncPtrMap[sSimType](EnvPtr);
#else // _DLLS_
            switch (SimConfig::GetSimParams()->sim_type)
            {
                case SimType::GENERIC_SIM:
                    newsim = Simulation::CreateSimulation(EnvPtr->Config);
                break;
#if defined(ENABLE_ENVIRONMENTAL)
                case SimType::ENVIRONMENTAL_SIM:
                    throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "ENVIRONMENTAL_SIM currently disabled. Consider using GENERIC_SIM." );
                break;
#endif
#if defined( ENABLE_POLIO)
                case SimType::POLIO_SIM:
                    newsim = SimulationPolio::CreateSimulation(EnvPtr->Config);
                break;
#endif        
#if defined( ENABLE_TYPHOID)
                case SimType::TYPHOID_SIM:
                    newsim = SimulationTyphoid::CreateSimulation(EnvPtr->Config);
                break;
#endif        
#ifndef DISABLE_VECTOR
                case SimType::VECTOR_SIM:
                    newsim = SimulationVector::CreateSimulation(EnvPtr->Config);
                break;
#endif
#ifndef DISABLE_MALARIA
                case SimType::MALARIA_SIM:
                    newsim = SimulationMalaria::CreateSimulation(EnvPtr->Config);
                break;
#endif
#ifndef DISABLE_AIRBORNE
                case SimType::AIRBORNE_SIM:
                    throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "AIRBORNE_SIM currently disabled. Consider using GENERIC_SIM." );
#endif
#ifndef DISABLE_TBHIV
                case SimType::TBHIV_SIM:
                    newsim = SimulationTBHIV::CreateSimulation(EnvPtr->Config);
                break;
#endif // TBHIV
#ifndef DISABLE_STI
                case SimType::STI_SIM:
                    newsim = SimulationSTI::CreateSimulation(EnvPtr->Config);
                break;
#endif
#ifndef DISABLE_HIV 
                case SimType::HIV_SIM:
                    newsim = SimulationHIV::CreateSimulation(EnvPtr->Config);
                break;
#endif // HIV
#ifdef ENABLE_DENGUE
                case SimType::DENGUE_SIM:
                    newsim = SimulationDengue::CreateSimulation(EnvPtr->Config);
                break;
#endif 
#ifdef ENABLE_PYTHON_FEVER 
                case SimType::PY_SIM:
                    newsim = SimulationPy::CreateSimulation(EnvPtr->Config);
                break;
#endif
                default: 
                    std::ostringstream msg;
                    msg << "Simulation_Type " << sSimType << " not recognized." << std::endl;
                    throw Kernel::GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
                break;
            }
#endif
            release_assert(newsim);
        }
        catch ( GeneralConfigurationException& e ) {
            throw e;
        }

        return newsim;
    }
}
