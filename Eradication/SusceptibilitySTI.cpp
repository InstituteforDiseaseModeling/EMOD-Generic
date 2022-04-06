/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "SusceptibilitySTI.h"

namespace Kernel
{
    SusceptibilitySTI *SusceptibilitySTI::CreateSusceptibility(IIndividualHumanContext *context, float immmod, float riskmod)
    {
        SusceptibilitySTI *newsusceptibility = _new_ SusceptibilitySTI(context);
        newsusceptibility->Initialize(immmod, riskmod);

        return newsusceptibility;
    }

    SusceptibilitySTI::~SusceptibilitySTI(void) { }
    SusceptibilitySTI::SusceptibilitySTI() { }
    SusceptibilitySTI::SusceptibilitySTI(IIndividualHumanContext *context) : Susceptibility(context) { }

    void SusceptibilitySTI::Initialize(float _immmod, float _riskmod)
    {
        Susceptibility::Initialize(_immmod, _riskmod);
    }

    REGISTER_SERIALIZABLE(SusceptibilitySTI);

    void SusceptibilitySTI::serialize(IArchive& ar, SusceptibilitySTI* obj)
    {
        Susceptibility::serialize( ar, obj );
        SusceptibilitySTI& susceptibility = *obj;
    }
}
