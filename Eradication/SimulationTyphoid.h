/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "SimulationEnvironmental.h"
#include "IndividualTyphoid.h"
#include "NodeTyphoid.h"
#include "InfectionTyphoid.h"
#include "SusceptibilityTyphoid.h"

namespace Kernel
{
    class NodeTyphoid;
    class IndividualHumanTyphoid;

    class SimulationTyphoid : public SimulationEnvironmental
    {
    public:
        static SimulationTyphoid *CreateSimulation();
        static SimulationTyphoid *CreateSimulation(const ::Configuration *config);
        virtual ~SimulationTyphoid(void) { }
        virtual void Reports_CreateBuiltIn();

    protected:
        SimulationTyphoid();

        virtual void Initialize() override;
        virtual void Initialize(const ::Configuration *config) override;

        virtual bool ValidateConfiguration(const ::Configuration *config) override;

        // Allows correct type of community to be added by derived class Simulations
        virtual void addNewNodeFromDemographics( ExternalNodeId_t externalNodeId,
                                                 suids::suid node_suid,
                                                 NodeDemographicsFactory *nodedemographics_factory, 
                                                 ClimateFactory *climate_factory ) override;

        virtual void resolveMigration();

    private:
        friend class Kernel::SimulationFactory; // allow them to create us
    };
}
