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
#include "EventCoordinator.h"
#include "Configure.h"
#include "StrainIdentity.h"
#include "Common.h"
#include "Outbreak.h"
#include "Types.h"
#include <vector>

namespace Kernel
{
    class ImportPressure : public Outbreak
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_CONFIGURED(Outbreak)
        DECLARE_FACTORY_REGISTERED(InterventionFactory, ImportPressure, INodeDistributableIntervention)

    public:
        ImportPressure();
        virtual ~ImportPressure();

        // INodeDistributableIntervention
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void Update(float dt);
        virtual void SetContextTo(INodeEventContext *context);
        virtual bool Distribute(INodeEventContext *context, IEventCoordinator2* pEC);

    protected:
        INodeEventContext* parent;

        float duration_counter;

        std::vector<float> durations; 
        std::vector<float> daily_import_pressures;

        std::vector<std::pair<float, float>> durations_and_pressures;
    };
}
