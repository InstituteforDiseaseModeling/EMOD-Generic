/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Individual.h"

#include <list>
#include <map>

#include "Common.h"
#include "ConfigParams.h"
#include "IContagionPopulation.h"
#include "Debug.h"
#include "Exceptions.h"
#include "IndividualEventContext.h"
#include "Infection.h"
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "IMigrationInfo.h"
#include "INodeContext.h"
#include "NodeDemographics.h"
#include "NodeEventContext.h"
#include "suids.hpp"
#include "Susceptibility.h"
#include "Properties.h"
#include "StrainIdentity.h"
#include "EventTrigger.h"
#include "ISimulationContext.h"
#include "RANDOM.h"
#include "RapidJsonImpl.h" // Once JSON lib wrapper is completely done, this underlying JSON library specific include can be taken out
#include <IArchive.h>

SETUP_LOGGING( "Individual" )

namespace Kernel
{
    bool IndividualHumanConfig::aging            = false;
    bool IndividualHumanConfig::enable_immunity  = false;
    bool IndividualHumanConfig::superinfection   = false;
    bool IndividualHumanConfig::enable_skipping  = false;

    int   IndividualHumanConfig::infection_updates_per_tstep =  0;
    int   IndividualHumanConfig::max_ind_inf                 =  1;

