/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <map>
#include <vector>

#include "IdmApi.h"
#include "Climate.h"
#include "Common.h"
#include "Environment.h"

#include "NodeDemographics.h"
#include "ITransmissionGroups.h"
#include "suids.hpp"
#include "IInfectable.h"
#include "MathFunctions.h"
#include "SerializationParameters.h"
#include "NodeProperties.h"
#include "INodeContext.h"
#include "StrainIdentity.h"


class Report;
class ReportVector;
class DemographicsReport;
class BaseChannelReport;

namespace Kernel
{
    class RANDOMBASE;
    struct INodeEventContext;
    class  NodeEventContextHost;
    struct ISimulation;
    struct IMigrationInfoFactory;
    struct IDistribution;

    class IDMAPI Node : public INodeContext
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        // This needs to go once we COM-ify and/or make accessors
        // BTW, the scope operators are needed by MSVC, not GCC (or is it the other way around?)
        friend class ::Report;
        friend class ::ReportVector;
        friend class ::DemographicsReport;

    public:
        static Node *CreateNode(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid node_suid);

        Node(ISimulationContext *_parent_sim, ExternalNodeId_t externalNodeId, suids::suid _suid);
        Node(); // constructor for serialization use
        virtual ~Node();

        // INodeContext
        virtual void PreUpdate();
        virtual void Update(float dt) override;
        virtual ISimulationContext* GetParent() override;
        virtual suids::suid   GetSuid() const override;
        virtual suids::suid   GetNextInfectionSuid() override; 
        virtual RANDOMBASE* GetRng() override;
        virtual void SetRng( RANDOMBASE* prng ) override; 
        virtual void AddEventsFromOtherNodes( const std::vector<EventTrigger::Enum>& rTriggerList ) override;

        virtual const NodeParams* GetParams() const;

        virtual IMigrationInfo*   GetMigrationInfo() override;
        virtual NPKeyValueContainer& GetNodeProperties() override;


        // Migration
        virtual void SetupMigration( IMigrationInfoFactory * migration_factory ) override;
        virtual IIndividualHuman* processImmigratingIndividual( IIndividualHuman* ) override;
        virtual void SortHumans() override;
        virtual const std::vector<IIndividualHuman*>& GetHumans() const override;

        // Strain tracking reporter data
        virtual std::map<std::pair<uint32_t,uint64_t>, std::vector<float>>& GetStrainData() override { return strain_map_data; }

        // Network infectivity
        virtual const float                   GetNetInfectFrac()                        const override;
        virtual       void                    SetNetInfectFrac(float)                         override;
        virtual const sparse_contagion_repr&  GetNetInfRep()                            const override;
        virtual       void                    DepositNetInf(sparse_contagion_id,float)        override;

        // Initialization
        virtual void SetContextTo(ISimulationContext* context) override;
        virtual void SetParameters( NodeDemographicsFactory *demographics_factory, ClimateFactory *climate_factory ) override;
        virtual void PopulateFromDemographics() override;
        virtual void InitializeTransmissionGroupPopulations() override;

        // Campaign event-related
        bool IsInPolygon(float* vertex_coords, int numcoords); // might want to create a real polygon object at some point
        bool IsInPolygon( const json::Array &poly );
        bool IsInExternalIdSet( const std::list<ExternalNodeId_t>& nodelist );

        // Reporting to higher levels (intermediate form)
        // Possible TODO: refactor into common interfaces if there is demand
        virtual       INodeEventContext*  GetEventContext()                     override;
        virtual       ExternalNodeId_t    GetExternalID() const override;

        virtual const IdmDateTime&  GetTime()                   const override;
        virtual const Climate*      GetLocalWeather()           const override;
        virtual float               GetInfected()               const override;
        virtual float               GetSymptomatic()            const override;
        virtual float               GetNewlySymptomatic()       const override;
        virtual float               GetStatPop()                const override;
        virtual float               GetBirths()                 const override;
        virtual float               GetCampaignCost()           const override;
        virtual float               GetInfectivity()            const override;
        virtual float               GetInfectionRate()          const override;
        virtual float               GetSusceptDynamicScaling()  const override;
        virtual long int            GetPossibleMothers()        const override;
        virtual uint64_t            GetTotalGenomes()           const override;

        virtual float GetNonDiseaseMortalityRateByAgeAndSex( float age, Gender::Enum sex ) const override;


        // Heterogeneous intra-node transmission
        virtual void ChangePropertyMatrix(const std::string& propertyName, const ScalingMatrix_t& newScalingMatrix) override;
        virtual void ExposeIndividual(IInfectable* candidate, TransmissionGroupMembership_t individual, float dt, TransmissionRoute::Enum route) override;
        virtual void DepositFromIndividual( const IStrainIdentity& strain_IDs, float contagion_quantity, TransmissionGroupMembership_t individual, TransmissionRoute::Enum route) override;
        virtual void GetGroupMembershipForIndividual(TransmissionRoute::Enum route, const tProperties& properties, TransmissionGroupMembership_t& membershipOut) override;
        virtual void UpdateTransmissionGroupPopulation(const tProperties& properties, float size_changes,float mc_weight) override;
        virtual void SetupIntranodeTransmission();
        virtual ITransmissionGroups* CreateTransmissionGroups();
        virtual ITransmissionGroups* GetTransmissionGroups() const override;
        virtual void AddRoute(TransmissionRoute::Enum rRouteName);
        virtual void BuildTransmissionRoutes( float contagionDecayRate );

