/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifndef DISABLE_AIRBORNE

#include "SusceptibilityAirborne.h"

namespace Kernel
{
    SusceptibilityAirborne *SusceptibilityAirborne::CreateSusceptibility(IIndividualHumanContext *context, float immmod, float riskmod)
    {
        SusceptibilityAirborne *newsusceptibility = _new_ SusceptibilityAirborne(context);
        newsusceptibility->Initialize(immmod, riskmod);

        return newsusceptibility;
    }

    SusceptibilityAirborne::~SusceptibilityAirborne(void) { }
    SusceptibilityAirborne::SusceptibilityAirborne() { }
    SusceptibilityAirborne::SusceptibilityAirborne(IIndividualHumanContext *context) : Susceptibility(context) { }

    void SusceptibilityAirborne::Initialize(float _immmod, float _riskmod)
    {
        Susceptibility::Initialize(_immmod, _riskmod);
    }

    REGISTER_SERIALIZABLE(SusceptibilityAirborne);

    void SusceptibilityAirborne::serialize(IArchive& ar, SusceptibilityAirborne* obj)
    {
        Susceptibility::serialize(ar, obj);
        SusceptibilityAirborne& susceptibility = *obj;
    }
}

#endif // DISABLE_AIRBORNE
