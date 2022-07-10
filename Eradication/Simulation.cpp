/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Simulation.h"
#include "ConfigParams.h"

#include <iomanip>      // std::setprecision

#include "FileSystem.h"
#include "Debug.h"
#include "Report.h"
#include "BinnedReport.h"
#include "DemographicsReport.h"
#include "SpatialReport.h"
#include "PropertyReport.h"
#include "Exceptions.h"
#include "Instrumentation.h"
#include "Memory.h"
#include "IMigrationInfo.h"
#include "Node.h"
#include "NodeDemographics.h"
#include "RANDOM.h"
#include "SimulationEventContext.h"
#include "NodeInfo.h"
#include "ReportEventRecorder.h"
#include "ReportEventRecorderNode.h"
#include "ReportEventRecorderCoordinator.h"
#include "ReportSurveillanceEventRecorder.h"
#include "SQLReporter.h"
#include "Individual.h"
#include "Susceptibility.h"
#include "Infection.h"
#include "LoadBalanceScheme.h"
#include "EventTrigger.h"
#include "RandomNumberGeneratorFactory.h"
#include "DllLoader.h"
#include "MpiDataExchanger.h"
#include "IdmMpi.h"
#include "Properties.h"
#include "NodeProperties.h"
#include "SerializationParameters.h"
#include "PythonSupport.h"

#ifdef _DEBUG
#include "BinaryArchiveReader.h"
#include "BinaryArchiveWriter.h"
#endif

#include <chrono>
typedef std::chrono::high_resolution_clock _clock;

using namespace std;

SETUP_LOGGING( "Simulation" )


#define RUN_ALL_CUSTOM_REPORTS "RunAllCustomReports"
#define NO_CUSTOM_REPORTS "NoCustomReports"