        virtual act_prob_vec_t DiscreteGetTotalContagion( void ) override;

        virtual float GetTotalContagion( void ) override;
        virtual std::map<TransmissionRoute::Enum, float> GetContagionByRoute() const;
        virtual const RouteList_t& GetTransmissionRoutes() const override;

        virtual float GetContagionByRouteAndProperty( TransmissionRoute::Enum route, const IPKeyValue& property_value ) override;
        virtual float GetLatitudeDegrees()override;
        virtual float GetLongitudeDegrees() override;

        virtual bool IsEveryoneHome() const override;
        virtual void SetWaitingForFamilyTrip( suids::suid migrationDestination,
                                              MigrationType::Enum migrationType,
                                              float timeUntilTrip,
                                              float timeAtDestination,
                                              bool isDestinationNewHome ) override;

        virtual const NodeDemographicsDistribution* GetImmunityDistribution()        const override { return SusceptibilityDistribution; }
        virtual const NodeDemographicsDistribution* GetFertilityDistribution()       const override { return FertilityDistribution; }
        virtual const NodeDemographicsDistribution* GetMortalityDistribution()       const override { return MortalityDistribution; }
        virtual const NodeDemographicsDistribution* GetMortalityDistributionMale()   const override { return MortalityDistributionMale; }
        virtual const NodeDemographicsDistribution* GetMortalityDistributionFemale() const override { return MortalityDistributionFemale; }
        virtual const NodeDemographicsDistribution* GetAgeDistribution()             const override { return AgeDistribution; }

        virtual void ManageFamilyTrip( float currentTime, float dt );

                void  updateVitalDynamics(float dt = 1.0f);             // handles births and non-disease mortality
        virtual void  considerPregnancyForIndividual( bool bPossibleMother, bool bIsPregnant, float age, int individual_id, float dt, IIndividualHuman* pIndividual = nullptr ); 
        virtual float initiatePregnancyForIndividual( int individual_id, float dt ) override;
        virtual bool  updatePregnancyForIndividual( int individual_id, float duration ) override;
        virtual void  accumulateIndividualPopStatsByValue(float mcw, float infectiousness, bool poss_mom, bool is_infected, bool is_symptomatic, bool is_newly_symptomatic);
        virtual void  resetNodeStateCounters(void);

    protected:
#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details

        SerializationBitMask_t serializationFlags;
        static SerializationBitMask_t serializationFlagsDefault;

        NodeDemographicsDistribution* SusceptibilityDistribution;
        NodeDemographicsDistribution* FertilityDistribution;
        NodeDemographicsDistribution* MortalityDistribution;
        NodeDemographicsDistribution* MortalityDistributionMale;
        NodeDemographicsDistribution* MortalityDistributionFemale;
        NodeDemographicsDistribution* AgeDistribution;

        // Do not access these directly but use the access methods above.
        float _latitude;
        float _longitude;

        // Standard distributions for SIMPLE initialization
        IDistribution* distribution_age;
        IDistribution* distribution_migration;
        IDistribution* distribution_demographic_risk;
        IDistribution* distribution_susceptibility;

        float susceptibility_dynamic_scaling;

        // Node properties
        suids::suid suid;
        float base_samp_rate_node;
        float birthrate;

        uint32_t  initial_population;

        // ----------------------------------------------------------------------------------------
        // --- DMB 9-16-2014 Through comparison, it was determined that using a vector (and moving
        // --- the last human in the list to the location being removed) was faster and used less
        // --- memory than using a std::list (even with a variety of allocators).
        // --- Please the IDM Wiki for more details.
        // --- http://ivlabsdvapp50:8090/pages/viewpage.action?pageId=30015603
        // ----------------------------------------------------------------------------------------
        std::vector<IIndividualHuman*> individualHumans;
        std::map<int,suids::suid> home_individual_ids; // people who call this node home

        bool                family_waiting_to_migrate;
        suids::suid         family_migration_destination;
        MigrationType::Enum family_migration_type;
        float               family_time_until_trip;
        float               family_time_at_destination;
        bool                family_is_destination_new_home;

        // Heterogeneous intra-node transmission
        ITransmissionGroups *transmissionGroups;
        
        // Climate and demographics
        Climate *localWeather;
        IMigrationInfo *migration_info;
        NodeDemographics demographics;
        ExternalNodeId_t externalId; // DON'T USE THIS EXCEPT FOR INPUT/OUTPUT PURPOSES!
        NPKeyValueContainer node_properties;

        // Infectivity modifications
        float infectivity_multiplier;
        float infectivity_reservoir_end_time;
        float infectivity_reservoir_size;
        float infectivity_reservoir_start_time;
        float infectivity_overdispersion;

