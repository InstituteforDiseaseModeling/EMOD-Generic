/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#if defined(ENABLE_ENVIRONMENTAL)

#include "Sugar.h"
#include "InfectionEnvironmental.h"
#include "IIndividualHumanContext.h"

SETUP_LOGGING( "InfectionEnvironmental" )

namespace Kernel
{
    bool InfectionEnvironmentalConfig::Configure( const Configuration * config )
    {
        LOG_DEBUG("Configure\n");

        bool bRet = JsonConfigurable::Configure( config );
        return bRet;
    }

    InfectionEnvironmental::InfectionEnvironmental()
    { }

    InfectionEnvironmental::~InfectionEnvironmental(void)
    { }

    InfectionEnvironmental::InfectionEnvironmental(IIndividualHumanContext *context) : Kernel::Infection(context)
    { }

    InfectionEnvironmental *InfectionEnvironmental::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        InfectionEnvironmental *newinfection = _new_ InfectionEnvironmental(context);
        newinfection->Initialize(_suid);

        return newinfection;
    }

    REGISTER_SERIALIZABLE(InfectionEnvironmental);

    void InfectionEnvironmental::serialize(IArchive& ar, InfectionEnvironmental* obj)
    {
        Infection::serialize(ar, obj);
    }
}

#endif // ENABLE_ENVIRONMENTAL
