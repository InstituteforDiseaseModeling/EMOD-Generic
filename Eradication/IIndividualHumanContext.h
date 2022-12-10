/***************************************************************************************************

Copyright (c) 2017 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

// put all contexts in one place to reduce clutter in includes
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
    struct IIndividualHumanInterventionsContext;
    struct IIndividualHumanEventContext;
    struct IIndividualHumanSTI;
    struct IIndividualHumanHIV;
    struct IIndividualHumanTB;
    struct IMalariaHumanContext;
    class  IIndividualHumanPolio;
    struct ISusceptibilityContext;
    struct INodeContext;
    struct NodeDemographics;
    struct IInfection;
    struct IVaccineConsumer;

    struct IIndividualHumanContext : ISupports
    {
        virtual const AgentParams* GetParams() const = 0;

        virtual suids::suid GetSuid() const = 0;
        virtual float GetAge() const = 0;

        virtual suids::suid GetNextInfectionSuid() = 0;
        virtual RANDOMBASE* GetRng() = 0;

        virtual IIndividualHumanInterventionsContext*  GetInterventionsContext()                                   const = 0; // internal components of individuals interact with interventions via this interface
        virtual IIndividualHumanSTI*                   GetIndividualSTI()                                                = 0;
        virtual IIndividualHumanHIV*                   GetIndividualHIV()                                                = 0;
        virtual IIndividualHumanTB*                    GetIndividualTB()                                                 = 0;
        virtual IMalariaHumanContext*                  GetIndividualMalaria()                                            = 0;
        virtual IIndividualHumanPolio*                 GetIndividualPolio()                                              = 0;
        virtual IVaccineConsumer*                      GetVaccineContext()                                         const = 0;
        virtual IIndividualHumanEventContext*          GetEventContext()                                                 = 0; // access to specific attributes of the individual useful for events
        virtual ISusceptibilityContext*                GetSusceptibilityContext()                                  const = 0; // access to immune attributes useful for infection, interventions, reporting, etc.

        virtual float GetImmunityReducedAcquire() const = 0;
        virtual float GetInterventionReducedAcquire() const = 0;

        virtual INodeContext* GetParent() const = 0;

        virtual void UpdateGroupMembership() = 0;
        virtual void UpdateGroupPopulation( float size_changes ) = 0;

        virtual const std::string& GetPropertyReportString() const = 0;
        virtual void SetPropertyReportString( const std::string& str ) = 0;
    };
}
