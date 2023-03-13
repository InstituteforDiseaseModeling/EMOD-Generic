/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportRelationshipMigrationTracking.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "ISimulationContext.h"
#include "INodeSTI.h"
#include "NodeEventContext.h"
#include "IIndividualHuman.h"
#include "IIndividualHumanContext.h"
#include "VectorContexts.h"
#include "VectorPopulation.h"
#include "IMigrationInfo.h"
#include "IRelationship.h"
#include "IIndividualHumanSTI.h"
#include "IdmDateTime.h"
#include "INodeContext.h"
#include "IMigrate.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING( "ReportRelationshipMigrationTracking" )

static const char* _sim_types[] = { "STI_SIM", "HIV_SIM", nullptr };

Kernel::DllInterfaceHelper DLL_HELPER( _module, _sim_types );

//******************************************************************************
// DLL Methods
//******************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

DTK_DLLEXPORT char*
__cdecl GetEModuleVersion(char* sVer, const Environment* pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void
__cdecl GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char*
__cdecl GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT Kernel::IReport*
__cdecl GetReportInstantiator()
{
    return new Kernel::ReportRelationshipMigrationTracking();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- ReportRelationshipMigrationTracking Methods
// ----------------------------------------

namespace Kernel
{
    ReportRelationshipMigrationTracking::ReportRelationshipMigrationTracking()
        : BaseTextReportEvents( "ReportRelationshipMigrationTracking.csv" )
        , m_EndTime(0.0)
        , m_MigrationDataMap()
    {
        // ------------------------------------------------------------------------------------------------
        // --- Since this report will be listening for events, it needs to increment its reference count
        // --- so that it is 1.  Other objects will be AddRef'ing and Release'ing this report/observer
        // --- so it needs to start with a refcount of 1.
        // ------------------------------------------------------------------------------------------------
        AddRef();
    }

    ReportRelationshipMigrationTracking::~ReportRelationshipMigrationTracking()
    {
    }

    bool ReportRelationshipMigrationTracking::Configure( const Configuration * inputJson )
    {

        bool ret = BaseTextReportEvents::Configure( inputJson );

        // Manually push required events into the eventTriggerList
        //eventTriggerList.push_back( EventTrigger::Emigrating         );
        //eventTriggerList.push_back( EventTrigger::Immigrating        );
        eventTriggerList.push_back( EventTrigger::STIPreEmigrating   );
        eventTriggerList.push_back( EventTrigger::STIPostImmigrating );
        eventTriggerList.push_back( EventTrigger::NonDiseaseDeaths   );
        eventTriggerList.push_back( EventTrigger::DiseaseDeaths      );
        
        return ret;
    }

    void ReportRelationshipMigrationTracking::UpdateEventRegistration( float currentTime, 
                                                                       float dt, 
                                                                       std::vector<INodeEventContext*>& rNodeEventContextList,
                                                                       ISimulationEventContext* pSimEventContext )
    {
        BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList, pSimEventContext );
    }


    std::string ReportRelationshipMigrationTracking::GetHeader() const
    {
        std::stringstream header ;
        header << "Time"             << ", "
               << "IndividualID"     << ", "
               << "AgeYears"         << ", "
               << "Gender"           << ", "
               << "From_NodeID"      << ", "
               << "To_NodeID"        << ", "
               << "MigrationType"    << ", "
               << "Event"            << ","
               << "IsInfected"       << ", "
               << "Rel_ID"           << ", "
               << "NumCoitalActs"    << ", "
               << "IsDiscordant"     << ", "
               << "HasMigrated"      << ", "
               << "RelationshipType" << ", "
               << "RelationshipState"<< ", "
               << "PartnerID"        << ", "
               << "Male_NodeID"      << ", "
               << "Female_NodeID"
               ;

        return header.str();
    }

    bool ReportRelationshipMigrationTracking::notifyOnEvent( IIndividualHumanEventContext *context, 
                                                             const EventTrigger::Enum& trigger )
    {
        IIndividualHuman* p_ih = context->GetIndividual();
        release_assert(p_ih);

        IMigrate* im = p_ih->GetIMigrate();
        release_assert(im);

        IIndividualHumanSTI* p_hsti = p_ih->GetIndividualContext()->GetIndividualSTI();
        release_assert(p_hsti);

        INodeSTI* p_node_sti = context->GetNodeEventContext()->GetNodeContext()->GetNodeSTI();
        release_assert(p_node_sti);

        ISimulationContext* p_sim = context->GetNodeEventContext()->GetNodeContext()->GetParent();

        bool is_emigrating  = (trigger == EventTrigger::STIPreEmigrating);

        float time = context->GetNodeEventContext()->GetTime().time ;
        long individual_id = context->GetSuid().data ;
        float age_years = p_ih->GetAge() / DAYSPERYEAR ;
        char gender = (p_ih->GetGender() == 0) ? 'M' : 'F' ;
        uint32_t from_node_id = p_sim->GetNodeExternalID( context->GetNodeEventContext()->GetId() ) ;
        uint32_t to_node_id = 0;
        std::string mig_type_str = "N/A" ;
        if( is_emigrating )
        {
            to_node_id = p_sim->GetNodeExternalID( im->GetMigrationDestination() ) ;
            int mig_type = im->GetMigrationType();
            if( mig_type == MigrationType::LOCAL_MIGRATION )
                mig_type_str = "local" ;
            else if( mig_type == MigrationType::AIR_MIGRATION )
                mig_type_str = "air" ;
            else if( mig_type == MigrationType::REGIONAL_MIGRATION )
                mig_type_str = "regional" ;
            else if( mig_type == MigrationType::SEA_MIGRATION )
                mig_type_str = "sea" ;
            else
                release_assert( false );
        }

        bool is_infected = context->IsInfected();
        for( auto prel : p_hsti->GetRelationships() )
        {
            int rel_id = prel->GetSuid().data;
            int female_id = prel->GetFemalePartnerId().data;
            int male_id   = prel->GetMalePartnerId().data;
            int partner_id = 0;
            if( female_id == context->GetSuid().data )
            {
                partner_id = male_id;
            }
            else
            {
                partner_id = female_id;
            }
            unsigned int num_acts = prel->GetNumCoitalActs();
            bool is_discordant = prel->IsDiscordant();
            bool has_migrated = prel->HasMigrated();
            std::string rel_type_str  = p_node_sti->GetRelationshipName( prel->GetType() );
            std::string rel_state_str = p_node_sti->GetRelationshipStateName( prel->GetState() );
            unsigned int male_node_id = 0;
            if( prel->MalePartner() != nullptr )
            {
                male_node_id = prel->MalePartner()->GetNodeSuid().data;
            }
            unsigned int female_node_id = 0;
            if( prel->FemalePartner() != nullptr )
            {
                female_node_id = prel->FemalePartner()->GetNodeSuid().data;
            }

            GetOutputStream() << time
                       << "," << individual_id 
                       << "," << age_years 
                       << "," << gender 
                       << "," << from_node_id 
                       << "," << to_node_id 
                       << "," << mig_type_str 
                       << "," << EventTrigger::pairs::lookup_key( trigger )
                       << "," << is_infected 
                       << "," << rel_id 
                       << "," << num_acts 
                       << "," << is_discordant 
                       << "," << has_migrated 
                       << "," << rel_type_str 
                       << "," << rel_state_str 
                       << "," << partner_id 
                       << "," << male_node_id 
                       << "," << female_node_id 
                       << endl;
        }

        return true;
    }

    void ReportRelationshipMigrationTracking::LogIndividualData( IIndividualHuman* individual ) 
    {
    }

    bool ReportRelationshipMigrationTracking::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return false ;
    }

    void ReportRelationshipMigrationTracking::EndTimestep( float currentTime, float dt )
    {
        m_EndTime = currentTime ;
        BaseTextReportEvents::EndTimestep( currentTime, dt );
    }

    void ReportRelationshipMigrationTracking::Reduce()
    {
        BaseTextReportEvents::EndTimestep( m_EndTime, 1.0 );
    }
}
