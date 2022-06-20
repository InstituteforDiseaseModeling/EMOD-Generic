/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Node.h"
#include "IndividualEnvironmental.h"
#include <math.h>

namespace Kernel
{
    class INodeEnvironmental // : public ISupports
    {
    public:
        // Nothing special here, yet.
    };

    class NodeEnvironmental : public Node, public INodeEnvironmental
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        static NodeEnvironmental *CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid);
        virtual ~NodeEnvironmental(void);

    protected:
        NodeEnvironmental( ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid );
        NodeEnvironmental();

        // Factory methods
        virtual IIndividualHuman* createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender) override;

        virtual void updateInfectivity( float dt ) override;

        virtual void SetupIntranodeTransmission() override;
        virtual ITransmissionGroups* CreateTransmissionGroups() override;
        virtual void BuildTransmissionRoutes( float contagionDecayRate ) override;

        virtual void DepositFromIndividual( const IStrainIdentity& strain_IDs, float contagion_quantity, TransmissionGroupMembership_t individual, TransmissionRoute::Enum route ) override;

        virtual void UpdateTransmissionGroupPopulation(const tProperties& properties, float size_changes,float mc_weight) override;
        virtual void ExposeIndividual(IInfectable* candidate, TransmissionGroupMembership_t individual, float dt, TransmissionRoute::Enum route) override;

        virtual float GetContagionByRouteAndProperty( TransmissionRoute::Enum route, const IPKeyValue& property_value ) override;
        virtual void GetGroupMembershipForIndividual(TransmissionRoute::Enum route, const tProperties& properties, TransmissionGroupMembership_t& membershipOut) override;
        virtual std::map<TransmissionRoute::Enum, float> GetContagionByRoute() const override;

        DECLARE_SERIALIZABLE(NodeEnvironmental);

    private:
        float getSeasonalAmplitude() const;
    };
}
