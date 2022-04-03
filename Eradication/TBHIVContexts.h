/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "ISupports.h"
#include "Common.h"

namespace Kernel
{
    struct NodeDemographicsDistribution;
    class IndividualHuman;

    struct INodeTBHIV : public ISupports
    {
        virtual const NodeDemographicsDistribution* GetHIVCoinfectionDistribution() const = 0;
        virtual const NodeDemographicsDistribution* GetHIVMortalityDistribution()   const = 0;
        virtual void SetNewInfectionState(InfectionStateChange::_enum inf_state_change, IndividualHuman *ih) = 0;
    };
}
