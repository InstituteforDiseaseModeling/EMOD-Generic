/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "ISusceptibilityContext.h"
#include "suids.hpp"
#include "SimulationEnums.h"
#include "Common.h"             // InfectionStateChange
#include "Types.h"              // NonNegativeFloat

namespace Kernel
{
    struct IIndividualHumanContext;
    struct IStrainIdentity;
    class IInfectionMalaria;

    struct IInfection : ISerializable
    {
        virtual suids::suid GetSuid() const = 0;
        virtual void Update( float, ISusceptibilityContext* = nullptr ) = 0;
        virtual InfectionStateChange::_enum GetStateChange() const = 0;
        virtual float GetInfectiousness() const = 0;
        virtual float GetInfectiousnessByRoute(TransmissionRoute::Enum) const = 0;

        virtual TransmissionRoute::Enum GetSourceRoute() const = 0;

        virtual void GetInfectiousStrainID( IStrainIdentity* ) = 0;
        virtual const IStrainIdentity* GetStrain() const  = 0;

        virtual IInfectionMalaria*        GetInfectionMalaria()                 = 0;

        virtual bool IsActive() const = 0;
        virtual NonNegativeFloat GetDuration() const = 0;
        virtual void SetContextTo(IIndividualHumanContext*) = 0;
        virtual void SetParameters( IStrainIdentity* infstrain=nullptr, float incubation_period_override=-1.0f, TransmissionRoute::Enum tx_route=TransmissionRoute::CONTACT ) = 0;
        virtual void InitInfectionImmunology(ISusceptibilityContext* _immunity) = 0;
        virtual bool StrainMatches( IStrainIdentity * pStrain ) = 0;
        virtual bool IsSymptomatic() const = 0;
        virtual bool IsNewlySymptomatic() const = 0;

        virtual ~IInfection() {}
    };

    typedef std::list<IInfection*> infection_list_t;
}
