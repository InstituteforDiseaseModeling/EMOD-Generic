/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <list>

#include "NodeEventContext.h"
#include "BroadcasterObserver.h"
#include "BroadcasterImpl.h"

namespace Kernel
{
    struct NodeDemographics;
    class Node;
    
    struct INodeContext;
    struct IIndividualHuman;
    class IndividualHuman;

    // The NodeEventContextHost implements functionality properly belonging to the Node class but it split out manually to make development easier.
    // Like, you know, what partial class declarations are for.
    class NodeEventContextHost : public INodeEventContext,
                                 public INodeInterventionConsumer,
                                 public ICampaignCostObserver,
                                 public IIndividualEventBroadcaster
    {
        IMPLEMENT_NO_REFERENCE_COUNTING()

    public:
        NodeEventContextHost();
        NodeEventContextHost(Node* _node);
        virtual ~NodeEventContextHost();

        // INodeEventContext
        virtual QueryResult QueryInterface(iid_t iid, void** pinstance) override;
        virtual void VisitIndividuals(individual_visit_function_t func) override;
        virtual int VisitIndividuals(IVisitIndividual* ) override;
        virtual const IdmDateTime& GetTime() const override;
        virtual bool IsInPolygon(float* vertex_coords, int numcoords) override; // might want to create a real polygon object at some point
        virtual bool IsInPolygon( const json::Array &poly ) override;
        virtual bool IsInExternalIdSet( const std::list<ExternalNodeId_t>& nodelist ) override;
        virtual RANDOMBASE* GetRng() override;
        virtual INodeContext* GetNodeContext() override;
        virtual int GetIndividualHumanCount() const override;
        virtual ExternalNodeId_t GetExternalId() const override;

        virtual INodeInterventionConsumer*   GetNodeInterventionConsumer()   override;
        virtual ICampaignCostObserver*       GetCampaignCostObserver()       override;
        virtual IIndividualEventBroadcaster* GetIndividualEventBroadcaster() override;

        virtual void IncrementCampaignCost(float cost) override;

        virtual void  UpdateInterventions(float = 0.0f) override;

        virtual void  UpdateBirthRateMultiplier(float mult_val)                                      override;
        virtual void  UpdateConnectionModifiers(float inbound, float outbound)                       override;
        virtual void  UpdateInfectivityMultiplier(float mult_val, TransmissionRoute::Enum tx_route)  override;

        virtual float GetBirthRateMultiplier()                                    const override;
        virtual float GetInboundConnectionModifier()                              const override;
        virtual float GetOutboundConnectionModifier()                             const override;
        virtual float GetInfectivityMultiplier(TransmissionRoute::Enum tx_route)   const override;

        // TODO: methods to install hooks for birth and other things...can follow similar pattern presumably

        virtual void RegisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, TravelEventType type) override;
        virtual void UnregisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, TravelEventType type) override;

        virtual const suids::suid & GetId() const override;

        virtual void SetContextTo(INodeContext* context) override;
        virtual void SetupTxRoutes() override;

        virtual std::list<INodeDistributableIntervention*> GetInterventionsByType(const std::string& type_name) override;
        virtual std::list<INodeDistributableIntervention*> GetInterventionsByName(const std::string& intervention_name) override;
        virtual void PurgeExisting( const std::string& iv_name ) override;
        virtual void PurgeExistingByName( const std::string& iv_name ) override;
        virtual bool ContainsExisting( const std::string& iv_name ) override;
        virtual bool ContainsExistingByName( const std::string& iv_name ) override;

        virtual void AddImportCases( const StrainIdentity* outbreak_strainID, float import_age, int num_cases_per_node, float inf_prob, float female_prob, float mc_weight) override;

        // INodeInterventionConsumer
        virtual bool GiveIntervention( INodeDistributableIntervention * pIV ) override;

        // IIndividualTriggeredInterventionConsumer
        virtual void RegisterObserver( IIndividualEventObserver *pIEO, const EventTrigger::Enum& trigger ) override;
        virtual void UnregisterObserver( IIndividualEventObserver *pIEO, const EventTrigger::Enum& trigger ) override;
        virtual void TriggerObservers( IIndividualHumanEventContext *ihec, const EventTrigger::Enum& trigger ) override;

        //////////////////////////////////////////////////////////////////////////
         
        void ProcessArrivingIndividual( IIndividualHuman* );
        void ProcessDepartingIndividual( IIndividualHuman * );

        // ICampaignCostObserver
        virtual void notifyCampaignExpenseIncurred( float expenseIncurred, const IIndividualHumanEventContext * pIndiv ) override;
        virtual void notifyCampaignEventOccurred( /*const*/ ISupports * pDistributedIntervention, /*const*/ ISupports * pDistributor, /*const*/ IIndividualHumanContext * pDistributeeIndividual ) override;

    protected:
        Node* node;

        float birthrate_multiplier;
        float connection_multiplier_inbound;
        float connection_multiplier_outbound;
        std::map<TransmissionRoute::Enum, float> inf_mult_by_route;

        typedef std::map<ITravelLinkedDistributionSource*,int> travel_distribution_source_map_t;

        travel_distribution_source_map_t arrival_distribution_sources;
        travel_distribution_source_map_t departure_distribution_sources;

        void cleanupDistributionSourceMap( travel_distribution_source_map_t &map );
        travel_distribution_source_map_t* sourcesMapForType( TravelEventType type );

        typedef std::list<INodeDistributableIntervention*> ndi_list_t;
        ndi_list_t node_interventions;

        IndividualEventBroadcaster broadcaster_impl;

        virtual void PropagateContextToDependents(); // pass context to interventions if they need it
    };
}
