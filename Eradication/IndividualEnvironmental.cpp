/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#if defined(ENABLE_ENVIRONMENTAL)

#include "NodeEnvironmental.h"
#include "IndividualEnvironmental.h"
#include "InfectionEnvironmental.h"
#include "SusceptibilityEnvironmental.h"
#include "StrainIdentity.h"

#pragma warning(disable: 4244)

SETUP_LOGGING( "IndividualEnvironmental" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanEnvironmental, IndividualHuman)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanEnvironmental, IndividualHuman)

    IndividualHumanEnvironmental::IndividualHumanEnvironmental( suids::suid _suid, float monte_carlo_weight, float initial_age, int gender) 
        : IndividualHuman( _suid, monte_carlo_weight, initial_age, gender)
    {
    }

    IndividualHumanEnvironmental* IndividualHumanEnvironmental::CreateHuman(INodeContext *context, suids::suid id, float MCweight, float init_age, int gender)
    {
        IndividualHumanEnvironmental *newindividual = _new_ IndividualHumanEnvironmental( id, MCweight, init_age, gender);

        newindividual->SetContextTo(context);

        return newindividual;
    }

    IndividualHumanEnvironmental::~IndividualHumanEnvironmental()
    {
    }

    void IndividualHumanEnvironmental::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        susceptibility = SusceptibilityEnvironmental::CreateSusceptibility(this, imm_mod, risk_mod);
    }

    IInfection* IndividualHumanEnvironmental::createInfection( suids::suid _suid )
    {
        return InfectionEnvironmental::CreateInfection(this, _suid);
    }

    REGISTER_SERIALIZABLE(IndividualHumanEnvironmental);

    void IndividualHumanEnvironmental::serialize(IArchive& ar, IndividualHumanEnvironmental* obj)
    {
        IndividualHuman::serialize(ar, obj);
    }
}

#endif // ENABLE_ENVIRONMENTAL
