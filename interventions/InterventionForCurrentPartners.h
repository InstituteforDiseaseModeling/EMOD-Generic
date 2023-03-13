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
#include "FactorySupport.h"
#include "Configure.h"
#include "IRelationship.h"
#include "EventTrigger.h"

namespace Kernel
{
    class InterventionForCurrentPartners : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED( InterventionFactory, InterventionForCurrentPartners, IDistributableIntervention )

    public:
        InterventionForCurrentPartners();
        InterventionForCurrentPartners( const InterventionForCurrentPartners& rMaster );
        virtual ~InterventionForCurrentPartners();

        virtual bool Configure( const Configuration * config ) override;

        // IDistributableIntervention
        virtual QueryResult QueryInterface( iid_t iid, void **ppvObject ) override;
        virtual void Update( float dt ) override;

    protected:
        std::vector<IRelationship*> SelectRelationships( const RelationshipSet_t& rRelationships );
        std::vector<IIndividualHumanEventContext*> SelectPartners( IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );
        void ReducePartners( std::vector<IIndividualHumanEventContext*>& partners );
        void DistributeToPartnersEvent( const std::vector<IIndividualHumanEventContext*>& partners );
        void DistributeToPartnersIntervention( const std::vector<IIndividualHumanEventContext*>& partners );

        std::vector<IIndividualHumanEventContext*> SelectPartnersNoPrioritization( IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );
        std::vector<IIndividualHumanEventContext*> SelectPartnersChosenAtRandom(   IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );
        std::vector<IIndividualHumanEventContext*> SelectPartnersLongerTime(       IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );
        std::vector<IIndividualHumanEventContext*> SelectPartnersShorterTime(      IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );
        std::vector<IIndividualHumanEventContext*> SelectPartnersOlderAge(         IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );
        std::vector<IIndividualHumanEventContext*> SelectPartnersYoungerAge(       IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );
        std::vector<IIndividualHumanEventContext*> SelectPartnersRelationshipType( IIndividualHumanSTI* pHumanStiSelf, std::vector<IRelationship*>& reducedRelationships );

        std::vector<RelationshipType::Enum> m_RelationshipTypes;
        PartnerPrioritizationType::Enum m_PrioritizePartnersBy;
        float m_MinimumDurationYears;
        float m_MinimumDurationDays;
        float m_MaximumPartners;
        EventOrConfig::Enum m_UseEventOrConfig;
        EventTrigger::Enum m_EventToBroadcast;
        IDistributableIntervention* m_di;

        DECLARE_SERIALIZABLE( InterventionForCurrentPartners );
    };
}
