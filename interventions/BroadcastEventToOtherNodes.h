/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Configuration.h"
#include "InterventionFactory.h"
#include "Interventions.h"
#include "EventTrigger.h"
#include "INodeInfo.h"
#include "InterventionEnums.h"

namespace Kernel
{
    // This intervention is used to broadcast an event to all of the individuals in other nodes.
    // For example, if an individual in the current node is detected to have malaria, one could use
    // this intervention to send the event to the surrounding nodes so that the people in those
    // nodes could be given anti-malaria drugs.
    class IDMAPI BroadcastEventToOtherNodes :  public BaseIntervention, public INodeQualifier
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_FACTORY_REGISTERED(InterventionFactory, BroadcastEventToOtherNodes, IDistributableIntervention)

    public: 
        BroadcastEventToOtherNodes();
        BroadcastEventToOtherNodes( const BroadcastEventToOtherNodes& master );
        virtual ~BroadcastEventToOtherNodes() {  }

        virtual bool Configure( const Configuration* pConfig ) override;

        // IDistributingDistributableIntervention
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;
        virtual void SetContextTo(IIndividualHumanContext *context) override;
        virtual void Update(float dt) override;

        // INodeQualifier
        virtual bool Qualifies( const INodeInfo& rni ) const  override;

    protected:

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        INodeContext* p_node_context ;
        EventTrigger::Enum event_trigger;
        bool include_my_node ;
        NodeSelectionType::Enum node_selection_type ;
        float max_distance_km ;

        DECLARE_SERIALIZABLE(BroadcastEventToOtherNodes);
#pragma warning( pop )
    };
}
