/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <list>
#include <map>
#include <string>

#include "Common.h"
#include "Configure.h"
#include "IIndividualHumanContext.h"
#include "IInfectable.h"
#include "IMigrate.h"
#include "IndividualEventContext.h"
#include "InterventionsContainer.h" // for serialization code to compile
#include "TransmissionGroupMembership.h"
#include "SimulationEnums.h"
#include "suids.hpp"
#include "IContagionPopulation.h"
#include "IIndividualHuman.h"
#include "Types.h" // for ProbabilityNumber

class Configuration;

namespace Kernel
{
    class RANDOMBASE;
    struct INodeContext;
    struct IMigrationInfoFactory;
    struct IIndividualHumanInterventionsContext;
    class  Infection;
    class  InterventionsContainer;
    struct ISusceptibilityContext;
    class  NodeNetwork;
    class  StrainIdentity;
    class  Susceptibility;

    class IndividualHumanConfig : public JsonConfigurable
    {
        friend class Simulation;
        friend class IndividualHuman;
        friend class IndividualHumanTyphoid;
        friend class Node;
        friend class IndividualHumanMalariaConfig;
        friend class IndividualHumanPolioConfig;

        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER( IndividualHumanConfig )

    public:
        virtual bool Configure( const Configuration* config ) override;

    protected:
        static bool aging;
        static bool enable_immunity;
        static bool enable_skipping;
        static bool superinfection;

        static int infection_updates_per_tstep;
        static int max_ind_inf;
    };

    class IndividualHuman : public IIndividualHuman,
                            public IIndividualHumanContext,
                            public IIndividualHumanEventContext,
                            public IInfectable,
                            public IInfectionAcquirable,
                            public IMigrate
    {
        friend class Simulation;

        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        DECLARE_SERIALIZABLE( IndividualHuman )

    public:
        static IndividualHuman *CreateHuman();
        static IndividualHuman *CreateHuman(INodeContext *context, suids::suid id, float MCweight = 1.0f, float init_age = 0.0f, int gender = 0);
        virtual ~IndividualHuman();

        virtual const AgentParams* GetParams() const;

        virtual void InitializeHuman() override;

        virtual void Update(float currenttime, float dt) override;

        virtual IMigrate* GetIMigrate() override;

        // IIndividualHumanContext
        virtual suids::suid GetSuid() const override;
        virtual suids::suid GetNextInfectionSuid() override;
        virtual RANDOMBASE* GetRng() override;

        virtual IIndividualHumanInterventionsContext* GetInterventionsContext() const override;
        virtual IIndividualHumanInterventionsContext* GetInterventionsContextbyInfection(IInfection* infection) override;
        virtual IVaccineConsumer*                     GetVaccineContext() const;
        virtual IIndividualHumanEventContext*         GetEventContext() override;
        virtual ISusceptibilityContext*               GetSusceptibilityContext() const override;

        // IIndividualHumanEventContext methods
        virtual bool              IsPregnant()                     const override { return is_pregnant; };
        virtual float             GetAge()                         const override { return m_age; }
        virtual float             GetImmuneFailAgeAcquire()        const override;
        inline  float             getAgeInYears()                  const          { return floor(GetAge()/DAYSPERYEAR);}
        virtual int               GetGender()                      const override { return m_gender; }
        virtual float             GetMonteCarloWeight()            const override { return m_mc_weight; }
        virtual bool              IsPossibleMother()               const override;
        virtual bool              IsInfected()                     const override { return m_is_infected; }
        virtual float             GetImmunityReducedAcquire()      const override;
        virtual float             GetInterventionReducedAcquire()  const override;
        virtual HumanStateChange  GetStateChange()                 const override { return StateChange; }
        virtual void              BroadcastDeath()                       override;
        virtual void              Die( HumanStateChange )                override;

        virtual INodeEventContext   * GetNodeEventContext() override; // for campaign cost reporting in e.g. HealthSeekingBehavior
        virtual IPKeyValueContainer* GetProperties() override;
        virtual const std::string& GetPropertyReportString() const override { return m_PropertyReportString; }
        virtual void SetPropertyReportString( const std::string& str ) override { m_PropertyReportString = str; }
        virtual bool AtHome() const override;

        virtual bool IsDead() const override;

        // IMigrate
        virtual void ImmigrateTo(INodeContext* destination_node) override;
        virtual void SetMigrating( suids::suid destination, 
                                   MigrationType::Enum type, 
                                   float timeUntilTrip, 
                                   float timeAtDestination,
                                   bool isDestinationNewHome ) override;
        virtual const suids::suid& GetMigrationDestination() override;
        virtual MigrationType::Enum GetMigrationType() const override { return migration_type; }
        virtual bool IsOnFamilyTrip() const override { return is_on_family_trip; } ;
        virtual const suids::suid& GetHomeNodeId() const override { return home_node_id ; } ;

        // Migration
        virtual bool IsMigrating() override;
        virtual void CheckForMigration(float currenttime, float dt);
        void SetNextMigration();

        // Heterogeneous intra-node transmission
        virtual void UpdateGroupMembership() override;
        virtual void UpdateGroupPopulation(float size_changes) override;

        // Initialization
        virtual void SetParameters( INodeContext* pParent, float imm_mod, float risk_mod) override; // specify each parameter, default version of SetParams()
        virtual void CreateSusceptibility(float susceptibility_mod=1.0, float risk_mod=1.0);
        virtual void setupMaternalAntibodies(IIndividualHumanContext* mother, INodeContext* node) override;

        // Infections
        virtual void ExposeToInfectivity(float dt, TransmissionGroupMembership_t transmissionGroupMembership);
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route = TransmissionRoute::TRANSMISSIONROUTE_CONTACT ) override;
        virtual bool ShouldAcquire( float contagion, float dt, float suscept_mod, TransmissionRoute::Enum transmission_route = TransmissionRoute::TRANSMISSIONROUTE_CONTACT ) override;
        virtual void AcquireNewInfection( const IStrainIdentity *infstrain = nullptr, float incubation_period_override = -1.0f ) override;

