/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Node.h"
#include "IndividualSTI.h" // for serialization only
#include "INodeSTI.h"

namespace Kernel
{
    class NodeSTI : public Node, public INodeSTI
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE();

    public:
        virtual ~NodeSTI(void);
        static NodeSTI *CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid);

        virtual void Initialize() override;

        virtual void GetGroupMembershipForIndividual_STI( const std::map<std::string, uint32_t>& properties, std::map< int, TransmissionGroupMembership_t>& membershipOut ) override;

    protected:
        NodeSTI();
        NodeSTI(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid);

        IRelationshipManager* relMan;
        ISociety* society;

        virtual void LoadOtherDiseaseSpecificDistributions(const NodeDemographics* demog_ptr) override;

        // Factory methods
        virtual IIndividualHuman* createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender) override;

        // INodeContext
        virtual INodeSTI* GetNodeSTI() override;
        virtual act_prob_vec_t DiscreteGetTotalContagion( void ) override;

        // INodeSTI
        virtual /*const?*/ IRelationshipManager* GetRelationshipManager() /*const?*/ override;
        virtual ISociety* GetSociety() override;
        virtual std::string GetRelationshipName(int rel_enum_val) override;
        virtual std::string GetRelationshipStateName(int rel_state_enum_val) override;

        virtual void SetupIntranodeTransmission() override;
        virtual void Update( float dt ) override;
        virtual void processEmigratingIndividual( IIndividualHuman* individual ) override;
        virtual IIndividualHuman* processImmigratingIndividual( IIndividualHuman* movedind ) override;
        virtual void UpdateTransmissionGroupPopulation( const tProperties& properties, float size_changes, float mc_weight );
        

        DECLARE_SERIALIZABLE(NodeSTI);
    };
}
