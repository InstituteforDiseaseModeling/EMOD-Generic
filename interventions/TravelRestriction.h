/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Interventions.h"          // BaseNodeIntervention
#include "ISupports.h"              // IMPLEMENT_DEFAULT_REFERENCE_COUNTING
#include "InterventionFactory.h"    // InterventionFactory
#include "FactorySupport.h"         // DECLARE_FACTORY_REGISTERED
#include "Configuration.h"          // Configuration

namespace Kernel
{
    class TravelRestriction : public BaseNodeIntervention
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_FACTORY_REGISTERED(InterventionFactory, TravelRestriction, INodeDistributableIntervention)

    public:
        TravelRestriction();
        TravelRestriction(const TravelRestriction&);
        virtual ~TravelRestriction();

        virtual QueryResult QueryInterface(iid_t, void**) override;

        virtual bool Configure(const Configuration*) override;
        virtual bool Distribute(INodeEventContext* context, IEventCoordinator2* pEC = nullptr) override;
        virtual void Update(float) override;

        static void serialize(IArchive&, TravelRestriction*);

    protected:
        float connections_multiplier_inbound;
        float connections_multiplier_outbound;
        float age;
        float duration;
    };
}
