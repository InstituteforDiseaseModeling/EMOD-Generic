/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Interventions.h"
#include "ISupports.h"
#include "InterventionFactory.h"
#include "FactorySupport.h"
#include "Configuration.h"
#include "InterpolatedValueMap.h"

namespace Kernel
{
    class NodeInfectivityMult : public BaseNodeIntervention
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_FACTORY_REGISTERED(InterventionFactory, NodeInfectivityMult, INodeDistributableIntervention)

    public:
        NodeInfectivityMult();
        NodeInfectivityMult(const NodeInfectivityMult&);
        virtual ~NodeInfectivityMult();

        virtual QueryResult QueryInterface(iid_t, void**) override;

        virtual bool Configure(const Configuration*) override;
        virtual bool Distribute(INodeEventContext* context, IEventCoordinator2* pEC = nullptr) override;
        virtual void Update(float) override;

        static void serialize(IArchive&, NodeInfectivityMult*);

    protected:
        float                 duration;

        InterpolatedValueMap  mult_by_duration;
    };
}