    // QI stuff in case we want to use it more extensively outside of campaigns
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Individual,IndividualHumanConfig)
    BEGIN_QUERY_INTERFACE_BODY(IndividualHumanConfig)
    END_QUERY_INTERFACE_BODY(IndividualHumanConfig)

    //------------------------------------------------------------------
    //   Initialization methods
    //------------------------------------------------------------------

    bool IndividualHumanConfig::Configure( const Configuration* config )
    {
        LOG_DEBUG( "Configure\n" );

        initConfigTypeMap( "Enable_Aging",           &aging,            Enable_Aging_DESC_TEXT,            true, "Enable_Vital_Dynamics" );

        initConfigTypeMap( "Enable_Immunity",        &enable_immunity,  Enable_Immunity_DESC_TEXT,         true, "Simulation_Type", "GENERIC_SIM,VECTOR_SIM,STI_SIM,ENVIRONMENTAL_SIM,TBHIV_SIM,PY_SIM");
        initConfigTypeMap( "Enable_Skipping",        &enable_skipping,  Enable_Skipping_DESC_TEXT,        false, "Simulation_Type", "GENERIC_SIM,TYPHOID_SIM,TBHIV_SIM" );
        initConfigTypeMap( "Enable_Superinfection",  &superinfection,   Enable_Superinfection_DESC_TEXT,  false, "Simulation_Type", "VECTOR_SIM,STI_SIM,ENVIRONMENTAL_SIM,MALARIA_SIM");

        initConfigTypeMap( "Infection_Updates_Per_Timestep",  &infection_updates_per_tstep,  Infection_Updates_Per_Timestep_DESC_TEXT,   0,     144,  1 );
        initConfigTypeMap( "Max_Individual_Infections",       &max_ind_inf,                  Max_Individual_Infections_DESC_TEXT,        0,    1000,  1,    "Enable_Superinfection" );

        bool bRet = JsonConfigurable::Configure( config );

        return bRet;
    }

    IndividualHuman::IndividualHuman(suids::suid _suid, float mc_weight, float initial_age, int gender)
        : suid(_suid)
        , m_age(initial_age)
        , m_gender(gender)
        , m_mc_weight(mc_weight)
        , m_daily_mortality_rate(0)
        , is_pregnant(false)
        , pregnancy_timer(FLT_MAX)
        , susceptibility(nullptr)
        , infections()
        , interventions(nullptr)
        , transmissionGroupMembership()
        , m_is_infected(false)
        , infectiousness(0.0f)
        , cumulativeInfs(0)
        , m_new_infection_state(NewInfectionState::Invalid)
        , StateChange(HumanStateChange::None)
        , migration_type(MigrationType::NO_MIGRATION)
        , migration_destination(suids::nil_suid())
        , migration_time_until_trip(0.0)
        , migration_time_at_destination(0.0)
        , migration_is_destination_new_home(false)
        , migration_will_return(false)
        , migration_outbound(false)
        , max_waypoints(0)
        , waypoints()
        , waypoints_trip_type()
        , waiting_for_family_trip(false)
        , leave_on_family_trip(false)
        , is_on_family_trip(false)
        , family_migration_destination(suids::nil_suid())
        , family_migration_type(MigrationType::NO_MIGRATION)
        , family_migration_time_until_trip(0.0)
        , family_migration_time_at_destination(0.0)
        , family_migration_is_destination_new_home(false)
        , home_node_id(suids::nil_suid())
        , Properties()
        , m_PropertyReportString()
        , parent(nullptr)
        , broadcaster(nullptr)
        , m_newly_symptomatic(false)
    {
    }

    IndividualHuman::IndividualHuman(INodeContext *context)
        : suid(suids::nil_suid())
        , m_age(0)
        , m_gender(0)
        , m_mc_weight(0)
        , m_daily_mortality_rate(0)
        , is_pregnant(false)
        , pregnancy_timer(FLT_MAX)
        , susceptibility(nullptr)
        , infections()
        , interventions(nullptr)
        , transmissionGroupMembership()
        , m_is_infected(false)
        , infectiousness(0.0f)
        , cumulativeInfs(0)
        , m_new_infection_state(NewInfectionState::Invalid)
        , StateChange(HumanStateChange::None)
        , migration_type(MigrationType::NO_MIGRATION)
        , migration_destination(suids::nil_suid())
        , migration_time_until_trip(0.0)
        , migration_time_at_destination(0.0)
        , migration_is_destination_new_home(false)
        , migration_will_return(false)
        , migration_outbound(false)
        , max_waypoints(0)
        , waypoints()
        , waypoints_trip_type()
        , waiting_for_family_trip(false)
        , leave_on_family_trip(false)
        , is_on_family_trip(false)
        , family_migration_destination(suids::nil_suid())
        , family_migration_type(MigrationType::NO_MIGRATION)
        , family_migration_time_until_trip(0.0)
        , family_migration_time_at_destination(0.0)
        , family_migration_is_destination_new_home(false)
        , home_node_id(suids::nil_suid())
        , Properties()
        , m_PropertyReportString()
        , parent(nullptr)
        , broadcaster(nullptr)
        , m_newly_symptomatic( false )
    {
    }

    IndividualHuman::~IndividualHuman()
    {
        for (auto infection : infections)
        {
            delete infection;
        }

        delete susceptibility;
        delete interventions;
    }

    QueryResult IndividualHuman::QueryInterface( iid_t iid, void** ppinstance )
    {
        release_assert(ppinstance); // todo: add a real message: "QueryInterface requires a non-NULL destination!");

        ISupports* foundInterface;
        if ( iid == GET_IID(IIndividualHumanEventContext))
            foundInterface = static_cast<IIndividualHumanEventContext*>(this);
        else if ( iid == GET_IID(IIndividualHumanContext))
            foundInterface = static_cast<IIndividualHumanContext*>(this);
        else if ( iid == GET_IID(IIndividualHuman)) 
            foundInterface = static_cast<IIndividualHuman*>(this);
        else if ( iid == GET_IID(IInfectable))
            foundInterface = static_cast<IInfectable*>(this);
        else if ( iid == GET_IID(IInfectionAcquirable))
            foundInterface = static_cast<IInfectionAcquirable*>(this);
        else if ( iid == GET_IID(IMigrate))
            foundInterface = static_cast<IMigrate*>(this);
        else if ( iid == GET_IID(ISupports) )
            foundInterface = static_cast<ISupports*>(static_cast<IIndividualHumanContext*>(this));
        else if (iid == GET_IID(IGlobalContext))
            parent->QueryInterface(iid, reinterpret_cast<void**>(&foundInterface));
        else
            foundInterface = nullptr;

        QueryResult status = e_NOINTERFACE;
        if ( foundInterface )
        {
            foundInterface->AddRef();
            status = s_OK;
        }

        *ppinstance = foundInterface;
        return status;
    }

    IndividualHuman *IndividualHuman::CreateHuman()
    {
        IndividualHuman *newindividual = _new_ IndividualHuman();

        return newindividual;
    }

    IndividualHuman *IndividualHuman::CreateHuman(INodeContext *context, suids::suid id, float MCweight, float init_age, int gender)
    {
        IndividualHuman *newhuman = _new_ IndividualHuman(id, MCweight, init_age, gender);

        newhuman->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newhuman->m_age );

        return newhuman;
    }

    void IndividualHuman::InitializeHuman()
    {
        release_assert( parent );
        home_node_id = parent->GetSuid() ;
    }

    bool IndividualHuman::IsDead() const
    {
        bool is_dead = (StateChange == HumanStateChange::DiedFromNaturalCauses) ||
                       (StateChange == HumanStateChange::KilledByInfection)     ||
                       (StateChange == HumanStateChange::KilledByMCSampling);

        return is_dead ;
    }

    IMigrate* IndividualHuman::GetIMigrate()
    {
        return static_cast<IMigrate*>(this);
    }

    void IndividualHuman::SetContextTo(INodeContext* context)
    {
        INodeContext *old_context = parent;
        parent = context;

        if (parent)
        {
            // clear migration_destination whenever we've migrated to a new node
            // (as opposed to initial simulation initialization, new births, or outbreak import cases)
            if(parent->GetSuid() == migration_destination)
            {
                // -----------------------------------------------------------
                // --- If we've returned to the node we started at, clear the
                // --- migration parameters so we can select a new destination.
                // -----------------------------------------------------------
                if( !migration_outbound && (waypoints.size() == 1) && (waypoints[ 0 ] == migration_destination) )
                {
                    waypoints.clear();
                    migration_outbound    = true;
                    migration_will_return = true;
                }

                migration_destination = suids::nil_suid();
            }

            if( is_on_family_trip && (parent->GetSuid() == home_node_id) )
            {
                is_on_family_trip = false ;
            }

            // need to do this *after* (potentially) clearing waypoints above, so that AtHome() can return true
            PropagateContextToDependents();
        }
        else if(old_context)
        {
            if(migration_outbound)
            {
                if(migration_will_return)
                {
                    waypoints.push_back(old_context->GetSuid());
                    waypoints_trip_type.push_back(migration_type);
                }
            }
            else if( waypoints.size() > 0 )
            {
                waypoints.pop_back();
                waypoints_trip_type.pop_back();
            }
        }


        if( parent && parent->GetEventContext() )
        {
            broadcaster = parent->GetEventContext()->GetIndividualEventBroadcaster();
        }
    }

    void IndividualHuman::setupInterventionsContainer()
    {
        // set up a default interventions context
        // derived classes may override this
        interventions = _new_ InterventionsContainer();
    }

    void IndividualHuman::CreateSusceptibility(float init_mod_acq, float init_mod_risk)
    {
        LOG_DEBUG_F( "%s: Creating susceptibility; individual %d; init_mod_acq %f; init_mod_risk %f\n", __FUNCTION__, suid.data, init_mod_acq, init_mod_risk );
        susceptibility = Susceptibility::CreateSusceptibility(this, init_mod_acq, init_mod_risk);
    }

    void IndividualHuman::SetParameters( INodeContext* pParent, float susceptibility_modifier, float risk_modifier)
    {
        StateChange       = HumanStateChange::None;

        // migration stuff
        migration_outbound    = true;
        migration_will_return = true;

        // set to 0
        is_pregnant     = false;
        pregnancy_timer = 0;

        ClearNewInfectionState();
        susceptibility = nullptr;

        setupInterventionsContainer();
        // The original version caused a lot of issues when querying IIndividualHumanContext functions, initializing it
        // with the correct context works.  Serialization did it ok, which means when serializing and deserializing, it fixed the problem
        // making it harder to originally find
        IIndividualHumanContext *indcontext = GetContextPointer();
        interventions->SetContextTo(indcontext); //TODO: fix this when init pattern standardized <ERAD-291>  PE: See comment above

        CreateSusceptibility(susceptibility_modifier, risk_modifier);

        // Populate the individuals set of Individual Properties with one value for each property
        if( IPFactory::GetInstance() )
        {
            Properties = IPFactory::GetInstance()->GetInitialValues( pParent->GetExternalID(), GetRng() );
        }
    }

    void IndividualHuman::UpdateMCSamplingRate(float desired_mc_weight)
    {

        UpdateGroupPopulation(-1.0f);
        m_mc_weight = desired_mc_weight;
        UpdateGroupPopulation( 1.0f);
        LOG_DEBUG_F( "Changed mc_weight to %f for individual %d\n.", m_mc_weight, suid.data );
    }

    void IndividualHuman::setupMaternalAntibodies(IIndividualHumanContext* mother, INodeContext* node)
    {
        // derived classes can determine the specific initialization of maternal antibodies related to mother or possible mothers
    }

    void IndividualHuman::PropagateContextToDependents()
    {
        IIndividualHumanContext *context = GetContextPointer();

        // fix up child pointers
        for (auto infection : infections)
        {
            infection->SetContextTo(context);
        }

        if (susceptibility) susceptibility->SetContextTo(context);
        if (interventions)  interventions->SetContextTo(context);
    }

    //------------------------------------------------------------------
    //   Update methods
    //------------------------------------------------------------------

    void IndividualHuman::Update(float currenttime, float dt)
    {
        LOG_VALID_F( "%s\n", __FUNCTION__ );
        float infection_timestep = dt;
        int numsteps = 1;

        // eventually need to correct for time step in case of individuals moving among communities with different adapted time steps

        StateChange = HumanStateChange::None;

        //  Aging
        if (IndividualHumanConfig::aging)
        {
            UpdateAge( dt );
        }

        // Adjust time step for infections as specified by infection_updates_per_tstep.  A value of 0 reverts to a single update per time step for backward compatibility.
        // There is no special meaning of 1 being hourly.  For hourly infection updates with a tstep of one day, one must now specify 24.
        if ( IndividualHumanConfig::infection_updates_per_tstep > 1 )
        {
            // infection_updates_per_tstep is now an integer > 1, so set numsteps equal to it,
            // allowing the subdivision dt into smaller infection_timestep
            numsteps = IndividualHumanConfig::infection_updates_per_tstep;
            infection_timestep = dt / numsteps;
        }

        // Process list of infections
        if (infections.size() == 0) // don't need to process infections or go hour by hour
        {
            release_assert( susceptibility );
            susceptibility->Update(dt);
            release_assert( interventions );
            interventions->InfectiousLoopUpdate( dt );
            interventions->Update( dt );
        }
        else
        {
            for (int i = 0; i < numsteps; i++)
            {
                bool prev_symptomatic = IsSymptomatic();
                for (auto it = infections.begin(); it != infections.end();)
                {
                    // Update infection
                    (*it)->Update(infection_timestep, susceptibility);
                    // Note that the newly calculated infestiousness from the Update above won't get used (shed) until the next timestep
                    // Node::updateInfectivity is called before Individual::Update (this function)

                    // Check for a new infection/human state (e.g. Fatal, Cleared)
                    InfectionStateChange::_enum inf_state_change = (*it)->GetStateChange();
                    if (inf_state_change != InfectionStateChange::None)
                    {
                        SetNewInfectionState(inf_state_change);

                        // Notify susceptibility of cleared infection and remove from list
                        if ( inf_state_change == InfectionStateChange::Cleared )
                        {
                            LOG_DEBUG_F( "Individual %d's infection cleared.\n", GetSuid().data );

                            if( broadcaster )
                            {
                                broadcaster->TriggerObservers( GetEventContext(), EventTrigger::InfectionCleared );
                            }

                            if ( IndividualHumanConfig::enable_immunity )
                            {
                                susceptibility->UpdateInfectionCleared();
                            }

                            delete *it;
                            it = infections.erase(it);
                            continue;
                        }

                        // Set human state change and stop updating infections if the person has died
                        if ( inf_state_change == InfectionStateChange::Fatal )
                        {
                            Die( HumanStateChange::KilledByInfection );
                            break;
                        }
                    }
                    ++it;
                }

                if ( IndividualHumanConfig::enable_immunity )
                {
                    susceptibility->Update(infection_timestep);      // Immunity update: mainly decay of immunity
                }

                m_newly_symptomatic = !prev_symptomatic && IsSymptomatic();
                if( m_newly_symptomatic && broadcaster != nullptr )
                {
                    broadcaster->TriggerObservers( GetEventContext(), EventTrigger::NewlySymptomatic );
                }

                if( prev_symptomatic && !IsSymptomatic() && broadcaster ) //no longer symptomatic 
                {
                    broadcaster->TriggerObservers( GetEventContext(), EventTrigger::SymptomaticCleared );
                }

                if (StateChange == HumanStateChange::KilledByInfection)
                {
                    break; // If individual died, no need to keep simulating infections.
                }
                interventions->InfectiousLoopUpdate( infection_timestep );
            }
            if( StateChange != HumanStateChange::KilledByInfection )
            {
                interventions->Update( dt );
            }
        }

        applyNewInterventionEffects(dt);

        // Trigger "every-update" event observers 
        if( broadcaster )
        {
            broadcaster->TriggerObservers(GetEventContext(), EventTrigger::EveryUpdate);
        }

        //  Get new infections
        ExposeToInfectivity(dt, transmissionGroupMembership); // Need to do it even if infectivity==0, because of diseases in which immunity of acquisition depends on challenge (eg malaria)
        if( broadcaster )
        {
            broadcaster->TriggerObservers(GetEventContext(), EventTrigger::ExposureComplete);
        }

        //  Is there an active infection for statistical purposes?
        m_is_infected = (infections.size() > 0);

        // Check if died of natural causes
        if( parent->GetParams()->enable_natural_mortality && StateChange == HumanStateChange::None )
        {
            CheckVitalDynamics(currenttime, dt);
        }

        if (StateChange == HumanStateChange::None && parent->GetMigrationInfo() && parent->GetMigrationInfo()->GetParams()->migration_structure) // Individual can't migrate if they're already dead
        {
            CheckForMigration(currenttime, dt);
        }

        if( (m_new_infection_state == NewInfectionState::NewInfection) ||
            (m_new_infection_state == NewInfectionState::NewAndDetected) )
        {
            if( broadcaster )
            {
                broadcaster->TriggerObservers( GetEventContext(), EventTrigger::NewInfection );
            }
        }

        // Age-dependent event triggers go last for backward compatibility
        if ( broadcaster && IndividualHumanConfig::aging )
        {
            if( ((m_age - dt) <  6.0f*DAYSPERWEEK)       && ( 6.0f*DAYSPERWEEK       <= m_age) )
            {
                broadcaster->TriggerObservers( GetEventContext(), EventTrigger::SixWeeksOld );
            }
            if( ((m_age - dt) <  6.0f*IDEALDAYSPERMONTH) && ( 6.0f*IDEALDAYSPERMONTH <= m_age) )
            {
                broadcaster->TriggerObservers( GetEventContext(), EventTrigger::SixMonthsOld );
            }
            if( ((m_age - dt) <  1.0f*DAYSPERYEAR)       && ( 1.0f*DAYSPERYEAR       <= m_age) )
            {
                broadcaster->TriggerObservers( GetEventContext(), EventTrigger::OneYearOld );
            }
            if( ((m_age - dt) < 18.0f*IDEALDAYSPERMONTH) && (18.0f*IDEALDAYSPERMONTH <= m_age) )
            {
                broadcaster->TriggerObservers( GetEventContext(), EventTrigger::EighteenMonthsOld );
            }
        }

    }

    void IndividualHuman::UpdateAge( float dt )
    {
        float age_was = m_age;

        m_age += dt;

        // broadcast an event when a person passes their birthday
        float age_years = m_age / DAYSPERYEAR;
        float years = float( int( age_years ) );
        float birthday_in_days = years * DAYSPERYEAR;
        if( (broadcaster != nullptr) && (age_was < birthday_in_days) && (birthday_in_days <= m_age) )
        {
            broadcaster->TriggerObservers( GetEventContext(), EventTrigger::HappyBirthday );
        }
    }

    void IndividualHuman::CheckVitalDynamics(float currenttime, float dt)
    {
        if( m_daily_mortality_rate == 0 || fmodf( GetAge(), IDEALDAYSPERMONTH) < dt )
        {
            m_daily_mortality_rate = parent->GetNonDiseaseMortalityRateByAgeAndSex( m_age, Gender::Enum( GetGender() ) );
        }

        float adjusted_rate = m_daily_mortality_rate * dt;
        
        if( (GetRng()!= nullptr) && GetRng()->SmartDraw( adjusted_rate ) )
        {
            Die( HumanStateChange::DiedFromNaturalCauses );
        }
    }

    void IndividualHuman::applyNewInterventionEffects(float dt)
    { }

    float IndividualHuman::GetImmunityReducedAcquire() const
    {
        return susceptibility->getModAcquire();
    }

    float IndividualHuman::GetInterventionReducedAcquire() const
    {
        return interventions->GetInterventionReducedAcquire();
    }

    float IndividualHuman::GetImmuneFailAgeAcquire() const
    {
        return susceptibility->getImmuneFailAgeAcquire();
    }

    bool IndividualHuman::UpdatePregnancy(float dt)
    {
        bool birth_this_timestep = false;
        if (is_pregnant)
        {
            pregnancy_timer -= dt;
            if (pregnancy_timer <= 0)
            {
                is_pregnant         = false;
                pregnancy_timer     = 0;
                birth_this_timestep = true;

                // Broadcast GaveBirth
                if( GetNodeEventContext() != nullptr )
                {
                    IIndividualEventBroadcaster* broadcaster = GetNodeEventContext()->GetIndividualEventBroadcaster();
                    broadcaster->TriggerObservers( GetEventContext(), EventTrigger::GaveBirth );
                }
            }
        }

        return birth_this_timestep;
    }

    void IndividualHuman::InitiatePregnancy(float duration)
    {
        if (!is_pregnant)
        {
            is_pregnant = true;
            pregnancy_timer = duration; 

            // Broadcast Pregnant
            if( GetNodeEventContext() != nullptr )
            {
                IIndividualEventBroadcaster* broadcaster = GetNodeEventContext()->GetIndividualEventBroadcaster();
                broadcaster->TriggerObservers( GetEventContext(), EventTrigger::Pregnant );
            }
        }
    }

    //------------------------------------------------------------------
    //   Migration methods
    //------------------------------------------------------------------

    void IndividualHuman::ImmigrateTo(INodeContext* destination_node)
    {
        if( destination_node == nullptr )
        {
            throw NullPointerException( __FILE__, __LINE__, __FUNCTION__, "destination_node", "INodeContext" );
        }

        destination_node->processImmigratingIndividual(this);
        if( migration_is_destination_new_home )
        {
            home_node_id = destination_node->GetSuid();
            migration_is_destination_new_home = false;
            migration_outbound = false ;
            migration_will_return = false ;
            waypoints.clear();
            waypoints_trip_type.clear();
        }
    }

    void IndividualHuman::CheckForMigration(float currenttime, float dt)
    {
        //  Determine if individual moves during this time step
        switch (parent->GetMigrationInfo()->GetParams()->migration_structure)
        {
        case MigrationStructure::FIXED_RATE_MIGRATION:
            if( leave_on_family_trip )
            {
                migration_outbound                = true;
                migration_will_return             = true;
                migration_destination             = family_migration_destination;
                migration_type                    = family_migration_type;
                migration_time_until_trip         = family_migration_time_until_trip;
                migration_time_at_destination     = family_migration_time_at_destination;
                migration_is_destination_new_home = family_migration_is_destination_new_home;
                is_on_family_trip                 = true;

                leave_on_family_trip             = false;
                family_migration_destination     = suids::nil_suid();
                family_migration_type            = MigrationType::NO_MIGRATION;
                family_migration_time_until_trip = 0.0 ;
            }
            else if( !waiting_for_family_trip )
            {
                if( migration_destination.is_nil() )
                    SetNextMigration();
            }

            if( !migration_destination.is_nil() )
            {
                migration_time_until_trip -= dt*(parent->GetEventContext()->GetOutboundConnectionModifier());

                // --------------------------------------------------------------------
                // --- This check should really be zero, but epsilon makes the test for
                // --- this pass as expected.  Namely, it helps things work more like
                // --- you'd expect with an intervention where the times might be round numbers.
                // --------------------------------------------------------------------
                if( migration_time_until_trip <= 0.0000001f )
                {
                    float dest_modifier = parent->GetParent()->GetNodeInboundMultiplier(migration_destination);

                    if(GetRng()->SmartDraw(dest_modifier))
                    {
                        LOG_DEBUG_F( "%s: individual %d is migrating.\n", __FUNCTION__, suid.data );
                        StateChange = HumanStateChange::Migrating;
                    }
                    else
                    {
                        LOG_DEBUG_F( "%s: individual %d not migrating due to destination restriction.\n", __FUNCTION__, suid.data );
                        SetNextMigration();
                    }
                    
                }
            }
            break;

        case MigrationStructure::NO_MIGRATION:
        default:
            std::stringstream msg;
            msg << "Invalid migration_structure=" << parent->GetMigrationInfo()->GetParams()->migration_structure;
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            // break;
        }
    }

    void IndividualHuman::SetNextMigration(void)
    {
        IMigrationInfo* migration_info = parent->GetMigrationInfo();

        // ----------------------------------------------------------------------------------------
        // --- We don't want the check for reachable nodes here because one could travel to a node
        // --- that doesn't let its residents travel and we need the ability to get back.
        // --- That is, I should be able to travel to a node and return from it, even if the
        // --- residents of the node do not migrate.
        // ----------------------------------------------------------------------------------------
        if( migration_info->GetParams()->migration_structure != MigrationStructure::NO_MIGRATION )
        {
            if(waypoints.size() == 0)
                migration_outbound = true;
            else if(waypoints.size() == migration_info->GetParams()->roundtrip_waypoints)
                migration_outbound = false;

            if( migration_outbound && (migration_info->GetReachableNodes().size() > 0) )
            {
                migration_info->PickMigrationStep( GetRng(), this, migration_destination, migration_type, migration_time_until_trip );

                if( migration_type == MigrationType::NO_MIGRATION )
                {
                    return ;
                }
                else if( migration_type == MigrationType::FAMILY_MIGRATION )
                {
                    waiting_for_family_trip = true ;

                    float time_at_destination = GetRoundTripDurationRate( migration_type );
                    parent->SetWaitingForFamilyTrip( migration_destination, 
                                                     migration_type,
                                                     migration_time_until_trip,
                                                     time_at_destination,
                                                     false );

                    migration_destination = suids::nil_suid();
                    migration_type = MigrationType::NO_MIGRATION;
                    migration_time_until_trip = 0.0 ;
                    migration_will_return = true; // family trips must return
                }
                else
                {
                    float return_prob;
                    switch(migration_type)
                    {
                        case MigrationType::LOCAL_MIGRATION:    return_prob = migration_info->GetParams()->local_roundtrip_prob;  break;
                        case MigrationType::AIR_MIGRATION:      return_prob = migration_info->GetParams()->air_roundtrip_prob;    break;
                        case MigrationType::REGIONAL_MIGRATION: return_prob = migration_info->GetParams()->region_roundtrip_prob; break;
                        case MigrationType::SEA_MIGRATION:      return_prob = migration_info->GetParams()->sea_roundtrip_prob;    break;
                        default:
                            throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "migration_type", migration_type, MigrationType::pairs::lookup_key(migration_type) );
                    }

                    migration_will_return = GetRng()->SmartDraw( return_prob );
                }
            }
            else if( waypoints.size() > 0 )
            {
                migration_destination = waypoints.back();
                MigrationType::Enum trip_type = waypoints_trip_type.back();


                if( migration_time_at_destination > 0.0f )
                {
                    migration_time_until_trip = migration_time_at_destination ;
                    migration_time_at_destination = 0.0f ;
                }
                else
                {
                    migration_time_until_trip = GetRoundTripDurationRate( trip_type );
                }
            }
        }
    }

    float IndividualHuman::GetRoundTripDurationRate( MigrationType::Enum trip_type )
    {
        IMigrationInfo* migration_info = parent->GetMigrationInfo();

        float duration_value = 0.0f;

        switch(trip_type)
        {
            case MigrationType::LOCAL_MIGRATION:    duration_value = migration_info->GetParams()->local_roundtrip_duration;  break;
            case MigrationType::AIR_MIGRATION:      duration_value = migration_info->GetParams()->air_roundtrip_duration;    break;
            case MigrationType::REGIONAL_MIGRATION: duration_value = migration_info->GetParams()->region_roundtrip_duration; break;
            case MigrationType::SEA_MIGRATION:      duration_value = migration_info->GetParams()->sea_roundtrip_duration;    break;
            case MigrationType::FAMILY_MIGRATION:   duration_value = migration_info->GetParams()->family_roundtrip_duration; break;
            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "trip_type", trip_type, MigrationType::pairs::lookup_key( migration_type ) );
        }

        if( duration_value > 0.0f )
        {
            duration_value = static_cast<float>( GetRng()->expdist( 1.0f/duration_value ) );
        }

        return duration_value;
    }

    const suids::suid& IndividualHuman::GetMigrationDestination()
    {
        return migration_destination;
    }

    bool IndividualHuman::IsMigrating()
    {
        return StateChange == HumanStateChange::Migrating;
    }

    void IndividualHuman::UpdateGroupMembership()
    {
        tProperties properties = GetProperties()->GetOldVersion();
        const RouteList_t& routes = parent->GetTransmissionRoutes();

        parent->GetGroupMembershipForIndividual( routes, properties, transmissionGroupMembership );
    }

    void IndividualHuman::UpdateGroupPopulation(float size_changes)
    {
        parent->UpdateTransmissionGroupPopulation( GetProperties()->GetOldVersion(), size_changes, this->GetMonteCarloWeight() );
    }

    bool IndividualHuman::AtHome() const
    {
        return home_node_id == parent->GetSuid();
    }

    void IndividualHuman::GoHome()
    {
        migration_destination = home_node_id ;
    }

    void IndividualHuman::SetGoingOnFamilyTrip( suids::suid migrationDestination,
                                                MigrationType::Enum migrationType,
                                                float timeUntilTrip,
                                                float timeAtDestination,
                                                bool isDestinationNewHome )
    {
        leave_on_family_trip                     = true;
        family_migration_destination             = migrationDestination;
        family_migration_type                    = migrationType;
        family_migration_time_until_trip         = timeUntilTrip;
        family_migration_time_at_destination     = timeAtDestination;
        family_migration_is_destination_new_home = isDestinationNewHome;
        waiting_for_family_trip                  = false;

        if( family_migration_time_at_destination <= 0.0 )
        {
            // -----------------------------------------------------------------------------------------------
            // --- If the user were to set the time at destion to be equal to zero (or gets set randomly),
            // --- we want to ensure that the actual value used is not zero.  The migration logic assumes
            // --- that when migration_time_at_destination > 0, this value is used versus using other return trip logic.
            // -----------------------------------------------------------------------------------------------
            family_migration_time_at_destination = 0.001f;
        }
    }

    void IndividualHuman::SetWaitingToGoOnFamilyTrip()
    {
        waiting_for_family_trip   = true ;
        migration_destination     = suids::nil_suid();
        migration_time_until_trip = 0.0 ;
    }

    void IndividualHuman::SetMigrating( suids::suid destination, 
                                        MigrationType::Enum type, 
                                        float timeUntilTrip, 
                                        float timeAtDestination,
                                        bool isDestinationNewHome )
    {
        if( parent->GetSuid().data != destination.data )
        {
            migration_destination             = destination;
            migration_type                    = type;
            migration_time_until_trip         = timeUntilTrip;
            migration_time_at_destination     = timeAtDestination;
            migration_is_destination_new_home = isDestinationNewHome;
            migration_outbound                = !isDestinationNewHome;
            migration_will_return             = !isDestinationNewHome;

            if( migration_time_at_destination <= 0.0 )
            {
                // -----------------------------------------------------------------------------------------------
                // --- If the user were to set the time at destion to be equal to zero (or gets set randomly),
                // --- we want to ensure that the actual value used is not zero.  The migration logic assumes
                // --- that when migration_time_at_destination > 0, this value is used versus using other return trip logic.
                // -----------------------------------------------------------------------------------------------
                migration_time_at_destination = 0.001f;
            }
        }
    }

    //------------------------------------------------------------------
    //   Infection methods
    //------------------------------------------------------------------

    void IndividualHuman::ExposeToInfectivity(float dt, TransmissionGroupMembership_t transmissionGroupMembership)
    {
        parent->ExposeIndividual(static_cast<IInfectable*>(this), transmissionGroupMembership, dt);
    }

    // TODO: port normal exposure_to_infectivity logic to this pattern as well <ERAD-328>
    void IndividualHuman::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route)
    {
        ProbabilityNumber prob = EXPCDF(-cp->GetTotalContagion()*dt*susceptibility->getModAcquire()*susceptibility->getModRisk()*interventions->GetInterventionReducedAcquire());
        LOG_DEBUG_F( "id = %lu, group_id = %d, total contagion = %f, dt = %f, immunity factor = %f, interventions factor = %f, prob=%f, infectiousness=%f\n",
                     GetSuid().data, transmissionGroupMembership.group, cp->GetTotalContagion(), dt, susceptibility->getModAcquire(), interventions->GetInterventionReducedAcquire(), float(prob), infectiousness);
        bool acquire = false; 
        if( IndividualHumanConfig::enable_skipping ) 
        {
            float maxProb = parent->GetMaxInfectionProb( transmission_route ); 
            if( maxProb > 0 )
            {
                release_assert(maxProb>=0.0 && maxProb<=1.0);
                LOG_DEBUG_F( "Using maxProb of %f.\n", float( maxProb ) );
                /*if( maxProb < prob) // occasionally need this when debugging.
                {
                    LOG_ERR_F( "maxProb = %f, prob = %f.\n", maxProb, float(prob) );
                }*/
                release_assert(maxProb>=prob);

                if( maxProb==prob ) // individual is maximally susceptible
                {
                    LOG_DEBUG_F( "Individual is maximally susceptible." );
                    acquire = true; 
                }
                else if( prob/maxProb > GetRng()->e() )
                {
                    // DLC - individual is not maximally susceptible
                    LOG_DEBUG_F( "Individual is infected stochastically." );
                    acquire = true;
                }
            }
        }
        else
        {
            if( GetRng()->SmartDraw( prob ) ) // infection results from this strain? 
            {
                acquire = true;
            }
        }
        LOG_VALID_F( "acquire = %d.\n", acquire );

        if( acquire ) 
        {
            LOG_VALID_F( "Individual %d is acquiring a new infection on route %s.\n", GetSuid().data, TransmissionRoute::pairs::lookup_key( transmission_route ) );
            AcquireNewInfection( (IStrainIdentity*)cp );
        }
    }

    bool IndividualHuman::ShouldAcquire(float contagion, float dt, float suscept_mod, TransmissionRoute::Enum transmission_route)
    {
        throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Function exists due to inheritance. Not a valid code path for GENERIC_SIM." );
    }

    void IndividualHuman::AcquireNewInfection( const IStrainIdentity *strain_ptr, float incubation_period_override )
    {
        StrainIdentity newStrainId;
        if( strain_ptr != nullptr )
        {
            strain_ptr->ResolveInfectingStrain( &newStrainId ); // get the clade and genome ID
        }

        int numInfs = int(infections.size());
        if ( (IndividualHumanConfig::superinfection && (numInfs < IndividualHumanConfig::max_ind_inf)) || (numInfs == 0) )
        {
            cumulativeInfs++;
            m_is_infected = true;

            IInfection* newinf = createInfection( parent->GetNextInfectionSuid() );
            newinf->SetParameters(&newStrainId, incubation_period_override);
            newinf->InitInfectionImmunology(susceptibility);

            LOG_DEBUG( "Adding infection to infections list.\n" );
            infections.push_front(newinf);
            infectiousness += newinf->GetInfectiousness();
            ReportInfectionState(); // can specify different reporting in derived classes
        }
    }

    void IndividualHuman::UpdateInfectiousness(float dt)
    {
        interventions->PreInfectivityUpdate( dt );

        infectiousness = 0;

        if ( infections.size() == 0 )
            return;

        for (auto infection : infections)
        {
            infectiousness += infection->GetInfectiousness();
            float tmp_infectiousness =  m_mc_weight * infection->GetInfectiousness() * susceptibility->getModTransmit() * interventions->GetInterventionReducedTransmit();
            StrainIdentity tmp_strainIDs;
            infection->GetInfectiousStrainID(&tmp_strainIDs);
            if(GetParams()->enable_label_infector)
            {
                uint64_t new_genome = (static_cast<uint64_t>(GetSuid().data) << SHIFT_BIT) + (tmp_strainIDs.GetGeneticID() & MAX_24BIT);
                tmp_strainIDs.SetGeneticID(new_genome);
            }
            if( tmp_infectiousness )
            {
                LOG_DEBUG_F( "Individual %d depositing contagion into transmission group.\n", GetSuid().data );
                parent->DepositFromIndividual( tmp_strainIDs, tmp_infectiousness, transmissionGroupMembership );
            }
        }
        float raw_inf = infectiousness;
        infectiousness *= susceptibility->getModTransmit() * interventions->GetInterventionReducedTransmit();
        LOG_VALID_F("Infectiousness for individual %d = %f (raw=%f, immunity modifier=%f, intervention modifier=%f, weight=%f).\n",
            GetSuid().data, infectiousness, raw_inf, susceptibility->getModTransmit(), interventions->GetInterventionReducedTransmit(), m_mc_weight);
    }

    bool IndividualHuman::InfectionExistsForThisStrain(IStrainIdentity* check_strain_id)
    {
        for (auto infection : infections)
        {
            if (infection->StrainMatches( check_strain_id ) )
            {
                return true;
            }
        }

        return false;
    }

    IInfection* IndividualHuman::createInfection(suids::suid _suid)
    {
        return Infection::CreateInfection(this, _suid);
    }

    bool IndividualHuman::SetNewInfectionState(InfectionStateChange::_enum inf_state_change)
    {
        // Derived disease classes can report their own NewInfection states
        return false;
    }

    void IndividualHuman::ReportInfectionState()
    {
        m_new_infection_state = NewInfectionState::NewInfection;
    }

    void IndividualHuman::ClearNewInfectionState()
    {
        m_new_infection_state = NewInfectionState::Invalid;
    }

    //------------------------------------------------------------------
    //   Assorted getters and setters
    //------------------------------------------------------------------

    // IIndividualHumanContext methods
    const AgentParams* IndividualHuman::GetParams() const
    {
        return AgentConfig::GetAgentParams();
    }

    suids::suid IndividualHuman::GetSuid() const         { return suid; }
    suids::suid IndividualHuman::GetNextInfectionSuid()  { return parent->GetNextInfectionSuid(); }
    RANDOMBASE* IndividualHuman::GetRng()              { return parent->GetRng(); }

    IIndividualHumanInterventionsContext* IndividualHuman::GetInterventionsContext() const
    {
        return static_cast<IIndividualHumanInterventionsContext*>(interventions);
    }

    IVaccineConsumer* IndividualHuman::GetVaccineContext() const
    {
        return static_cast<IVaccineConsumer*>(interventions);
    }

    IIndividualHumanInterventionsContext* IndividualHuman::GetInterventionsContextbyInfection(IInfection* infection)
    {
        //Note can also throw exception here since it's not using the infection to find the intervention
        return static_cast<IIndividualHumanInterventionsContext*>(interventions); 
    }

    IIndividualHumanEventContext* IndividualHuman::GetEventContext()
    {
        return static_cast<IIndividualHumanEventContext*>(this);
    }

    ISusceptibilityContext* IndividualHuman::GetSusceptibilityContext() const
    {
        return static_cast<ISusceptibilityContext*>(susceptibility);
    }

    bool IndividualHuman::IsPossibleMother() const
    {
        if (m_age > (14 * DAYSPERYEAR) && m_age < (45 * DAYSPERYEAR) && m_gender == Gender::FEMALE)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    void IndividualHuman::Die( HumanStateChange newState )
    {
        StateChange = newState;
    }

    void IndividualHuman::BroadcastDeath()
    {
        IIndividualEventBroadcaster* broadcaster = nullptr;
        if( GetNodeEventContext() != nullptr )
        {
            broadcaster = GetNodeEventContext()->GetIndividualEventBroadcaster();
        }

        switch (StateChange)
        {
            case HumanStateChange::DiedFromNaturalCauses:
            {
                //LOG_DEBUG_F( "%s: individual %d (%s) died of natural causes at age %f with daily_mortality_rate = %f\n", __FUNCTION__, suid.data, (GetGender() == Gender::FEMALE ? "Female" : "Male"), GetAge() / DAYSPERYEAR, m_daily_mortality_rate );
                if( broadcaster )
                {
                    broadcaster->TriggerObservers( GetEventContext(), EventTrigger::NonDiseaseDeaths );
                }
            }
            break;

            case HumanStateChange::KilledByInfection:
            {
                LOG_DEBUG_F( "%s: individual %d died from infection\n", __FUNCTION__, suid.data );
                if( broadcaster )
                {
                    broadcaster->TriggerObservers( GetEventContext(), EventTrigger::DiseaseDeaths );
                }
            }
            break;

            case HumanStateChange::KilledByMCSampling:
            {
                LOG_DEBUG_F( "%s: individual %d will be removed as part of downsampling\n", __FUNCTION__, suid.data );
                if( broadcaster )
                {
                    broadcaster->TriggerObservers( GetEventContext(), EventTrigger::MonteCarloDeaths );
                }
            }
            break;

            default:
            {
                release_assert( false );
            }
            break;
        }
    }

    INodeEventContext * IndividualHuman::GetNodeEventContext()
    {
        release_assert( GetParent() );
        return GetParent()->GetEventContext();
    }

    IIndividualHumanContext* IndividualHuman::GetContextPointer() { return this; }

    INodeContext* IndividualHuman::GetParent() const
    {
        return parent;
    }

    suids::suid IndividualHuman::GetParentSuid() const
    {
        return parent->GetSuid();
    }


    IPKeyValueContainer* IndividualHuman::GetProperties()
    {
        return &Properties;
    }

    const infection_list_t& IndividualHuman::GetInfections() const
    {
        return infections;
    }

    bool  IndividualHuman::IsSymptomatic() const
    {
        for( auto &it : infections )
        {
            if( ( *it ).IsSymptomatic() ) return true;
        }
        return false;
    }

    bool  IndividualHuman::IsNewlySymptomatic() const
    {
        return m_newly_symptomatic;
    }


    ProbabilityNumber IndividualHuman::getProbMaternalTransmission() const
    {
        return parent->GetProbMaternalTransmission();
    }

    void serialize_waypoint_types( IArchive& ar, std::vector<MigrationType::Enum>& waypointTripTypes )
    {
        size_t count = ar.IsWriter() ? waypointTripTypes.size() : -1;

        ar.startArray(count);
        if (ar.IsWriter())
        {
            for (auto& entry : waypointTripTypes)
            {
                ar & (uint32_t&)entry;
            }
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                MigrationType::Enum value;
                ar & (uint32_t&)value;
                waypointTripTypes.push_back( value );
            }
        }
        ar.endArray();
    }

    void IndividualHuman::serialize(IArchive& ar, IndividualHuman* obj)
    {
        IndividualHuman& individual = *obj;
        ar.labelElement("suid") & individual.suid;
        ar.labelElement("m_age") & individual.m_age;
        ar.labelElement("m_gender") & individual.m_gender;
        ar.labelElement("m_mc_weight") & individual.m_mc_weight;
        ar.labelElement("m_daily_mortality_rate") & individual.m_daily_mortality_rate;
        ar.labelElement("is_pregnant") & individual.is_pregnant;
        ar.labelElement("pregnancy_timer") & individual.pregnancy_timer;
        ar.labelElement("susceptibility") & individual.susceptibility;
        ar.labelElement("infections") & individual.infections;
        ar.labelElement("interventions") & individual.interventions;
        // don't serialize transmissionGroupMembership, it will be reconstituted on deserialization
        // don't serialize transmissionGroupMembershipByRoute, it will be reconstituted on deserialization
        ar.labelElement("m_is_infected") & individual.m_is_infected;
        ar.labelElement("infectiousness") & individual.infectiousness;
        ar.labelElement("cumulativeInfs") & individual.cumulativeInfs;
        ar.labelElement("m_new_infection_state") & (uint32_t&)individual.m_new_infection_state;
        ar.labelElement("StateChange") & (uint32_t&)individual.StateChange;
        ar.labelElement("migration_type") & (uint32_t&)individual.migration_type;
        ar.labelElement("migration_destination") & individual.migration_destination;
        ar.labelElement("migration_time_until_trip") & individual.migration_time_until_trip;
        ar.labelElement("migration_time_at_destination") & individual.migration_time_at_destination;
        ar.labelElement("migration_is_destination_new_home") & individual.migration_is_destination_new_home;
        ar.labelElement("migration_will_return") & individual.migration_will_return;
        ar.labelElement("migration_outbound") & individual.migration_outbound;
        ar.labelElement("max_waypoints") & individual.max_waypoints;
        ar.labelElement("waypoints") & individual.waypoints;
        ar.labelElement("waypoints_trip_type"); serialize_waypoint_types( ar, individual.waypoints_trip_type );
        ar.labelElement("home_node_id") & individual.home_node_id;
        ar.labelElement("Properties") & individual.Properties;
        ar.labelElement("waiting_for_family_trip") & individual.waiting_for_family_trip;
        ar.labelElement("leave_on_family_trip") & individual.leave_on_family_trip;
        ar.labelElement("is_on_family_trip") & individual.is_on_family_trip;
        ar.labelElement("family_migration_type") & (uint32_t&)individual.family_migration_type;
        ar.labelElement("family_migration_time_until_trip") & individual.family_migration_time_until_trip;
        ar.labelElement("family_migration_time_at_destination") & individual.family_migration_time_at_destination;
        ar.labelElement("family_migration_is_destination_new_home") & individual.family_migration_is_destination_new_home;
        ar.labelElement("family_migration_destination") & individual.family_migration_destination;
        ar.labelElement("m_newly_symptomatic") & individual.m_newly_symptomatic;
    }

    REGISTER_SERIALIZABLE(IndividualHuman);
}
