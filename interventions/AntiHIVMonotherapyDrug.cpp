/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "AntiHIVMonotherapyDrug.h"

#include "HIVDrugTypeParameters.h"

#include "IIndividualHumanContext.h"

#define DAYSPERHOUR (0.0416666666666667f)
#define LOG2 (0.693147180559945f)

static const char* _module = "AntiHIVMonotherapyDrug";

namespace Kernel
{

    BEGIN_QUERY_INTERFACE_DERIVED(AntiHIVMonotherapyDrug, GenericDrug)
        HANDLE_INTERFACE(IHIVIntervention)
    END_QUERY_INTERFACE_DERIVED(AntiHIVMonotherapyDrug, GenericDrug)

    IMPLEMENT_FACTORY_REGISTERED(AntiHIVMonotherapyDrug)

    AntiHIVMonotherapyDrug::AntiHIVMonotherapyDrug()
    : GenericDrug()
    , ihivda(NULL)
    {
        initSimTypes( 2, "HIV_SIM", "TBHIV_SIM" );
    }

    AntiHIVMonotherapyDrug::~AntiHIVMonotherapyDrug()
    {
    }

    float AntiHIVMonotherapyDrug::GetDrugInactivationRate()
    {
        return 0;       // ART monotherapy does not inactivate HIV
    }

    float AntiHIVMonotherapyDrug::GetDrugClearanceRate()
    {
        return 0;       // ART monotherapy does not clear HIV
    }

    bool
    AntiHIVMonotherapyDrug::Configure(
        const Configuration * inputJson
    )
    {
        initConfigTypeMap( "Drug_Name", &drug_name, "Name of the HIV drug to distribute in a drugs intervention." );
        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, DRUG_Cost_To_Consumer_DESC_TEXT, 0, 999999, 10);

        return GenericDrug::Configure( inputJson );
    }

    bool
    AntiHIVMonotherapyDrug::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * pCCO
    )
    {
        if (s_OK != context->QueryInterface(GET_IID(IHIVDrugEffectsApply), (void**)&ihivda) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IHIVDrugEffectsApply", "IIndividualHumanInterventionsContext" );
        } 
        
        return GenericDrug::Distribute( context, pCCO ); //GHH do not use this because it uses the base class of interventionscontext, the TBInterventionsContainer!
    }

    void
    AntiHIVMonotherapyDrug::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IHIVDrugEffectsApply), (void**)&ihivda) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IHIVDrugEffectsApply", "IIndividualHumanContext" );
        } 

        return GenericDrug::SetContextTo( context );
    }

    void 
    AntiHIVMonotherapyDrug::ConfigureDrugTreatment( 
        IIndividualHumanInterventionsContext * ivc
    )
    {
        IHIVDrugEffects * hivde = NULL;
        if( ivc->QueryInterface( GET_IID(IHIVDrugEffects), (void**)&hivde ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "ivc", "IHIVDrugEffects", "IIndividualHumanInterventionsContext" );
        };
        auto hivdtMap = hivde->GetHIVdtParams();

        if( hivdtMap.find( drug_name ) == hivdtMap.end() )
        {
            throw BadMapKeyException( __FILE__, __LINE__, __FUNCTION__, "hivdtMap", drug_name.c_str() );
        }

        fast_decay_time_constant = hivdtMap[drug_name]->pkpd_halflife__hours * DAYSPERHOUR / LOG2;        // (1/rate)
        // Set secondary dtc to primary so that primary_fraction=0 and we get
        //  only drug_compartment1 in UpdateWithPkPd
        slow_decay_time_constant = fast_decay_time_constant;

        time_between_doses = hivdtMap[drug_name]->dose_interval__days;
        Cmax = hivdtMap[drug_name]->pkpd_Cmax__uMol;

        // All HIV drugs are for life.
        remaining_doses = -1;

        // No "defaulting" from HIV drugs
        fraction_defaulters = 0;

        // Get the drug class and nucleoside information for informational purposes
        hiv_drug_class = hivdtMap[drug_name]->hiv_drug_class;
        if( hiv_drug_class == HIVDrugClass::NucleosideReverseTranscriptaseInhibitor )
            nucleoside_analog = hivdtMap[drug_name]->nucleoside_analog;   // Used only for NRTI drugs to identify target

        IGlobalContext *pGC = NULL;
        const SimulationConfig* simConfigObj = NULL;
        if (s_OK == parent->QueryInterface(GET_IID(IGlobalContext), (void**)&pGC))
        {
            simConfigObj = pGC->GetSimulationConfigObj();
        }
        if (!simConfigObj)
        {
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "The pointer obtained to SimulationConfig object is not valid (could be DLL specific)" );
        }

        //durability_time_profile = simConfigObj->PKPD_model;
        //primary_decay_time_constant = hivdtMap[drug_name]->drug_decay_T1;
        //secondary_decay_time_constant = hivdtMap[drug_name]->drug_decay_T2;

        //PkPdParameterValidation();
    }


    void AntiHIVMonotherapyDrug::ApplyEffects()
    {
        assert(ihivda);
        ihivda->ApplyDrugConcentrationAction( drug_name, current_concentration );
    }

}

#endif