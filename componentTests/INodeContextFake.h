/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "ISimulationContext.h"
#include "NodeEventContext.h"
#include "IIndividualHumanContext.h"
#include "IdmDateTime.h"
#include "INodeContext.h"
#include "TransmissionGroupMembership.h"

using namespace Kernel;

struct ISimulationContextFake : ISimulationContext
{
public:
    ISimulationContextFake()
    { }

    virtual suids::suid GetNodeSuid( ExternalNodeId_t external_node_id ) override
    {
        suids::suid fake_suid;
        fake_suid.data = external_node_id;
        return fake_suid;
    }

    virtual const                        DemographicsContext* GetDemographicsContext() const override                                                   { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const                        SimParams* GetParams() const override                                                                          { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const IdmDateTime&           GetSimulationTime() const override                                                                             { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual suids::suid                  GetNextInfectionSuid() override                                                                                { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual ExternalNodeId_t             GetNodeExternalID( const suids::suid& rNodeSuid ) override                                                     { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual float                        GetNodeInboundMultiplier( const suids::suid& rNodeSuid ) override                                              { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual uint32_t                     GetNodeRank( const suids::suid& rNodeSuid ) override                                                           { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual void                         PostMigratingIndividualHuman( IIndividualHuman *i )override                                                    { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual bool                         CanSupportFamilyTrips() const override                                                                         { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual void                         DistributeEventToOtherNodes( const EventTrigger::Enum& rEventTrigger, INodeQualifier* pQualifier ) override    { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual void                         UpdateNodeEvents() override                                                                                    { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual ISimulationEventContext*     GetSimulationEventContext() override                                                                           { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual std::vector<IReport*>&       GetReports() override                                                                                          { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual std::vector<IReport*>&       GetReportsNeedingIndividualData() override                                                                     { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const IInterventionFactory*  GetInterventionFactory() const override                                                                        { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual QueryResult                  QueryInterface( iid_t iid, void** pinstance ) override                                                         { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual int32_t                      AddRef() override                                                                                              { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual int32_t                      Release() override                                                                                             { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
};

struct INodeContextFake : INodeContext
{
private:
    suids::suid m_suid ;
    INodeEventContext* m_pNEC;
    NPKeyValueContainer m_NodeProperties;
    IdmDateTime m_Time;
    ISimulationContextFake m_FakeSim;

public:
    INodeContextFake( int id = 1 )
    : m_suid()
    , m_pNEC(nullptr)
    , m_NodeProperties()
    , m_Time()
    , m_FakeSim()
    {
        m_suid.data = id ;
    }

    INodeContextFake( const suids::suid& rSuid, INodeEventContext* pNEC = nullptr )
    : m_suid(rSuid)
    , m_pNEC(pNEC)
    , m_NodeProperties()
    , m_Time()
    , m_FakeSim()
    {
        if( m_pNEC != nullptr )
        {
            m_pNEC->SetContextTo( this );
        }
    }

    virtual bool operator==( const INodeContext& rThat ) const override
    {
        const INodeContextFake* pThat = dynamic_cast<const INodeContextFake*>(&rThat) ;
        if( pThat == nullptr ) return false ;

        if( this->m_suid.data != pThat->m_suid.data ) return false ;

        return true ;
    }

    virtual ISimulationContext* GetParent() override
    {
        return &m_FakeSim;
    }

    virtual suids::suid GetSuid() const override
    {
        return m_suid ;
    }

    virtual const NodeParams* GetParams() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual suids::suid GetNextInfectionSuid() override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual RANDOMBASE* GetRng() override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void SetRng( RANDOMBASE* prng ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented." );
    }

    virtual void ChangePropertyMatrix( const std::string& propertyName, const ScalingMatrix_t& newScalingMatrix ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void ExposeIndividual( IInfectable* candidate, TransmissionGroupMembership_t individual, float dt ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void DepositFromIndividual( const IStrainIdentity& strain_IDs, float contagion_quantity, TransmissionGroupMembership_t individual, TransmissionRoute::Enum route ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void GetGroupMembershipForIndividual( const RouteList_t& route, const tProperties& properties, TransmissionGroupMembership_t& membershipOut ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void UpdateTransmissionGroupPopulation( const tProperties& properties, float size_changes,float mc_weight ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual std::map< std::string, float > GetContagionByRoute() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetTotalContagion( void ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual RouteList_t& GetTransmissionRoutes() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetContagionByRouteAndProperty( const std::string& route, const IPKeyValue& property_value )
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented." );
    }

    virtual IMigrationInfo* GetMigrationInfo() override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual NPKeyValueContainer& GetNodeProperties() override
    {
        return m_NodeProperties;
    }

    virtual const IdmDateTime& GetTime() const override
    {
        if( m_pNEC != nullptr )
        {
            return m_pNEC->GetTime();
        }
        else
        {
            static IdmDateTime time(0);
            return time;
        }
    }

    virtual float GetInfected() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetSymptomatic() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented." );
    }

    virtual float GetNewlySymptomatic() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented." );
    }

    virtual float GetStatPop() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetBirths() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetCampaignCost() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetInfectivity() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetInfectionRate() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetSusceptDynamicScaling() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual const Climate* GetLocalWeather() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual INodeEventContext* GetEventContext() override
    {
        release_assert( m_pNEC );
        return m_pNEC;
    }

    virtual void AddEventsFromOtherNodes( const std::vector<EventTrigger::Enum>& rTriggerList ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual QueryResult QueryInterface( iid_t iid, void** pinstance ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual int32_t AddRef() override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual int32_t Release() override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    act_prob_vec_t DiscreteGetTotalContagion( void ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual long int GetPossibleMothers() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual bool IsEveryoneHome() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void SetWaitingForFamilyTrip( suids::suid migrationDestination, MigrationType::Enum migrationType, float timeUntilTrip, float timeAtDestination, bool isDestinationNewHome ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetMeanAgeInfection() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual uint64_t GetTotalGenomes() const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual const NodeDemographicsDistribution* GetImmunityDistribution()        const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const NodeDemographicsDistribution* GetFertilityDistribution()       const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const NodeDemographicsDistribution* GetMortalityDistribution()       const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const NodeDemographicsDistribution* GetMortalityDistributionMale()   const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const NodeDemographicsDistribution* GetMortalityDistributionFemale() const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }
    virtual const NodeDemographicsDistribution* GetAgeDistribution()             const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented."); }

    virtual ExternalNodeId_t GetExternalID() const override
    {
        return m_suid.data ;
    }

    virtual float GetLatitudeDegrees() override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float GetLongitudeDegrees() override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void SetupMigration( IMigrationInfoFactory * migration_factory ) override
    { 
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented.");
    }

    virtual void SetParameters( NodeDemographicsFactory *demographics_factory, ClimateFactory *climate_factory, bool white_list_enabled ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented.");
    }

    virtual void SetContextTo(ISimulationContext*)                                                             override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual void PopulateFromDemographics()                                                                    override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual void InitializeTransmissionGroupPopulations()                                                      override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual ITransmissionGroups* GetTransmissionGroups() const                                                 override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual void Update(float)                                                                                 override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual IIndividualHuman* processImmigratingIndividual(IIndividualHuman*)                                  override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual void SortHumans()                                                                                  override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }

    virtual const std::vector<IIndividualHuman*>&             GetHumans()                                const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual       std::map<std::pair<uint32_t,uint64_t>, std::vector<float>>&  GetStrainData()                 override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual const float                                       GetNetInfectFrac()                         const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual       void                                        SetNetInfectFrac(float)                          override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual const sparse_contagion_repr&                      GetNetInfRep()                             const override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }
    virtual       void                                        DepositNetInf(sparse_contagion_id, float)        override { throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method is not implemented."); }

    virtual ProbabilityNumber GetProbMaternalTransmission() const
    {
        return 1.0;
    }
    virtual float GetMaxInfectionProb( TransmissionRoute::Enum route ) const
    {
        return 0.0f;
    }


    virtual float GetNonDiseaseMortalityRateByAgeAndSex( float age, Gender::Enum sex ) const override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual float initiatePregnancyForIndividual( int individual_id, float dt ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual bool updatePregnancyForIndividual( int individual_id, float duration ) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

    virtual void populateNewIndividualsByBirth(int count_new_individuals = 100) override
    {
        throw Kernel::NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "The method or operation is not implemented.");
    }

};
