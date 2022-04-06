/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>

#include "Interventions.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "EventCoordinator.h"
#include "Configure.h"
#include "VectorEnums.h"
#include "VectorDefs.h"
#include "Common.h"

namespace Kernel
{
    class InputEIR : public BaseNodeIntervention
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_CONFIGURED(Outbreak)
        DECLARE_FACTORY_REGISTERED(InterventionFactory, InputEIR, INodeDistributableIntervention)

    public:
        InputEIR();
        InputEIR( const InputEIR& master );
        virtual ~InputEIR() { }

        // INodeDistributableIntervention
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;
        virtual void Update(float dt) override;

        // IBaseIntervention
        virtual float GetCostPerUnit() const override;
    protected:
        AgeDependentBitingRisk::Enum age_dependence;
        std::vector<float> monthly_EIR; // 12 values of EIR by month
        float today;
        float daily_EIR;
        tAgeBitingFunction risk_function;
    };
}