namespace Kernel
{
    // Enable querying of interfaces from Simulation objects
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Simulation,Simulation)
    BEGIN_QUERY_INTERFACE_BODY(Simulation)
        HANDLE_INTERFACE(IGlobalContext)
        HANDLE_INTERFACE(ISimulation)
        HANDLE_INTERFACE(ISimulationContext)
        HANDLE_ISUPPORTS_VIA(ISimulationContext)
    END_QUERY_INTERFACE_BODY(Simulation)

    //------------------------------------------------------------------
    //   Initialization methods
    //------------------------------------------------------------------

    Simulation::Simulation()
        : serializationFlags( SerializationBitMask_t{}.set( SerializationFlags::Population )
                            | SerializationBitMask_t{}.set( SerializationFlags::Parameters ) )           
        , nodes()
        , nodeRankMap()
        , node_events_added()
        , node_events_to_be_processed()
        , node_event_context_list()
        , migratingIndividualQueues()
        , m_interventionFactoryObj(nullptr)
        , demographicsContext(nullptr)
        , infectionSuidGenerator(EnvPtr->MPI.Rank, EnvPtr->MPI.NumTasks)
        , nodeSuidGenerator(EnvPtr->MPI.Rank, EnvPtr->MPI.NumTasks)
        , rng(nullptr)
        , reports()
        , individual_data_reports()
        , reportClassCreator(nullptr)
        , binnedReportClassCreator(nullptr)
        , spatialReportClassCreator(nullptr)
        , propertiesReportClassCreator(nullptr)
        , demographicsReportClassCreator(nullptr)
        , eventReportClassCreator( nullptr )
        , nodeEventReportClassCreator( nullptr )
        , coordinatorEventReportClassCreator( nullptr )
        , surveillanceEventReportClassCreator( nullptr )
        , sqlReportCreator( nullptr )
        , event_coordinators()
        , campaign_events()
        , event_context_host(nullptr)
        , currentTime()
        , custom_reports_filename( RUN_ALL_CUSTOM_REPORTS )
        , demographics_factory(nullptr)
        , m_pRngFactory( new RandomNumberGeneratorFactory() )
        , new_node_observers()
        , node_ctxt_ptr_vec()
        , node_info_ptr_vec()
        , node_ctxt_info_dex()
        , node_pop_vec()
        , node_dist_mat()
    {
        LOG_DEBUG( "CTOR\n" );

        reportClassCreator                  = Report::CreateReport;
        binnedReportClassCreator            = BinnedReport::CreateReport;
        spatialReportClassCreator           = SpatialReport::CreateReport;
        propertiesReportClassCreator        = PropertyReport::CreateReport;
        demographicsReportClassCreator      = DemographicsReport::CreateReport;
        eventReportClassCreator             = ReportEventRecorder::CreateReport;
        nodeEventReportClassCreator         = ReportEventRecorderNode::CreateReport;
        coordinatorEventReportClassCreator  = ReportEventRecorderCoordinator::CreateReport;
        surveillanceEventReportClassCreator = ReportSurveillanceEventRecorder::CreateReport;
        sqlReportCreator                    = SQLReporter::CreateReport;

        nodeRankMap.SetNodeInfoFactory( this );

        // Initialize node demographics from file
        if( !JsonConfigurable::_dryrun )
        {
            demographics_factory = NodeDemographicsFactory::CreateNodeDemographicsFactory(EnvPtr->Config);
            if( demographics_factory == nullptr )
            {
                throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "Failed to create NodeDemographicsFactory" );
            }
            demographicsContext = demographics_factory->CreateDemographicsContext();

            ExternalNodeId_t first_node_id = demographics_factory->GetNodeIDs()[ 0 ];
            JsonObjectDemog json_for_first_node = demographics_factory->GetJsonForNode( first_node_id );
            IPFactory::GetInstance()->Initialize( first_node_id, json_for_first_node );

            NPFactory::GetInstance()->Initialize( demographics_factory->GetNodePropertiesJson() );
        }
    }

    Simulation::~Simulation()
    {
/* maybe #ifdef _DEBUG?
        LOG_DEBUG( "DTOR\n" );
        for (auto& entry : nodes)
        {
            delete entry.second;
        }
        nodes.clear();

        delete demographics_factory;
        demographics_factory = nullptr;

        if (rng) delete rng;

        delete event_context_host;
        event_context_host = nullptr;

        for (auto report : reports)
        {
            LOG_DEBUG_F( "About to delete report = %s\n", report->GetReportName().c_str() );
            delete report;
        }
        reports.clear();
*/        
    }

    bool Simulation::Configure( const Configuration * inputJson )
    {
        LOG_DEBUG("Configure\n");

        if( JsonConfigurable::_dryrun || EnvPtr->Config->Exist( "Custom_Reports_Filename" ) )
        {
            initConfigTypeMap( "Custom_Reports_Filename", &custom_reports_filename, Custom_Reports_Filename_DESC_TEXT, RUN_ALL_CUSTOM_REPORTS );
        }

        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret || JsonConfigurable::_dryrun )
        {
            m_pRngFactory->CreateFromSerializeData( SerializationParameters::GetInstance()->GetCreateRngFromSerializedData() );
            if( JsonConfigurable::_dryrun || !SerializationParameters::GetInstance()->GetCreateRngFromSerializedData() )
            {
                m_pRngFactory->Configure( inputJson );
            }
        }

        return ret;
    }

    // Check simulation abort conditions
    bool Simulation::TimeToStop()
    {
        bool abortSim = false;

        if(GetParams()->enable_termination_on_zero_total_infectivity && currentTime.time > GetParams()->sim_time_end_min)
        {
            // Accumulate node infectivity
            float totInfVal         = 0.0f;
            float totInfVal_unified = 0.0f;

            for (auto iterator = nodes.rbegin(); iterator != nodes.rend(); ++iterator)
            {
                INodeContext* n = iterator->second;
                totInfVal += n->GetInfectivity();
            }

            // Accumulate infectivity from all cores
            EnvPtr->MPI.p_idm_mpi->Allreduce_SUM(&totInfVal, &totInfVal_unified, 1);

            // Abort on zero infectivity
            if(totInfVal_unified == 0.0f)
            {
                LOG_INFO("Zero infectivity at current time-step; simulation aborting.\n");
                abortSim = true;
            }
        }

        if(GetParams()->enable_termination_on_total_wall_time && EnvPtr->Log)
        {
            LogTimeInfo tInfo;
            EnvPtr->Log->GetLogInfo(tInfo);
            float wall_time_minutes = tInfo.hours*60.0f + tInfo.mins + tInfo.secs/60.0f;

            if(wall_time_minutes > GetParams()->wall_time_max_minutes)
            {
                LOG_INFO("Total wall time duration at current time-step exceeds maximum; simulation aborting.\n");
                abortSim = true;
            }
        }

        return abortSim;
    }

    Simulation *Simulation::CreateSimulation()
    {
        Simulation *newsimulation = _new_ Simulation();
        newsimulation->Initialize();

        return newsimulation;
    }

    Simulation *Simulation::CreateSimulation(const ::Configuration *config)
    {
        Simulation *newsimulation = _new_ Simulation();

        if (newsimulation)
        {
            // This sequence is important: first
            // Creation-->Initialization-->Validation
            newsimulation->Initialize(config);
            if(!newsimulation->ValidateConfiguration(config))
            {
                delete newsimulation;
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "GENERIC_SIM requested with invalid configuration." );
            }
        }

        return newsimulation;
    }

    bool Simulation::ValidateConfiguration(const ::Configuration *config)
    {
        const ClimateParams*   cp = ClimateConfig::GetClimateParams();
        const MigrationParams* mp = MigrationConfig::GetMigrationParams();
        const NodeParams*      np = NodeConfig::GetNodeParams();
        const AgentParams*     ap = AgentConfig::GetAgentParams();
        const SimParams*       sp = SimConfig::GetSimParams();

        if( demographics_factory->GetEnableDemographicsBuiltin() && cp->climate_structure != ClimateStructure::CLIMATE_OFF 
                                                                 && cp->climate_structure != ClimateStructure::CLIMATE_CONSTANT )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Enable_Demographics_Builtin", demographics_factory->GetEnableDemographicsBuiltin(),
                                                                                      "Climate_Model", ClimateStructure::pairs::lookup_key(cp->climate_structure));
        }

        if( mp->enable_mig_family && !CanSupportFamilyTrips() )
        {
            std::stringstream msg;
            msg << "Invalid Configuration for Family Trips." << std::endl;
            msg << "Migration_Pattern must be SINGLE_ROUND_TRIPS and the 'XXX_Migration_Roundtrip_Probability' must equal 1.0 if that Migration Type is enabled." << std::endl;
            msg << "Migration_Pattern = " << MigrationPattern::pairs::lookup_key( mp->migration_pattern ) << std::endl;
            msg << "Enable_Local_Migration = "    << mp->enable_mig_local    << " and Local_Migration_Roundtrip_Probability = "    << mp->local_roundtrip_prob  << std::endl;
            msg << "Enable_Air_Migration = "      << mp->enable_mig_air      << " and Air_Migration_Roundtrip_Probability = "      << mp->air_roundtrip_prob    << std::endl;
            msg << "Enable_Regional_Migration = " << mp->enable_mig_regional << " and Regional_Migration_Roundtrip_Probability = " << mp->region_roundtrip_prob << std::endl;
            msg << "Enable_Sea_Migration = "      << mp->enable_mig_sea      << " and Sea_Migration_Roundtrip_Probability = "      << mp->sea_roundtrip_prob    << std::endl;
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        // Check for not-yet-implemented strain tracking features.
        if(ap->enable_strain_tracking && np->enable_infectivity_reservoir)
        {
            std::ostringstream msg;
            msg << "Enable_Strain_Tracking with Enable_Infectivity_Reservoir functionality not yet added.";
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        if( (sp->sim_type == SimType::STI_SIM || sp->sim_type == SimType::HIV_SIM) && 
            (np->ind_sampling_type != IndSamplingType::TRACK_ALL)      &&
            (np->ind_sampling_type != IndSamplingType::FIXED_SAMPLING) )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__,
                                                    "Individual_Sampling_Type", IndSamplingType::pairs::lookup_key(np->ind_sampling_type),
                                                    "Simulation_Type", SimType::pairs::lookup_key(sp->sim_type),
                                                    "Relationship-based transmission network only works with 100% sampling.");
        }

        if (IndividualHumanConfig::superinfection && (IndividualHumanConfig::max_ind_inf < 2))
        {
            throw IncoherentConfigurationException(__FILE__, __LINE__, __FUNCTION__, "Max_Individual_Infections", IndividualHumanConfig::max_ind_inf,
                                                                                     "Enable_Superinfection", IndividualHumanConfig::superinfection);
        }

        if( IndividualHumanConfig::enable_skipping && np->enable_hint )
        {
            throw IncoherentConfigurationException(__FILE__, __LINE__, __FUNCTION__, "Enable_Skipping", 1, "Enable_Heterogeneous_Intranode_Transmission", 1);
        }

        if(ap->enable_genome_mutation && ap->genome_mutation_rates.size() == 0)
        {
            throw IncoherentConfigurationException(__FILE__, __LINE__, __FUNCTION__, "Enable_Genome_Mutation", "1", "Genome_Mutation_Rates", "<empty>");
        }

        if(ap->enable_genome_dependent_infectivity && ap->genome_infectivity_multipliers.size() == 0)
        {
            throw IncoherentConfigurationException(__FILE__, __LINE__, __FUNCTION__, "Enable_Genome_Dependent_Infectivity", "1", "Genome_Infectivity_Multipliers", "<empty>");
        }

        if(ap->enable_label_mutator && ap->genome_mutations_labeled.size() == 0)
        {
            throw IncoherentConfigurationException(__FILE__, __LINE__, __FUNCTION__, "Enable_Label_By_Mutator", "1", "Genome_Mutations_Labeled", "<empty>");
        }

        if(sp->enable_interventions && sp->campaign_filename.empty())
        {
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "'Campaign_Filename' is empty.  You must have a file." );
        }

        if(sp->enable_property_output && !IPFactory::GetInstance()->HasIPs() )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "<Number of Individual Properties>", "0", "Enable_Property_Output", "1" );
        }

        for (auto report : reports)
        {
            report->Validate( this );
        }

        return true;
    }

    void Simulation::Initialize()
    {
        LOG_DEBUG( "Initialize\n" );
    }

    void Simulation::Initialize(const ::Configuration *config)
    {
        LOG_DEBUG( "Initialize\n" );

        Configure( config );

        IndividualHumanConfig   gen_individual_config_obj;
        SusceptibilityConfig    gen_susceptibility_config_obj;
        InfectionConfig         gen_infection_config_obj;
        MemoryGauge             mem_gauge_obj;

        gen_individual_config_obj.Configure( config );
        gen_susceptibility_config_obj.Configure( config );
        gen_infection_config_obj.Configure( config );
        mem_gauge_obj.Configure( config );

        m_interventionFactoryObj = InterventionFactory::getInstance();

        setupMigrationQueues();
        setupEventContextHost();
        setupRng();
        setParams(config);
        Reports_CreateBuiltIn();
        Reports_ConfigureBuiltIn();
        Reports_CreateCustom();
        Reports_FindReportsCollectingIndividualData();
    }

    INodeInfo* Simulation::CreateNodeInfo()
    {
        INodeInfo* pin = new NodeInfo();
        return pin ;
    }

    INodeInfo* Simulation::CreateNodeInfo( int rank, INodeContext* pNC )
    {
        INodeInfo* pin = new NodeInfo( rank, pNC );
        return pin ;
    }

    void Simulation::setupMigrationQueues()
    {
        migratingIndividualQueues.resize(EnvPtr->MPI.NumTasks); // elements are instances not pointers
    }

    void Simulation::setupEventContextHost()
    {
        event_context_host = _new_ SimulationEventContextHost(this);
    }

    void Simulation::setupRng()
    {
        RANDOMBASE* tmp_rng = m_pRngFactory->CreateRng();
        // don't change pointer in case created from serialization
        if( tmp_rng != nullptr )
        {
            rng = tmp_rng;
        }
    }

    void Simulation::setParams(const ::Configuration *config)
    {
        currentTime.time = GetParams()->sim_time_start;
        currentTime.setBaseYear(GetParams()->sim_time_base_year);
    }

    //------------------------------------------------------------------
    //   Reporting methods
    //------------------------------------------------------------------

    void Simulation::Reports_CreateBuiltIn()
    {
        // Check reporter_plugins directory and use report classes from there if any exists

        LOG_DEBUG( "Reports_CreateBuiltIn()\n" );

        if(GetParams()->enable_default_report)
        {
            // Default report
            IReport * report = (*reportClassCreator)();
            release_assert(report);
            reports.push_back(report);
        }

        if(GetParams()->enable_property_output)
        {
            IReport * prop_report = (*propertiesReportClassCreator)();
            release_assert(prop_report);
            reports.push_back(prop_report);
        }

        if(GetParams()->enable_spatial_output)
        {
            IReport * spatial_report = (*spatialReportClassCreator)();
            release_assert(spatial_report);
            reports.push_back(spatial_report);
        }

        if(GetParams()->enable_event_report)
        {
            IReport * event_report = (*eventReportClassCreator)();
            release_assert(event_report);
            reports.push_back(event_report);
        }

        if(GetParams()->enable_node_event_report)
        {
            IReport * node_event_report = (*nodeEventReportClassCreator)();
            release_assert( node_event_report );
            reports.push_back( node_event_report );
        }

        if(GetParams()->enable_coordinator_event_report)
        {
            IReport * coordinator_event_report = (*coordinatorEventReportClassCreator)();
            release_assert( coordinator_event_report );
            reports.push_back( coordinator_event_report );
        }

        if(GetParams()->enable_surveillance_event_report)
        {
            IReport * surveillance_event_report = (*surveillanceEventReportClassCreator)();
            release_assert( surveillance_event_report );
            reports.push_back( surveillance_event_report );
        }

        if(GetParams()->enable_event_db)
        {
            IReport * report = (*sqlReportCreator)();
            release_assert( report );
            reports.push_back( report );
        }

        if(GetParams()->enable_demographic_tracking)
        {
            IReport * binned_report = (*binnedReportClassCreator)();
            release_assert(binned_report);
            reports.push_back(binned_report);

            IReport* demo_report = (*demographicsReportClassCreator)();
            release_assert(demo_report);
            reports.push_back(demo_report);
        }
    }

    void Simulation::RegisterNewNodeObserver(void* id, Kernel::ISimulation::callback_t observer)
    {
        new_node_observers[id] = observer;
    }

    void Simulation::UnregisterNewNodeObserver(void* id)
    {
        if (new_node_observers.erase(id) == 0)
        {
            LOG_WARN_F("%s: Didn't find entry for id %08X in observer map.", __FUNCTION__, id);
        }
    }

    void Simulation::DistributeEventToOtherNodes( const EventTrigger::Enum& rEventTrigger, INodeQualifier* pQualifier )
    {
        release_assert( pQualifier );

        for( auto entry : nodeRankMap.GetRankMap() )
        {
            INodeInfo* pni = entry.second;
            if( pQualifier->Qualifies( *pni ) )
            {
                // -------------------------------------------------------------------------------------
                // --- One could use a map to keep a node from getting more than one event per timestep
                // --- but I think the logic for that belongs in the object processing the event
                // -------------------------------------------------------------------------------------
                node_events_added[ nodeRankMap.GetRankFromNodeSuid( pni->GetSuid() ) ].Add( pni->GetSuid(), rEventTrigger ) ;
            }
        }
    }


    void Simulation::UpdateNodeEvents()
    {
        WithSelfFunc to_self_func = [this](int myRank) 
        { 
            //do nothing
        }; 

        // -------------------------------------------------------------
        // --- Send Event Triggers destined for nodes not on this processor
        // -------------------------------------------------------------
        SendToOthersFunc to_others_func = [this](IArchive* writer, int toRank)
        {
            *writer & node_events_added[toRank];
        };

        // -----------------------------------------------------------------
        // --- Receive the Event Triggers destined for the nodes on this processor
        // -----------------------------------------------------------------
        ReceiveFromOthersFunc from_others_func = [this](IArchive* reader, int fromRank)
        {
            EventsForOtherNodes efon ;
            *reader & efon;
            node_events_added[ EnvPtr->MPI.Rank ].Update( efon );
        };

        ClearDataFunc clear_data_func = [this](int rank)
        {
        };

        MpiDataExchanger exchanger( "EventsForOtherNodes", to_self_func, to_others_func, from_others_func, clear_data_func );
        exchanger.ExchangeData(currentTime.time);

        // ---------------------------------------------------
        // --- Update the events to be processed during the 
        // --- next time step for the nodes on this processor
        // ---------------------------------------------------
        node_events_to_be_processed.clear();
        for( auto entry : node_events_added[ EnvPtr->MPI.Rank ].GetMap() )
        {
            suids::suid node_id = entry.first;
            auto& trigger_list = entry.second;
            for( auto trigger : trigger_list )
            {
                node_events_to_be_processed[ node_id ].push_back( trigger );
            }
        }
        node_events_added.clear();

    }

    ISimulationEventContext* Simulation::GetSimulationEventContext()
    {
        return event_context_host;
    }

    void Simulation::Reports_ConfigureBuiltIn()
    {
        for( auto report : reports )
        {
            report->Configure( EnvPtr->Config );
        }
    }

    void Simulation::Reports_CreateCustom()
    {
        // -------------------------------------------------------------
        // --- Allow the user to indicate that they do not want to use
        // --- any custom reports even if DLL's are present.
        // -------------------------------------------------------------
        if( custom_reports_filename.empty() || (custom_reports_filename == NO_CUSTOM_REPORTS) )
        {
            return ;
        }

        ReportInstantiatorMap report_instantiator_map ;
        DllLoader dllLoader(SimType::pairs::lookup_key(GetParams()->sim_type));
        if( !dllLoader.LoadReportDlls( report_instantiator_map ) )
        {
            LOG_WARN_F("Failed to load reporter emodules for SimType: %s from path: %s\n" , SimType::pairs::lookup_key(GetParams()->sim_type), dllLoader.GetEModulePath(REPORTER_EMODULES).c_str());
        }
        Reports_Instantiate( report_instantiator_map );
    }

    void Simulation::Reports_FindReportsCollectingIndividualData()
    {
        // ---------------------------------------------------------------------
        // --- Get the subset of reports that are collecting individual data
        // --- This allows us to avoid calling LogIndividualData() for reports
        // --- that just have no-ops on every individual.
        // ---------------------------------------------------------------------
        individual_data_reports.clear();
        for( auto report : reports )
        {
            if( report->IsCollectingIndividualData( currentTime.time, GetParams()->sim_time_delta ) )
            {
                individual_data_reports.push_back( report );
            }
        }
    }

    Configuration* Simulation::Reports_GetCustomReportConfiguration()
    {
        Configuration* p_cr_config = nullptr ;

        // ------------------------------------------------------------------------------
        // --- If the user does not define the custom_reports_filename input parameter,
        // --- then they want to run all reports.  Returning null will do this.
        // ------------------------------------------------------------------------------
        if( !custom_reports_filename.empty() && (custom_reports_filename != RUN_ALL_CUSTOM_REPORTS) )
        {
            LOG_INFO_F("Looking for custom reports file = %s\n", custom_reports_filename.c_str());
            if( FileSystem::FileExists( custom_reports_filename ) )
            {
                LOG_INFO_F("Found custom reports file = %s\n", custom_reports_filename.c_str());
                // it is extremely unlikely that this will return null.  It will throw an exception if an error occurs.
                Configuration* p_config = Configuration::Load( custom_reports_filename );
                if( !p_config ) 
                {
                    throw Kernel::InitializationException( __FILE__, __LINE__, __FUNCTION__, custom_reports_filename.c_str() );
                }
                p_cr_config = Configuration::CopyFromElement( (*p_config)["Custom_Reports"], p_config->GetDataLocation() );
                delete p_config ;
            }
            else
            {
                throw Kernel::FileNotFoundException(__FILE__, __LINE__, __FUNCTION__, custom_reports_filename.c_str());
            }
        }

        return p_cr_config ;
    }

    void Simulation::Reports_Instantiate( ReportInstantiatorMap& rReportInstantiatorMap )
    {
        auto cachedValue = JsonConfigurable::_useDefaults;
        JsonConfigurable::_useDefaults = true;
        Configuration* p_cr_config = Reports_GetCustomReportConfiguration();

        bool load_all_reports = (p_cr_config == nullptr) ||
                                !p_cr_config->Exist( "Use_Explicit_Dlls" ) ||
                                (int(p_cr_config->operator[]( "Use_Explicit_Dlls" ).As<json::Number>()) != 1) ;

        LOG_INFO_F("Found %d Custom Report DLL's to consider loading, load_all_reports=%d\n", rReportInstantiatorMap.size(), load_all_reports );

        // Verify that a DLL exists for each report defined in the custom reports file
        if( p_cr_config )
        {
            auto custom_reports_config = p_cr_config->As<json::Object>();
            for( auto it = custom_reports_config.Begin(); it != custom_reports_config.End(); ++it )
            {
                std::string reportname(it->name);
                if( reportname != "Use_Explicit_Dlls" &&  rReportInstantiatorMap.find( reportname ) == rReportInstantiatorMap.end() )
                {
                    //check if report is enabled
                    json::QuickInterpreter dll_data = p_cr_config->operator[]( reportname ).As<json::Object>();
                    if( int( dll_data["Enabled"].As<json::Number>() ) != 0 )
                    {
                        //Dll not found
                        std::stringstream ss;
                        ss << reportname << " (dll)";
                        throw Kernel::FileNotFoundException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
                    }                    
                }
            }
        }

        for( auto ri_entry : rReportInstantiatorMap )
        {
            std::string class_name = ri_entry.first ;
            try
            {
                if( (p_cr_config != nullptr) && p_cr_config->Exist( class_name ) )
                {
                    LOG_INFO_F("Found custom report data for %s\n", class_name.c_str());
                    json::QuickInterpreter dll_data = p_cr_config->operator[]( class_name ).As<json::Object>() ;
                    if( int(dll_data["Enabled"].As<json::Number>()) != 0 )
                    {
                        json::Array report_data = dll_data["Reports"].As<json::Array>() ;
                        for( int i = 0 ; i < report_data.Size() ; i++ )
                        {
                            LOG_INFO_F( "Created instance #%d of %s\n", (i+1),class_name.c_str() );
                            Configuration* p_cfg = Configuration::CopyFromElement( report_data[i], p_cr_config->GetDataLocation() );

                            IReport* p_cr = ri_entry.second(); // creates report object
                            p_cr->Configure( p_cfg );
                            reports.push_back( p_cr );
                            delete p_cfg ;
                            p_cfg = nullptr;
                        }
                    }
                }
                else if( load_all_reports )
                {
                    LOG_WARN_F("Did not find report configuration for report DLL %s.  Creating report with defaults.\n", class_name.c_str());

                    json::Object empty_json_obj ;
                    Configuration* p_cfg = Configuration::CopyFromElement( empty_json_obj, "no file" );

                    IReport* p_cr = ri_entry.second();  // creates report object
                    p_cr->Configure( p_cfg );
                    reports.push_back( p_cr );
                    delete p_cfg ;
                    p_cfg = nullptr;
                }
            }
            catch( json::Exception& e )
            {
                std::stringstream ss ;
                ss << "Error occured reading report data for " << class_name << ".  Error: " << e.what() << std::endl ;
                throw InitializationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
        }
        delete p_cr_config;
        p_cr_config = nullptr;
        JsonConfigurable::_useDefaults = cachedValue;
    }

    void Simulation::Reports_UpdateEventRegistration()
    {
        for (auto report : reports)
        {
            report->UpdateEventRegistration( currentTime.time,GetParams()->sim_time_delta, node_event_context_list, event_context_host );
        }
    }

    void Simulation::Reports_BeginTimestep()
    {
        for (auto report : reports)
        {
            release_assert(report);
            report->BeginTimestep();
        }
    }

    void Simulation::Reports_EndTimestep()
    {
        for (auto report : reports)
        {
            release_assert(report);
            report->EndTimestep( currentTime.time, GetParams()->sim_time_delta );
        }
    }

    void Simulation::Reports_LogNodeData( INodeContext* n )
    {
        for (auto report : reports)
        {
            report->LogNodeData( n );
        }
    }

    void Simulation::PrintTimeAndPopulation()
    {
        // print out infections and population out
        int stat_pop = 0, infected = 0;
        for (auto& entry : nodes)
        {
            INodeContext* n = entry.second;
            stat_pop += n->GetStatPop();
            infected += n->GetInfected();
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << "Update(): Time: " << float(currentTime.time);
        if( GetParams()->sim_time_base_year > 0.0f )
        {
            oss << std::fixed << " Year: " << currentTime.Year();
        }
        oss << std::fixed << " Rank: " << EnvPtr->MPI.Rank << " StatPop: " << stat_pop << " Infected: " << infected << std::endl;
        LOG_INFO( oss.str().c_str() );
    }

    void Simulation::WriteReportsData()
    {
        for (auto report : reports)
        {
            report->Reduce();

            // the rest only make sense on rank 0
            if (EnvPtr->MPI.Rank == 0)
            {
                LOG_INFO_F( "Finalizing '%s' reporter.\n", report->GetReportName().c_str() );
                report->Finalize();
                LOG_INFO_F( "Finalized  '%s' reporter.\n", report->GetReportName().c_str() );
            }
        }
    }

    //------------------------------------------------------------------
    //   Every timestep Update() method
    //------------------------------------------------------------------

    void Simulation::Update()
    {
        Reports_UpdateEventRegistration();
        Reports_FindReportsCollectingIndividualData();

        // -----------------
        // --- Update Events
        // -----------------
        release_assert(event_context_host);
        event_context_host->Update(GetParams()->sim_time_delta);

        Reports_BeginTimestep();

        // -----------------------------------
        // --- Network Infectivity - Calculate
        // -----------------------------------
        float total_sim_pop = FLT_MIN;
        float max_exp_frac  = GetParams()->net_infect_max_frac;
        float min_inf_val   = 1.0e-8f;

        if(GetParams()->enable_net_infect)
        {
            int   k1, k2;
            float inf_mult, inf_cum;

            // Get current populations for nodes on this rank
            for(k1 = 0; k1 < node_ctxt_ptr_vec.size(); k1++)
            {
                node_pop_vec[k1] = node_ctxt_ptr_vec[k1]->GetStatPop();
            }

            // Get current total population for nodes on all ranks
            for(k2 = 0; k2 < node_info_ptr_vec.size(); k2++)
            {
                total_sim_pop += node_info_ptr_vec[k2]->GetPopulation();
            }

            // Calculate network infectivity (push fraction) for nodes on this rank
            for(k1 = 0; k1 < node_ctxt_ptr_vec.size(); k1++)
            {
                inf_cum = 0.0f;

                for(k2 = 0; k2 < node_info_ptr_vec.size(); k2++)
                {
                    if(k2 == node_ctxt_info_dex[k1])
                    {
                        continue;
                    }

                    inf_mult = node_dist_mat[node_ctxt_info_dex[k1]][k2] * node_info_ptr_vec[k2]->GetPopulation() / total_sim_pop;

                    if(inf_mult < min_inf_val)
                    {
                        continue;
                    }

                    inf_cum += inf_mult;
                }

                // Update push fraction within node
                node_ctxt_ptr_vec[k1]->SetNetInfectFrac(inf_cum);
            }
        }

        // -----------------
        // --- Update Nodes
        // -----------------
        for (auto iterator = nodes.rbegin(); iterator != nodes.rend(); ++iterator)
        {
            INodeContext* n = iterator->second;
            release_assert(n);
            n->AddEventsFromOtherNodes(node_events_to_be_processed[n->GetSuid()]);
            n->Update(GetParams()->sim_time_delta);

            Reports_LogNodeData(n);
        }

        // -----------------------
        // --- Resolve Migration
        // -----------------------
        REPORT_TIME( ENABLE_DEBUG_MPI_TIMING, "resolveMigration", resolveMigration() );

        for (auto iterator = nodes.rbegin(); iterator != nodes.rend(); ++iterator)
        {
            INodeContext* n = iterator->second;
            nodeRankMap.Update(n);
        }
        nodeRankMap.Sync(currentTime.time);

        UpdateNodeEvents();

        // ---------------------------------
        // --- Network Infectivity - Deposit
        // ---------------------------------
        if(GetParams()->enable_net_infect)
        {
            int   k1, k2, k3;
            float cur_inf_val, inf_mult, n_exp_frac, node_exp_mult;

            // Calculate network infectivity (pull fraction)
            for(k1 = 0; k1 < node_ctxt_ptr_vec.size(); k1++)
            {
                for(k2 = 0; k2 < node_info_ptr_vec.size(); k2++)
                {
                    if(k2 == node_ctxt_info_dex[k1])
                    {
                        continue;
                    }

                    if(node_info_ptr_vec[k2]->GetNetInfRep()->size() == 0)
                    {
                        continue;
                    }

                    n_exp_frac    = node_info_ptr_vec[k2]->GetNetInfectFrac();
                    node_exp_mult = (n_exp_frac < max_exp_frac) ? 1.0f : (max_exp_frac/n_exp_frac);

                    inf_mult = node_dist_mat[node_ctxt_info_dex[k1]][k2] * node_exp_mult * node_pop_vec[k1] / total_sim_pop;

                    if(inf_mult < min_inf_val)
                    {
                        continue;
                    }

                    for(k3 = 0; k3 < node_info_ptr_vec[k2]->GetNetInfRep()->size(); k3++)
                    {
                        cur_inf_val = node_info_ptr_vec[k2]->GetNetInfRep()->GetInf(k3) * inf_mult;
                        node_ctxt_ptr_vec[k1]->DepositNetInf(node_info_ptr_vec[k2]->GetNetInfRep()->GetId(k3), cur_inf_val);
                    }
                }
            }
        }

        // -------------------
        // --- Increment Time
        // -------------------
        float time_for_python = currentTime.time;
        currentTime.Update(GetParams()->sim_time_delta);

        // ----------------------------------------------------------
        // --- Output Information for the end of the update/timestep
        // ----------------------------------------------------------
        PrintTimeAndPopulation();

        Reports_EndTimestep();

        // Everyone waits until report writing (if any) is done
        EnvPtr->MPI.p_idm_mpi->Barrier();

        // ---------------------
        // --- Python In-Process
        // ---------------------
        // Call out to embedded python in-processing script
        std::string py_input_string, new_campaign_filename; 
        py_input_string       = std::to_string(time_for_python);
        new_campaign_filename = Kernel::PythonSupport::RunPyFunction( py_input_string, Kernel::PythonSupport::SCRIPT_IN_PROCESS );

        // Ensure all processes have the same new_campaign_filename
        int fname_tmp_size = new_campaign_filename.size() + 1;
        EnvPtr->MPI.p_idm_mpi->BroadcastInteger(&fname_tmp_size, 1, 0);
        char* fname_tmp = static_cast<char*>(malloc(fname_tmp_size*sizeof(char)));
        std::strcpy(fname_tmp, new_campaign_filename.c_str());
        EnvPtr->MPI.p_idm_mpi->BroadcastChar(fname_tmp, fname_tmp_size, 0);
        new_campaign_filename = fname_tmp;
        free(fname_tmp); fname_tmp = nullptr;

        // In-process call returns input string if no python, empty string if no new campaign;
        if( new_campaign_filename != py_input_string && !new_campaign_filename.empty() )
        {
            const vector<ExternalNodeId_t>& nodeIDs = demographics_factory->GetNodeIDs();
            loadCampaignFromFile(new_campaign_filename, nodeIDs);
        }

        // ----------------
        // --- Memory Check
        // ----------------
        MemoryGauge::CheckMemoryFailure( false );

        return;
    }

    //------------------------------------------------------------------
    //   First timestep Populate() methods
    //------------------------------------------------------------------

    bool Simulation::Populate()
    {
        LOG_DEBUG("Calling populateFromDemographics()\n");

        // Populate nodes
        LOG_INFO_F("Campaign file name identified as: %s\n", (GetParams()->campaign_filename).c_str());
        int node_count = populateFromDemographics();
        LOG_INFO_F("populateFromDemographics() generated %d nodes.\n", node_count);

        LOG_INFO_F("Rank %d contributes %d nodes...\n", EnvPtr->MPI.Rank, nodeRankMap.Size());
        EnvPtr->Log->Flush();
        LOG_INFO_F("Merging node rank maps...\n");
        nodeRankMap.MergeMaps(); // merge rank maps across all processors
        LOG_INFO_F("Merged rank %d map now has %d nodes.\n", EnvPtr->MPI.Rank, nodeRankMap.Size());

        // Initialize migration structure from file
        IMigrationInfoFactory* migration_factory = ConstructMigrationInfoFactory( demographics_factory->GetIdReference(),
                                                                                  GetParams()->sim_type,
                                                                                  demographics_factory->GetEnableDemographicsBuiltin(),
                                                                                  demographics_factory->GetTorusSize() );
        release_assert(migration_factory);

        for (auto& entry : nodes)
        {
            release_assert(entry.second);
            (entry.second)->SetupMigration( migration_factory );
        } 

        delete migration_factory;
        migration_factory = nullptr;

//        if (nodeRankMap.Size() < 500)
//            LOG_INFO_F("Rank %d map contents:\n%s\n", EnvPtr->MPI.Rank, nodeRankMap.ToString().c_str());
//        else 
//            LOG_INFO("(Rank map contents not displayed due to large (> 500) number of entries.)\n");
        LOG_INFO("Rank map contents not displayed until NodeRankMap::ToString() (re)implemented.\n");

        // Inter-node distance factors for network infectivity
        if(GetParams()->enable_net_infect)
        {
            if(GetParams()->net_infect_grav_coeff.size() != GetParams()->net_infect_grav_dpow.size())
            {
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Different number of net infect coefficeints and exponents." );
            }

            LOG_INFO("Initializing values for network infectivity calculations\n");
            // Cache copy of node/nodeinfo pointers to avoid repeated map indexing
            int k1, k2, k3;
            float dist_fac = 0.0f;
            float dist_min = GetParams()->net_infect_min_dist;
            node_info_ptr_vec.resize(nodeRankMap.Size(), nullptr);
            node_ctxt_ptr_vec.resize(nodes.size(),       nullptr);
            node_ctxt_info_dex.resize(nodes.size(),           -1);

            k2 = 0;
            for(auto node_info_obj : nodeRankMap.GetRankMap())
            {
                node_info_ptr_vec[k2] = node_info_obj.second;
                k2++;
            }

            k1 = 0;
            for(auto node_obj : nodes)
            {
                node_ctxt_ptr_vec[k1] = node_obj.second;
                for(k2 = 0; k2 < node_info_ptr_vec.size(); k2++)
                {
                    if(node_obj.first == node_info_ptr_vec[k2]->GetSuid())
                    {
                        node_ctxt_info_dex[k1] = k2;
                        break;
                    }
                }
                release_assert(node_ctxt_info_dex[k1] >= 0);
                k1++;
            }

            node_pop_vec.resize(node_ctxt_ptr_vec.size());
            node_dist_mat.resize(node_info_ptr_vec.size());
            for(k1 = 0; k1 < node_info_ptr_vec.size(); k1++)
            {
                node_dist_mat[k1].resize(node_info_ptr_vec.size(),0.0f);
                for(k2 = 0; k2 < node_info_ptr_vec.size(); k2++)
                {
                    dist_fac  = CalculateDistanceKm(node_info_ptr_vec[k1]->GetLongitudeDegrees(),
                                                    node_info_ptr_vec[k1]->GetLatitudeDegrees(),
                                                    node_info_ptr_vec[k2]->GetLongitudeDegrees(),
                                                    node_info_ptr_vec[k2]->GetLatitudeDegrees());
                    dist_fac  = (dist_fac > dist_min ? dist_fac : dist_min);
                    for(k3 = 0; k3 < GetParams()->net_infect_grav_coeff.size(); k3++)
                    {
                        node_dist_mat[k1][k2] += GetParams()->net_infect_grav_coeff[k3] / 
                                                 std::pow(dist_fac,GetParams()->net_infect_grav_dpow[k3]);
                    }
                }
            }
        }

        // We'd like to be able to run even if a processor has no nodes, but there are other issues.
        // So for now just bail...
        if(node_count <= 0)
        {
            LOG_WARN_F("Rank %d wasn't assigned any nodes! (# of procs is too big for simulation?)\n", EnvPtr->MPI.Rank);
            return false;
        }

        for (auto report : reports)
        {
            LOG_DEBUG( "Initializing report...\n" );
            report->Initialize( nodeRankMap.Size());
            report->CheckForValidNodeIDs(demographics_factory->GetNodeIDs());
            LOG_INFO_F( "Initialized '%s' reporter\n", report->GetReportName().c_str() );
        }

        return true;
    }

    void Simulation::LoadInterventions(const std::vector<ExternalNodeId_t>& demographic_node_ids)
    {
        // Set up campaign interventions from file
        if( GetParams()->enable_interventions )
        {
            LOG_INFO_F( "Looking for campaign file %s\n", (GetParams()->campaign_filename).c_str() );

            if ( !FileSystem::FileExists( GetParams()->campaign_filename ) )
            {
                throw FileNotFoundException( __FILE__, __LINE__, __FUNCTION__, (GetParams()->campaign_filename).c_str() );
            }
            else 
            {
                LOG_INFO("Found campaign file successfully.\n");
            }

#ifdef WIN32
            DllLoader dllLoader;
            if (!dllLoader.LoadInterventionDlls())
            {
                LOG_WARN_F("Failed to load intervention emodules for SimType: %s from path: %s\n", SimType::pairs::lookup_key(GetParams()->sim_type), dllLoader.GetEModulePath(INTERVENTION_EMODULES).c_str());
            }
#endif

            JsonConfigurable::_track_missing = false;

            loadCampaignFromFile(GetParams()->campaign_filename, demographic_node_ids);

            JsonConfigurable::_track_missing = true;

            // ------------------------------------------
            // --- Setup Individual Property Transitions
            // ------------------------------------------
            if( IPFactory::GetInstance()->HasIPs() )
            {
                std::string transitions_file_path = FileSystem::Concat( Environment::getInstance()->OutputPath, std::string( IPFactory::transitions_dot_json_filename ) );

                if (EnvPtr->MPI.Rank == 0)
                {
                    // Delete any existing transitions.json file
                    LOG_DEBUG_F( "Deleting any existing %s file.\n", transitions_file_path.c_str() );
                    FileSystem::RemoveFile( transitions_file_path );

                    // Write the new transitions.json file
                    IPFactory::GetInstance()->WriteTransitionsFile();
                }

                // Everyone waits until the Rank=0 process is done deleting and creating the transitions file
                EnvPtr->MPI.p_idm_mpi->Barrier();

                if ( !FileSystem::FileExists( transitions_file_path ) )
                {
                    throw FileNotFoundException( __FILE__, __LINE__, __FUNCTION__, transitions_file_path.c_str() );
                }

                // Load the Individual Property Transitions
                JsonConfigurable::_track_missing = false;

                loadCampaignFromFile( transitions_file_path.c_str(), demographic_node_ids);

                JsonConfigurable::_track_missing = true;
            }
        }
    }

    int Simulation::populateFromDemographics()
    {
        string idreference  = demographics_factory->GetIdReference();
        const vector<ExternalNodeId_t>& nodeIDs = demographics_factory->GetNodeIDs();
        ClimateFactory * climate_factory = nullptr;
#ifndef DISABLE_CLIMATE
        // Initialize climate from file
        climate_factory = ClimateFactory::CreateClimateFactory(idreference, this);
        if (climate_factory == nullptr)
        {
            throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "Failed to create ClimateFactory" );
        }
#endif

        m_pRngFactory->SetNodeIds( nodeIDs );

        // Initialize load-balancing scheme from file
        IInitialLoadBalanceScheme* p_lbs = LoadBalanceSchemeFactory::Create(Environment::FindFileOnPath(GetParams()->loadbalance_filename).c_str(), nodeIDs.size(), EnvPtr->MPI.NumTasks );
        nodeRankMap.SetInitialLoadBalanceScheme( p_lbs );

        // Delete any existing transitions.json file
        // Anyone could delete the file, but we’ll delegate to rank 0
        if (EnvPtr->MPI.Rank == 0)
        {
            std::string transitions_file_path = FileSystem::Concat( Environment::getInstance()->OutputPath, std::string( IPFactory::transitions_dot_json_filename ) );
            LOG_DEBUG_F( "Deleting any existing %s file.\n", transitions_file_path.c_str() );
            FileSystem::RemoveFile( transitions_file_path );
        }
        EnvPtr->MPI.p_idm_mpi->Barrier();

        if (nodes.size() == 0)   // "Standard" initialization path
        {
            // Add nodes according to demographics-and climate file specifications
            uint32_t node_index = 0;
            for (auto external_node_id : nodeIDs)
            {
                if (getInitialRankFromNodeId( external_node_id ) == EnvPtr->MPI.Rank) // inclusion criteria to be added to this processor's shared memory space
                {
                    suids::suid node_suid;
                    node_suid.data = node_index + 1;
                    LOG_DEBUG_F( "Creating/adding new node: external_node_id = %lu, node_suid = %lu\n", external_node_id, node_suid.data );
                    addNewNodeFromDemographics( external_node_id, node_suid, demographics_factory, climate_factory );
                }
                ++node_index;
            }
        }
        else    // We already have nodes... must have loaded a serialized population.
        {
            for (auto& entry : nodes)
            {
                auto node = entry.second;
                node->SetContextTo(this);
                initializeNode( node, demographics_factory, climate_factory );
            }
        }

        if( GetParams()->enable_property_output && !IPFactory::GetInstance()->HasIPs() )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "<Number of Individual Properties>", "0", "Enable_Property_Output", "1" );
        }

        LoadInterventions(nodeIDs);

