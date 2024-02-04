/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "NodeSTI.h"
#include "Debug.h"

#include "IndividualSTI.h"
#include "RelationshipManagerFactory.h"
#include "RelationshipGroups.h"

#include "SocietyFactory.h"
#include "IIdGeneratorSTI.h"
#include "NodeEventContextHost.h"
#include "ISTISimulationContext.h"
#include "ISimulationContext.h"
#include "EventTrigger.h"

SETUP_LOGGING( "NodeSTI" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(NodeSTI, Node)
        HANDLE_INTERFACE(INodeSTI)
    END_QUERY_INTERFACE_DERIVED(NodeSTI, Node)

    NodeSTI::NodeSTI(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
        : Node(_parent_sim, externalNodeId, node_suid)
        , relMan(nullptr)
        , society(nullptr)
    {
        relMan = RelationshipManagerFactory::CreateManager( this );
        society = SocietyFactory::CreateSociety( relMan );
    }

    NodeSTI::NodeSTI()
        : Node()
        , relMan(nullptr)
        , society(nullptr)
    {
        relMan = RelationshipManagerFactory::CreateManager( this );
        society = SocietyFactory::CreateSociety( relMan );
    }

    NodeSTI::~NodeSTI(void)
    {
        delete society ;
        delete relMan ;
    }

    NodeSTI *NodeSTI::CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid)
    {
        NodeSTI *newnode = _new_ NodeSTI(_parent_sim, externalNodeId, node_suid);
        newnode->Initialize();

        return newnode;
    }

    void NodeSTI::Initialize()
    {
        society->Configure( EnvPtr->Config );

        Node::Initialize();
    }

    void NodeSTI::LoadOtherDiseaseSpecificDistributions(const NodeDemographics* demog_ptr)
    {
        const std::string SOCIETY_KEY( "Society" );
        if( !(*demog_ptr).Contains( SOCIETY_KEY ) )
        {
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "Could not find the 'Society' element in the demographics data." );
        }
        std::istringstream iss( (*demog_ptr)[SOCIETY_KEY].ToString() );
        Configuration* p_config = Configuration::Load( iss, "demographics" );
        society->SetParameters( GetRng(), dynamic_cast<IIdGeneratorSTI*>(parent), p_config );
        delete p_config ;
    }

    IIndividualHuman* NodeSTI::createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender)
    {
        return IndividualHumanSTI::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender);
    }

    void NodeSTI::SetupIntranodeTransmission()
    {
        RelationshipGroups * relNodePools = _new_ RelationshipGroups( GetRng() );
        relNodePools->SetParent( this );
        transmissionGroups = relNodePools;
        AddRoute(TransmissionRoute::CONTACT);
        event_context_host->SetupTxRoutes();
        transmissionGroups->Build(1.0f, 1, 1);
    }

    void NodeSTI::Update( float dt )
    {
        // Update relationships (dissolution only, at this point)
        list<IIndividualHuman*> population;  //not used by RelationshipManager
        relMan->Update( population, transmissionGroups, dt );

        society->BeginUpdate();

        for (auto& person : individualHumans)
        {
            IIndividualHumanSTI* sti_person = nullptr;
            if (person->QueryInterface(GET_IID(IIndividualHumanSTI), (void**)&sti_person) != s_OK)
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "person", "IIndividualHumanSTI", "IIndividualHuman" );
            }
            sti_person->UpdateEligibility();        // DJK: Could be slow to do this on every update.  Could check for relationship status changes. <ERAD-1869>
            sti_person->UpdateHistory( GetTime(), dt );
        }

        society->UpdatePairFormationRates( GetTime(), dt );

        for (auto& person : individualHumans)
        {
            IIndividualHumanSTI* sti_person = nullptr;
            if (person->QueryInterface(GET_IID(IIndividualHumanSTI), (void**)&sti_person) != s_OK)
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "person", "IIndividualHumanSTI", "IIndividualHuman" );
            }
            sti_person->ConsiderRelationships(dt);
        }

        society->UpdatePairFormationAgents( GetTime(), dt );

        transmissionGroups->Build( 1.0f, 1, 1 );
        
        Node::Update( dt );

        for( auto& person : individualHumans )
        {
            IIndividualHumanSTI* sti_person = nullptr;
            if( person->QueryInterface( GET_IID( IIndividualHumanSTI ), (void**)&sti_person ) != s_OK )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "person", "IIndividualHumanSTI", "IIndividualHuman" );
            }
            sti_person->UpdatePausedRelationships( GetTime(), dt );
        }
    }

    act_prob_vec_t NodeSTI::DiscreteGetTotalContagion( void )
    {
        return transmissionGroups->DiscreteGetTotalContagion();
    }

    /*const?*/ IRelationshipManager*
    NodeSTI::GetRelationshipManager() /*const?*/
    {
        return relMan;
    }

    ISociety* NodeSTI::GetSociety()
    {
        return society;
    }

    std::string NodeSTI::GetRelationshipName(int rel_enum_val)
    {
        return RelationshipType::pairs::lookup_key(rel_enum_val);
    }

    std::string NodeSTI::GetRelationshipStateName(int rel_state_enum_val)
    {
        return RelationshipState::pairs::lookup_key(rel_state_enum_val);
    }

    INodeSTI* NodeSTI::GetNodeSTI()
    {
        return static_cast<INodeSTI*>(this);
    }

    void
    NodeSTI::processEmigratingIndividual(
        IIndividualHuman* individual
    )
    {
        event_context_host->TriggerObservers( individual->GetEventContext(), EventTrigger::STIPreEmigrating );

        IIndividualHumanSTI* sti_individual=nullptr;
        if (individual->QueryInterface(GET_IID(IIndividualHumanSTI), (void**)&sti_individual) != s_OK)
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualSTI", "IndividualHuman" );
        }

        sti_individual->onEmigrating();

        Node::processEmigratingIndividual( individual );
    }

    IIndividualHuman*
    NodeSTI::processImmigratingIndividual(
        IIndividualHuman* movedind
    )
    {
        // -------------------------------------------------------------------------------
        // --- SetContextTo() is called in Node::processImmigratingIndividual() but
        // --- we need need to set context before onImmigrating().  onImmigrating() needs
        // --- the RelationshipManager which is part of the node.
        // -------------------------------------------------------------------------------
        movedind->SetContextTo(getContextPointer());

        IIndividualHumanSTI* sti_individual = nullptr;
        if (movedind->QueryInterface(GET_IID(IIndividualHumanSTI), (void**)&sti_individual) != s_OK)
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "retVal", "IIndividualSTI", "IndividualHuman" );
        }
        sti_individual->onImmigrating();

        auto retVal = Node::processImmigratingIndividual( movedind );

        event_context_host->TriggerObservers( retVal->GetEventContext(), EventTrigger::STIPostImmigrating );

        return retVal;
    }

    void NodeSTI::GetGroupMembershipForIndividual_STI(
        const std::map<std::string, uint32_t>& properties,
        std::map< int, TransmissionGroupMembership_t> &membershipOut
    )
    {
        RelationshipGroups* p_rg = static_cast<RelationshipGroups*>(transmissionGroups);
        p_rg->GetGroupMembershipForProperties( properties, membershipOut );
    }

    void NodeSTI::UpdateTransmissionGroupPopulation(
        const tProperties& properties,
        float size_changes,
        float mc_weight
    )
    {
    }

    REGISTER_SERIALIZABLE(NodeSTI);

    void NodeSTI::serialize(IArchive& ar, NodeSTI* obj)
    {
        Node::serialize(ar, obj);
    }
}
