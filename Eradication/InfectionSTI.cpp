/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "Debug.h"
#include "InfectionSTI.h"

SETUP_LOGGING( "InfectionSTI" )

namespace Kernel
{
    bool InfectionSTIConfig::Configure( const Configuration * config )
    {
        LOG_DEBUG("Configure\n");

        bool bRet = JsonConfigurable::Configure( config );
        return bRet;
    }

    InfectionSTI *InfectionSTI::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        InfectionSTI *newinfection = _new_ InfectionSTI(context);
        newinfection->Initialize(_suid);

        return newinfection;
    }

    InfectionSTI::~InfectionSTI(void) { }
    InfectionSTI::InfectionSTI() { }
    InfectionSTI::InfectionSTI(IIndividualHumanContext *context) : Infection(context) { }

    void InfectionSTI::Update(float dt, ISusceptibilityContext* immunity)
    {
        Infection::Update( dt, immunity );
    }

    REGISTER_SERIALIZABLE(InfectionSTI);

    void InfectionSTI::serialize(IArchive& ar, InfectionSTI* obj)
    {
        Infection::serialize( ar, obj );
        InfectionSTI& infection = *obj;
    }
}