#ifndef DISABLE_CLIMATE
        // Clean up
        delete climate_factory; 
        climate_factory = nullptr;
#endif

        LOG_INFO_F( "populateFromDemographics() created %d nodes\n", nodes.size() );
        return int(nodes.size());
    }

    void Kernel::Simulation::addNewNodeFromDemographics( ExternalNodeId_t externalNodeId,
                                                         suids::suid node_suid, 
                                                         NodeDemographicsFactory *nodedemographics_factory, 
                                                         ClimateFactory *climate_factory )
    {
        Node *node = Node::CreateNode(this, externalNodeId, node_suid);
        addNode_internal( node, nodedemographics_factory, climate_factory );
    }

    void Kernel::Simulation::addNode_internal( INodeContext *node,
                                               NodeDemographicsFactory *nodedemographics_factory,
                                               ClimateFactory *climate_factory )
    {

        release_assert(node);
        release_assert(nodedemographics_factory);
#ifndef DISABLE_CLIMATE
        release_assert(climate_factory);
#endif

        node->SetRng( m_pRngFactory->CreateRng( node->GetExternalID() ) );

        // Node initialization 
        node->SetParameters( nodedemographics_factory, climate_factory );

        // Populate node
        node->PopulateFromDemographics();

        // Add node to the map
        nodes.insert( std::pair<suids::suid, INodeContext*>(node->GetSuid(), node) );
        node_event_context_list.push_back( node->GetEventContext() );
        nodeRankMap.Add( EnvPtr->MPI.Rank, node );

        notifyNewNodeObservers(node);
    }

    void Kernel::Simulation::initializeNode( INodeContext* node, 
                                             NodeDemographicsFactory* nodedemographics_factory, 
                                             ClimateFactory* climate_factory )
    {
        release_assert( node );
        release_assert( nodedemographics_factory );
#ifndef DISABLE_CLIMATE
        release_assert( climate_factory );
#endif

        node->SetParameters( nodedemographics_factory, climate_factory );
        node->InitializeTransmissionGroupPopulations();

        node_event_context_list.push_back( node->GetEventContext() );
        nodeRankMap.Add( EnvPtr->MPI.Rank, node );

        notifyNewNodeObservers( node );
    }

    void Simulation::loadCampaignFromFile( const std::string& campaignfilename, const std::vector<ExternalNodeId_t>& demographic_node_ids)
    {
        // load in the configuration
        // parse the DM creation events, create them, and add them to an event queue
        event_context_host->LoadCampaignFromFile( campaignfilename, demographic_node_ids);
    }

    void Simulation::notifyNewNodeObservers(INodeContext* node)
    {
        if (new_node_observers.size() > 0)
        {
            for (const auto& entry : new_node_observers)
            {
                entry.second(node);
            }
        }
    }

    //------------------------------------------------------------------
    //   Individual migration methods
    //------------------------------------------------------------------

    void Simulation::resolveMigration()
    {
        LOG_DEBUG("resolveMigration\n");

        WithSelfFunc to_self_func = [this](int myRank) 
        { 
#ifndef _DEBUG
            // Don't bother to serialize locally
            // for (auto individual : migratingIndividualQueues[destination_rank]) // Note the direction of iteration below!
            for (auto iterator = migratingIndividualQueues[myRank].rbegin(); iterator != migratingIndividualQueues[myRank].rend(); ++iterator)
            {
                auto individual = *iterator;
                IMigrate* emigre = individual->GetIMigrate();
                emigre->ImmigrateTo( nodes[emigre->GetMigrationDestination()] );
                if( individual->IsDead() )
                {
                    // we want the individual to migrate home and then finish dieing
                    delete individual;
                }
            }
#else
            if ( migratingIndividualQueues[myRank].size() > 0 )
            {
                auto writer = make_shared<BinaryArchiveWriter>();
                (*static_cast<IArchive*>(writer.get())) & migratingIndividualQueues[myRank];

                for (auto& individual : migratingIndividualQueues[myRank])
                    delete individual; // individual->Recycle();

                migratingIndividualQueues[myRank].clear();

                auto reader = make_shared<BinaryArchiveReader>(static_cast<IArchive*>(writer.get())->GetBuffer(), static_cast<IArchive*>(writer.get())->GetBufferSize());
                (*static_cast<IArchive*>(reader.get())) & migratingIndividualQueues[myRank];
                for (auto individual : migratingIndividualQueues[myRank])
                {
                    IMigrate* immigrant = individual->GetIMigrate();
                    immigrant->ImmigrateTo( nodes[immigrant->GetMigrationDestination()] );
                }
            }
#endif
        }; 

        SendToOthersFunc to_others_func = [this](IArchive* writer, int toRank)
        {
            *writer & migratingIndividualQueues[toRank];
            for (auto& individual : migratingIndividualQueues[toRank])
                individual->Recycle();  // delete individual
        };

        ClearDataFunc clear_data_func = [this](int rank)
        {
            migratingIndividualQueues[rank].clear();
        };

        ReceiveFromOthersFunc from_others_func = [this](IArchive* reader, int fromRank)
        {
            *reader & migratingIndividualQueues[fromRank];
            for (auto individual : migratingIndividualQueues[fromRank])
            {
                IMigrate* immigrant = individual->GetIMigrate();
                immigrant->ImmigrateTo( nodes[immigrant->GetMigrationDestination()] );
                if( individual->IsDead() )
                {
                    // we want the individual to migrate home and then finish dieing
                    delete individual;
                }
            }
        };

        MpiDataExchanger exchanger( "HumanMigration", to_self_func, to_others_func, from_others_func, clear_data_func );
        exchanger.ExchangeData(currentTime.time);

        // -----------------------------------------------------------
        // --- We sort the humans so that they are in a known order.
        // --- This solves the problem where individuals will get in
        // --- different orders depending on how many cores are used.
        // -----------------------------------------------------------
        if( m_pRngFactory->GetPolicy() == RandomNumberGeneratorPolicy::ONE_PER_NODE )
        {
            for( auto node : nodes )
            {
                node.second->SortHumans();
            }
        }
    }

    void Simulation::PostMigratingIndividualHuman(IIndividualHuman *i)
    {
        migratingIndividualQueues[nodeRankMap.GetRankFromNodeSuid(i->GetMigrationDestination())].push_back(i);
    }

    bool Simulation::CanSupportFamilyTrips() const
    {
        const MigrationParams* mp = MigrationConfig::GetMigrationParams();
        return ( ( mp->migration_pattern == MigrationPattern::SINGLE_ROUND_TRIPS)   &&
                 (!mp->enable_mig_local     || (mp->local_roundtrip_prob  == 1.0f)) &&
                 (!mp->enable_mig_air       || (mp->air_roundtrip_prob    == 1.0f)) &&
                 (!mp->enable_mig_regional  || (mp->region_roundtrip_prob == 1.0f)) &&
                 (!mp->enable_mig_sea       || (mp->sea_roundtrip_prob    == 1.0f)) );
    }

    //------------------------------------------------------------------
    //   Assorted getters and setters
    //-----------------------------------------------------------------

    const SimParams* Simulation::GetParams() const
    {
        return SimConfig::GetSimParams();
    }

    const DemographicsContext* Simulation::GetDemographicsContext() const
    {
        return demographicsContext;
    }

    const IdmDateTime& Simulation::GetSimulationTime() const
    {
        return currentTime;
    }

    int Simulation::GetSimulationTimestep() const
    {
        return currentTime.TimeAsSimpleTimestep();
    }

    suids::suid Simulation::GetNextInfectionSuid()
    {
        return infectionSuidGenerator();
    }

    suids::suid Simulation::GetNodeSuid( uint32_t external_node_id )
    {
        return nodeRankMap.GetSuidFromExternalID( external_node_id );
    }

    ExternalNodeId_t Simulation::GetNodeExternalID( const suids::suid& rNodeSuid )
    {
        return nodeRankMap.GetNodeInfo( rNodeSuid ).GetExternalID();
    }

    float Simulation::GetNodeInboundMultiplier( const suids::suid& rNodeSuid )
    {
        return nodeRankMap.GetNodeInfo( rNodeSuid ).GetInboundMult();
    }

    uint32_t Simulation::GetNodeRank( const suids::suid& rNodeSuid )
    {
        return nodeRankMap.GetRankFromNodeSuid( rNodeSuid );
    }

    // Should only be accessed by Node
    RANDOMBASE* Simulation::GetRng()
    {
        return rng;
    }

    std::vector<IReport*>& Simulation::GetReports()
    {
        return reports;
    }

    std::vector<IReport*>& Simulation::GetReportsNeedingIndividualData()
    {
        return individual_data_reports ;
    }

    int Simulation::getInitialRankFromNodeId( ExternalNodeId_t node_id )
    {
        return nodeRankMap.GetInitialRankFromNodeId(node_id); // R: leave as a wrapper call to nodeRankMap.GetInitialRankFromNodeId()
    }
    
    ISimulationContext * Simulation::GetContextPointer() 
    { 
        return this; 
    }

    void Simulation::PropagateContextToDependents()
    {
        ISimulationContext *context = GetContextPointer();
        for (auto& entry : nodes)
        {
            entry.second->SetContextTo(context);
        }
    }

    const IInterventionFactory* Simulation::GetInterventionFactory() const
    {
        return m_interventionFactoryObj;
    }

    REGISTER_SERIALIZABLE(Simulation);

    void Simulation::serialize(IArchive& ar, Simulation* obj)
    {
        Simulation& sim = *obj;
        ar.labelElement("serializationFlags") & (uint32_t&)sim.serializationFlags;

        if ((sim.serializationFlags.test(SerializationFlags::Population)) ||
            (sim.serializationFlags.test(SerializationFlags::Properties)))
        {
            ar.labelElement("infectionSuidGenerator") & sim.infectionSuidGenerator;
            ar.labelElement("m_RngFactory") & sim.m_pRngFactory;
            ar.labelElement( "rng" ) & sim.rng;
        }

        if (sim.serializationFlags.test(SerializationFlags::Population))
        {
            if (ar.IsReader()) {
                // Read the nodes element in case it's a version 1 serialized file which includes
                // the nodes in the nodes element.
                // If it's a version 2+ serialized file, the nodes element will be empty and this
                // will be harmless.
                ar.labelElement("nodes"); serialize(ar, sim.nodes);
            }
            else {
                // Write an empty element as the nodes will be serialized separately.
                NodeMap_t empty;
                ar.labelElement("nodes"); serialize(ar, empty);
            }
        }

        if (sim.serializationFlags.test(SerializationFlags::Parameters))
        {
            ar.labelElement( "custom_reports_filename" )          & sim.custom_reports_filename;
        }

        if (sim.serializationFlags.test(SerializationFlags::Properties))
        { }
    }

    void Simulation::serialize(IArchive& ar, NodeMap_t& node_map)
    {
        size_t count = (ar.IsWriter() ? node_map.size() : -1);
        ar.startArray(count);
        if (ar.IsWriter())
        {
            for (auto& entry : node_map)
            {
                ar.startObject();
                // entry.first is const which doesn't play well with IArchive operator '&'
                suids::suid suid(entry.first);
                ar.labelElement("suid") & suid;
                ar.labelElement("node") & entry.second;
                ar.endObject();
            }
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                ar.startObject();
                suids::suid suid;
                ISerializable* obj;
                ar.labelElement("suid") & suid;
                ar.labelElement("node") & obj;
                ar.endObject();
                node_map[suid] = static_cast<Node*>(obj);
            }
        }
        ar.endArray();
    }
}
