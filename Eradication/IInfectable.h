/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "ISupports.h"
#include "SimulationEnums.h"
#include "IContagionPopulation.h"
#include "IInfection.h"

namespace Kernel
{
    struct IIndividualHuman;

    struct IInfectable : ISupports
    {
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum tx_route=TransmissionRoute::CONTACT ) = 0;
        virtual const infection_list_t& GetInfections() const = 0;
        virtual float GetInterventionReducedAcquire() const = 0;

        virtual IIndividualHuman* GetIndividual() = 0;

        virtual ~IInfectable() {}
    };

    struct IInfectionAcquirable : ISupports
    {
        virtual void AcquireNewInfection( const IStrainIdentity* infstrain=nullptr, TransmissionRoute::Enum tx_route=TransmissionRoute::CONTACT, float incubation_period_override=-1.0f ) = 0;

        virtual ~IInfectionAcquirable() {}
    };
}
