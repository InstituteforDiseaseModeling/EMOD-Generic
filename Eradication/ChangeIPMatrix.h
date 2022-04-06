/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "Configuration.h"
#include "InterventionFactory.h"
#include "Interventions.h"

namespace Kernel
{
    class ChangeIPMatrix : public BaseNodeIntervention
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_FACTORY_REGISTERED( InterventionFactory, ChangeIPMatrix, INodeDistributableIntervention )

    public:
        ChangeIPMatrix();
        virtual ~ChangeIPMatrix();

        virtual bool Configure( const Configuration* pConfig ) override;

        // IDistributingDistributableIntervention
        virtual QueryResult QueryInterface( iid_t iid, void **ppvObject ) override;
        virtual bool Distribute(INodeEventContext *context, IEventCoordinator2* pEC);
        virtual void Update(float dt);

    protected:
        std::string                        target_property_name;
        std::vector<std::vector<float>>    revised_matrix;
    };
}
