/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Drugs.h"
#include "HIVInterventionsContainer.h"  // For IHIVIntervention 

namespace Kernel
{
    //------------------------------ Anti-HIV Monotherapy Drug -------------------------------------------
    struct IHIVDrugEffectsApply;

    class AntiHIVMonotherapyDrug : public GenericDrug, public IHIVIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, AntiHIVMonotherapyDrug, IDistributableIntervention);

    public:
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        bool Configure( const Configuration * );
        AntiHIVMonotherapyDrug();
        virtual ~AntiHIVMonotherapyDrug();

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void SetContextTo(IIndividualHumanContext *context);

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO );

    protected:
        virtual void  ConfigureDrugTreatment( IIndividualHumanInterventionsContext * ivc ) override;

        // These have same names as analogous methods on container but are internal for the drug itself.
        virtual float GetDrugInactivationRate();
        virtual float GetDrugClearanceRate();

        float get_drug_viral_suppression_efficacy();
        float get_drug_concentration();

        virtual void ApplyEffects();            // DJK: ApplyEffects() --> ApplyAction() <ERAD-1853>

        IHIVDrugEffectsApply* ihivda;

        HIVDrugClass::Enum hiv_drug_class;        // Class of HIV drug
        ReverseTranscriptaseNucleosideAnalog::Enum nucleoside_analog;   // Used only for NRTI drugs to identify target
    };
}
