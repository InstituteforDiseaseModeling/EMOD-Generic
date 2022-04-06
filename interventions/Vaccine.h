/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Interventions.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "Configure.h"
#include "IWaningEffect.h"
#include "InterpolatedValueMap.h"

namespace Kernel
{
    struct IVaccineConsumer;
    struct ICampaignCostObserver;

    class Vaccine : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, Vaccine, IDistributableIntervention)

        DECLARE_SERIALIZABLE(Vaccine);

    public:
        Vaccine();
        Vaccine(const Vaccine&);
        virtual ~Vaccine();

        virtual bool Configure(const Configuration*) override;

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext*, ICampaignCostObserver* const) override;
        virtual void SetContextTo(IIndividualHumanContext*) override;
        virtual void Update(float dt) override;
        virtual bool NeedsInfectiousLoopUpdate() const;

        // ISupports
        virtual QueryResult QueryInterface(iid_t, void**) override;

    protected:

        bool  vaccine_took;
        bool  efficacy_is_multiplicative;

        float vaccine_take;
        float frac_acq_blocking_take;

        InterpolatedValueMap  take_by_age_map;
        InterpolatedValueMap  init_acq_by_effect_map;
        InterpolatedValueMap  init_trn_by_effect_map;
        InterpolatedValueMap  init_mor_by_effect_map;

        IWaningEffect* effect_acquire;
        IWaningEffect* effect_transmit;
        IWaningEffect* effect_mortality;

        IVaccineConsumer* ivc;
    };
}
