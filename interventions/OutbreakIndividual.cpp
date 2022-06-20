/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "OutbreakIndividual.h"
#include "IIndividualHuman.h"
#include "IIndividualHumanContext.h"
#include "ISimulationContext.h"
#include "InterventionFactory.h"
#include "NodeEventContext.h"
#include "Exceptions.h"
#include "StrainIdentity.h"
#include "IdmString.h"
#include "RANDOM.h"

SETUP_LOGGING( "OutbreakIndividual" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(OutbreakIndividual)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(IOutbreakIndividual)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(OutbreakIndividual)

    IMPLEMENT_FACTORY_REGISTERED(OutbreakIndividual)

    OutbreakIndividual::OutbreakIndividual()
        : clade(0)
        , genome(0)
        , ignoreImmunity( true )
        , incubation_period_override(-1.0f)
    {
        // Schema documentation
        initSimTypes( 11, "GENERIC_SIM", "VECTOR_SIM", "MALARIA_SIM", "AIRBORNE_SIM", "POLIO_SIM", "TBHIV_SIM", "STI_SIM", "HIV_SIM", "PY_SIM", "TYPHOID_SIM", "ENVIRONMENTAL_SIM" );
    }

    bool OutbreakIndividual::Configure(const Configuration * inputJson)
    {
        const std::map<std::string, std::string> dset_sim01 {{"Simulation_Type","GENERIC_SIM,VECTOR_SIM,AIRBORNE_SIM,POLIO_SIM,TBHIV_SIM,STI_SIM,HIV_SIM,PY_SIM,DENGUE_SIM,MALARIA_SIM"}};

        initConfigTypeMap("Ignore_Immunity",             &ignoreImmunity,              OB_Ignore_Immunity_DESC_TEXT,           true);
        initConfigTypeMap("Clade",                       &clade,                       Clade_DESC_TEXT,                        0,          9,  0);
        initConfigTypeMap("Genome",                      &genome,                      Genome_DESC_TEXT,                       0,  MAX_24BIT,  0,   nullptr, nullptr, &dset_sim01);
        initConfigTypeMap("Incubation_Period_Override",  &incubation_period_override,  Incubation_Period_Override_DESC_TEXT,  -1.0f, FLT_MAX, -1.0f);
        initConfigTypeMap("Cost_To_Consumer",            &cost_per_unit,               IV_Cost_To_Consumer_DESC_TEXT,          0.0f, FLT_MAX,  0.0f);

        return BaseIntervention::Configure(inputJson);
    }

    bool OutbreakIndividual::Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * pCCO)
    {
        bool distributed = BaseIntervention::Distribute(context, pCCO);

        StrainIdentity outbreak_strain(clade,genome);

        // TBD: Get individual from context, and infect
        IIndividualHuman* individual = dynamic_cast<IIndividualHuman*>(context->GetParent()); // QI in new code

        float mod_acq = individual->GetImmunityReducedAcquire()*individual->GetInterventionReducedAcquire();
        LOG_DEBUG( "Infecting individual from Outbreak.\n" );
        if( ignoreImmunity || context->GetParent()->GetRng()->SmartDraw(mod_acq) )
        {
            individual->AcquireNewInfection(&outbreak_strain, TransmissionRoute::OUTBREAK, incubation_period_override);
            distributed = true;
        }
        else
        {
            LOG_DEBUG_F( "We didn't infect individual %d with immunity %f (ignore=%d).\n", individual->GetSuid().data, mod_acq, ignoreImmunity );
        }

        return distributed;
    }

    void OutbreakIndividual::Update( float dt )
    {
        expired = true;

        return;
    }
}
