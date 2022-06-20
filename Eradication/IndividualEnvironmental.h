/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "stdafx.h"

#include "Individual.h"

namespace Kernel
{
    class IndividualHumanEnvironmentalConfig : public IndividualHumanConfig
    {
        friend class IndividualHumanEnvironmental; 
    public:
        virtual bool Configure( const Configuration* config ) override;
    };

    class IndividualHumanEnvironmental : public IndividualHuman
    {
        friend class SimulationEnvironmental;

        DECLARE_QUERY_INTERFACE()
        DECLARE_SERIALIZABLE( IndividualHumanEnvironmental )

    public:
        static IndividualHumanEnvironmental *CreateHuman(INodeContext *context, suids::suid _suid, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0);
        virtual ~IndividualHumanEnvironmental(void);

        virtual void CreateSusceptibility(float = 1.0, float = 1.0) override;
        virtual void UpdateInfectiousness(float dt) override;

    protected:
        IndividualHumanEnvironmental( suids::suid id = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0);

        // Factory methods
        virtual IInfection* createInfection(suids::suid _suid) override;
        virtual void AcquireNewInfection( const IStrainIdentity* infstrain, TransmissionRoute::Enum tx_route, float incubation_period_override ) override;

    private:
    };
}