        virtual const infection_list_t &GetInfections() const override;
        virtual bool IsSymptomatic() const override;
        virtual bool IsNewlySymptomatic() const override;
        virtual void UpdateInfectiousness(float dt) override;
        virtual bool InfectionExistsForThisStrain(IStrainIdentity* check_strain_id);
        virtual void ClearNewInfectionState() override;
        virtual NewInfectionState::_enum GetNewInfectionState() const override { return m_new_infection_state; }
        virtual inline float GetInfectiousness() const override { return infectiousness; }

        // Births and deaths
        virtual bool UpdatePregnancy(float dt=1) override; // returns true if birth happens this time step and resets is_pregnant to false
        virtual void InitiatePregnancy(float duration = (DAYSPERWEEK * WEEKS_FOR_GESTATION)) override;
        virtual void CheckVitalDynamics(float currenttime, float dt=1.0); // non-disease mortality
        // update and set dynamic MC weight
        virtual void UpdateMCSamplingRate(float current_sampling_rate) override;

        // Assorted getters and setters
        virtual void SetContextTo(INodeContext* context) override;
        virtual INodeContext* GetParent() const override;
        virtual inline Kernel::suids::suid GetParentSuid() const override;
        virtual ProbabilityNumber getProbMaternalTransmission() const override;

        virtual void SetGoingOnFamilyTrip( suids::suid migrationDestination, 
                                           MigrationType::Enum migrationType, 
                                           float timeUntilTrip, 
                                           float timeAtDestination,
                                           bool isDestinationNewHome ) override;
        virtual void SetWaitingToGoOnFamilyTrip() override;
        virtual void GoHome() override;

    protected:
        // Core properties
        suids::suid suid;
        float m_age;
        int   m_gender;
        float m_mc_weight;
        float m_daily_mortality_rate;
        bool  is_pregnant;      // pregnancy variables for vital_birth_dependence==INDIVIDUAL_PREGNANCIES
        float pregnancy_timer;

        // Immune system, infection(s), intervention(s), and transmission properties
        Susceptibility*               susceptibility;   // individual susceptibility (i.e. immune system)
        infection_list_t              infections;
        InterventionsContainer*       interventions;
        TransmissionGroupMembership_t transmissionGroupMembership;

        // Infections
        bool  m_is_infected;    // TODO: replace with more sophisticated strain-tracking capability
        float infectiousness;   // infectiousness calculated over all Infections and passed back to Node
        int   cumulativeInfs;   // counter of total infections over individual's history

        NewInfectionState::_enum  m_new_infection_state; // to flag various types of infection state changes
        HumanStateChange          StateChange;           // to flag that the individual has migrated or died

        // Migration
        float               migration_mod;
        MigrationType::Enum migration_type;
        suids::suid         migration_destination;
        float               migration_time_until_trip;
        float               migration_time_at_destination;
        bool                migration_is_destination_new_home;
        bool                migration_will_return;
        bool                migration_outbound;
        int                              max_waypoints;    // maximum waypoints a trip can have before returning home
        std::vector<suids::suid>         waypoints;
        std::vector<MigrationType::Enum> waypoints_trip_type;

        bool                waiting_for_family_trip ;
        bool                leave_on_family_trip ;
        bool                is_on_family_trip ;
        suids::suid         family_migration_destination ;
        MigrationType::Enum family_migration_type ;
        float               family_migration_time_until_trip ;
        float               family_migration_time_at_destination ;
        bool                family_migration_is_destination_new_home;

        suids::suid home_node_id ;

        IPKeyValueContainer Properties;
        std::string m_PropertyReportString;

        INodeContext* parent;   // Access back to node/simulation methods

        IndividualHuman(INodeContext *context);
        IndividualHuman(suids::suid id = suids::nil_suid(), float MCweight = 1.0f, float init_age = 0.0f, int gender = 0);

        virtual IInfection* createInfection(suids::suid _suid); // factory method (overridden in derived classes)
        virtual void setupInterventionsContainer();            // derived classes can customize the container, and hence the interventions supported, by overriding this method
        virtual void applyNewInterventionEffects(float dt);    // overriden when interventions (e.g. polio vaccine) updates individual properties (e.g. immunity)
        virtual void UpdateAge( float dt );

        float GetRoundTripDurationRate( MigrationType::Enum trip_type );

        // Infection updating
        virtual bool SetNewInfectionState(InfectionStateChange::_enum inf_state_change);
        virtual void ReportInfectionState();

        virtual void PropagateContextToDependents();
        IIndividualEventBroadcaster* broadcaster;

    private:

        bool m_newly_symptomatic;

        virtual IIndividualHumanContext* GetContextPointer();
    };
}
