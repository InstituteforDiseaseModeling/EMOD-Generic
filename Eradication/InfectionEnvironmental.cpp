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

        // Environmental infections use Genome to represent transmission route.
        InfectionConfig::number_genomes = 1;

        bool bRet = JsonConfigurable::Configure( config );
        return bRet;
    }

    InfectionEnvironmental::InfectionEnvironmental()
    { }

    InfectionEnvironmental::~InfectionEnvironmental(void)
    { }

    void InfectionEnvironmental::SetParameters(IStrainIdentity* _infstrain, float incubation_period_override )
    {
        Infection::SetParameters( _infstrain, incubation_period_override );

        infectiousnessByRoute[string("environmental")]    = Infection::GetInfectiousness();
        infectiousnessByRoute[string("contact")]          = Infection::GetInfectiousness();
    }

    void InfectionEnvironmental::Update(float dt, ISusceptibilityContext* immunity)
    {
        Infection::Update(dt);

        //for Environmental, same infectiousness is deposited to both environmental and contact routes (if applicable)
        //By default (if HINT is off), only environmental route is available.
        infectiousnessByRoute[string("environmental")]    = Infection::GetInfectiousness();
        infectiousnessByRoute[string("contact")]          = Infection::GetInfectiousness();
    }

    InfectionEnvironmental::InfectionEnvironmental(IIndividualHumanContext *context) : Kernel::Infection(context)
    { }

    void InfectionEnvironmental::Initialize(suids::suid _suid)
    {
        Kernel::Infection::Initialize(_suid);
    }

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
        // nothing else, yet
    }
}

#endif // ENABLE_ENVIRONMENTAL
