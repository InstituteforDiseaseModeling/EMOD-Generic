/***************************************************************************************************

Copyright (c) 2017 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include <list>
#include <vector>

#include "suids.hpp"
#include "ISupports.h"
#include "IdmApi.h"

namespace Kernel
{
    class RANDOMBASE;
    struct AgentParams;
    struct IIndividualHuman;
    struct IIndividualHumanInterventionsContext;
    struct IIndividualHumanEventContext;
    struct IIndividualHumanSTI;
    struct IIndividualHumanHIV;
    struct IIndividualHumanTB;
    struct IMalariaHumanContext;
    class  IIndividualHumanPolio;
    class  IIndividualHumanTyphoid;
    struct ISusceptibilityContext;
    struct INodeContext;
    struct IVaccineConsumer;

    struct IIndividualHumanContext : ISupports
    {
        virtual const AgentParams* GetParams() const = 0;

        virtual suids::suid GetSuid() const = 0;
        virtual float GetAge() const = 0;
        virtual suids::suid GetNextInfectionSuid() = 0;
        virtual RANDOMBASE* GetRng() = 0;

        virtual IIndividualHuman*                      GetIndividual()                                                   = 0;
        virtual IIndividualHumanSTI*                   GetIndividualSTI()                                                = 0;
        virtual IIndividualHumanHIV*                   GetIndividualHIV()                                                = 0;
        virtual IIndividualHumanTB*                    GetIndividualTB()                                                 = 0;
        virtual IMalariaHumanContext*                  GetIndividualMalaria()                                            = 0;
        virtual IIndividualHumanPolio*                 GetIndividualPolio()                                              = 0;
        virtual IIndividualHumanTyphoid*               GetIndividualTyphoid()                                            = 0;
        virtual IIndividualHumanInterventionsContext*  GetInterventionsContext()                                   const = 0; // internal components of individuals interact with interventions via this interface
        virtual IVaccineConsumer*                      GetVaccineContext()                                         const = 0;
        virtual IIndividualHumanEventContext*          GetEventContext()                                                 = 0; // access to specific attributes of the individual useful for events
        virtual ISusceptibilityContext*                GetSusceptibilityContext()                                  const = 0; // access to immune attributes useful for infection, interventions, reporting, etc.

        virtual INodeContext* GetParent() const = 0;
    };
}
