/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Infection.h"

namespace Kernel
{
    class InfectionEnvironmentalConfig : public InfectionConfig
    {
    public:
        virtual bool Configure( const Configuration* config ) override;

    protected:
        friend class InfectionEnvironmental;
    };

    class InfectionEnvironmental : public Infection
    {
    public:
        static InfectionEnvironmental *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~InfectionEnvironmental(void);

    protected:
        InfectionEnvironmental(IIndividualHumanContext *context);
        InfectionEnvironmental();

        DECLARE_SERIALIZABLE(InfectionEnvironmental);
    };
}