        // Event handling
        friend class NodeEventContextHost;
        friend class Simulation; // so migration can call configureAndAdd?????
        NodeEventContextHost *event_context_host;
        std::vector<EventTrigger::Enum> events_from_other_nodes ;

        //  Counters (some for reporting, others also for internal calculations)
        float statPop;
        float Infected;
        float Births;
        float Disease_Deaths;
        float new_infections;
        float new_reportedinfections;
        float Cumulative_Infections;
        float Cumulative_Reported_Infections;
        float Campaign_Cost;
        long int Possible_Mothers;
        float symptomatic;
        float newly_symptomatic;

        float mean_age_infection;      // (years)
        float newInfectedPeopleAgeProduct;
        std::list<float> infected_people_prior; // [infection_averaging_window];
        std::list<float> infected_age_people_prior; // [infection_averaging_window];

        float infectionrate; // TODO: this looks like its only a reporting counter now and possibly not accurately updated in all cases
        float mInfectivity;

        // Containers for network infectivity
        float                                         net_inf_frac;
        sparse_contagion_repr                         net_inf_rep;
        std::map<sparse_contagion_id, float>          net_inf_dep;

        // Container for strain tracking reporter data
        std::map<std::pair<uint32_t,uint64_t>, std::vector<float>> strain_map_data;

        ISimulationContext *parent;     // Access back to simulation methods
        ISimulation* parent_sim; //reduce access to RNG

        float acquisition_heterogeneity_variance;

        bool bSkipping; // for skip exposure

        int  gap;

        float initial_prevalence;
        float initial_percentage_children;

        std::vector<float> init_prev_fraction;
        std::vector<int>   init_prev_clade;
        std::vector<int>   init_prev_genome;

        RouteList_t routes;

        virtual void Initialize();
        virtual void setupEventContextHost();
        void ExtractDataFromDemographics();
        virtual void LoadImmunityDemographicsDistribution();
        virtual void LoadOtherDiseaseSpecificDistributions() {};

        // Updates
        virtual void updateInfectivity(float dt = 0.0f);
        virtual void updatePopulationStatistics(float=1.0);     // called by updateinfectivity to gather population statistics
        virtual void accumulateIndividualPopulationStatistics(float dt, IIndividualHuman* individual);

        // Population Initialization
        virtual void populateNewIndividualsByBirth(int count_new_individuals = 100) override;
        virtual void populateNewIndividualFromMotherId( unsigned int temp_mother_id );
        virtual void populateNewIndividualFromMotherPointer( IIndividualHuman* mother );
        virtual unsigned int populateNewIndividualFromMotherParams( float mcw, unsigned int child_infections );
        void  conditionallyInitializePregnancy( IIndividualHuman* temp_mother );
        float getPrevalenceInPossibleMothers();
        virtual float drawInitialSusceptibility(float ind_init_age);

        virtual IIndividualHuman *createHuman( suids::suid id, float MCweight, float init_age, int gender);
        IIndividualHuman* configureAndAddNewIndividual(float ind_MCweight=1.0f, float ind_init_age=(20*DAYSPERYEAR), float comm_init_prev=0.0f, float comm_female_ratio=0.5f, float init_mod_acquire=1.0f, float risk_parameter = 1.0);
        virtual IIndividualHuman* addNewIndividual(float monte_carlo_weight = 1.0, float initial_age = 0, int gender = 0, int initial_infections = 0, float susceptibility_parameter = 1.0, float risk_parameter = 1.0);

        virtual void RemoveHuman( int index );

        virtual IIndividualHuman* addNewIndividualFromSerialization();

        double calculateInitialAge( double temp_age );
        Fraction adjustSamplingRateByAge( Fraction sampling_rate, double age ) const;

        // Reporting
        virtual void updateNodeStateCounters(IIndividualHuman *ih);
        virtual void finalizeNodeStateCounters(void);
        virtual void reportNewInfection(IIndividualHuman *ih);
        virtual void reportDetectedInfection(IIndividualHuman *ih);

        // Migration
        virtual void processEmigratingIndividual(IIndividualHuman* i); // not a public method since decision to emigrate is an internal one;
        virtual void postIndividualMigration(IIndividualHuman* ind);
        void resolveEmigration(IIndividualHuman *tempind);

        // Fix up child object pointers after deserializing
        virtual INodeContext *getContextPointer();
        virtual void propagateContextToDependents();

        // Skipping functions & variables
        virtual int calcGap();

        std::map< TransmissionRoute::Enum, float > maxInfectionProb; // set to 1.0 if not defined

        virtual void computeMaxInfectionProb( float dt );

        virtual float GetMaxInfectionProb( TransmissionRoute::Enum route ) const
        {
            // Note that in GENERIC there's on ly one route. Can get tricky b/w CONTACT and ALL.
            return maxInfectionProb.at( route );
        }

        RANDOMBASE* m_pRng;
        suids::distributed_generator m_IndividualHumanSuidGenerator;

        DECLARE_SERIALIZABLE(Node);

#pragma warning( pop )
    };
}
