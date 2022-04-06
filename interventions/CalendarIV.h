/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Interventions.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "Configure.h"
#include "HealthSeekingBehavior.h"

namespace Kernel
{
    class IVCalendar : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, IVCalendar, IDistributableIntervention)

    public:
        // We inherit AddRef/Release abstractly through IHealthSeekBehavior,
        // even though BaseIntervention has a non-abstract version.
        virtual int32_t AddRef() override { return BaseIntervention::AddRef(); }
        virtual int32_t Release() override { return BaseIntervention::Release(); }

        IVCalendar();
        virtual ~IVCalendar();
        bool Configure( const Configuration* config ) override;

        // IDistributingDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pICCO ) override;
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;
        virtual void Update(float dt) override;

    protected:
        std::map< float, float > age2ProbabilityMap;
        IndividualInterventionConfig actual_intervention_config;
        bool dropout;

        DECLARE_SERIALIZABLE(IVCalendar);

    private:
        std::list<float> scheduleAges;
        std::string dumpCalendar();
    };
}
