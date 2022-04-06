/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Susceptibility.h"

namespace Kernel
{
    class SusceptibilityEnvironmentalConfig : public SusceptibilityConfig
    {
    protected:
        friend class SusceptibilityEnvironmental;
    };

    class SusceptibilityEnvironmental : public Susceptibility
    {
    public:
        static SusceptibilityEnvironmental *CreateSusceptibility(IIndividualHumanContext *context, float immmod, float riskmod);
        virtual ~SusceptibilityEnvironmental(void);

    protected:
        SusceptibilityEnvironmental();
        SusceptibilityEnvironmental(IIndividualHumanContext *context);
        virtual void Initialize(float immmod, float riskmod) override;

        DECLARE_SERIALIZABLE(SusceptibilityEnvironmental);
    };
}
