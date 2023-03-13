/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "NodeEnvironmental.h"
#include "IndividualTyphoid.h"
#include "TyphoidInterventionsContainer.h"

class ReportTyphoid;

namespace Kernel
{
    class SpatialReportTyphoid;

    class NodeTyphoid : public NodeEnvironmental
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        // TODO Get rid of friending and provide accessors for all these floats
        friend class ::ReportTyphoid;
        friend class Kernel::SpatialReportTyphoid;

    public:
        static NodeTyphoid *CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid);
        virtual ~NodeTyphoid(void);

        virtual void resetNodeStateCounters(void);

        virtual int calcGap() override;

    protected:
        NodeTyphoid();
        NodeTyphoid(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid);
        virtual void Initialize() override;
        virtual void setupEventContextHost() override;

        // Factory methods
        virtual Kernel::IndividualHuman *createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender);

        virtual void computeMaxInfectionProb( float dt ) override;

    private:
    };
}
