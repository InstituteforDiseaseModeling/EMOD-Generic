/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "MalariaTransmissionReport.h"

#include <algorithm>
#include <numeric>

#include "FileSystem.h"
#include "Environment.h"
#include "Exceptions.h"
#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"
#include "IdmMpi.h"
#include "ReportUtilities.h"

#include "ISimulationContext.h"
#include "MalariaContexts.h"
#include "VectorContexts.h"
#include "IIndividualHuman.h"
#include "NodeEventContext.h"
#include "VectorCohortIndividual.h"
#include "IMigrate.h"
#include "RANDOM.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING("MalariaTransmissionReport")

static const char* _sim_types[] = {"MALARIA_SIM", nullptr};

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
    return new Kernel::MalariaTransmissionReport();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

namespace Kernel
{
    MigratingVector::MigratingVector(uint64_t id, ExternalNodeId_t from_node_id, ExternalNodeId_t to_node_id)
        : id(id)
        , from_node_id(from_node_id)
        , to_node_id(to_node_id)
    {
    }

    MigratingVector::MigratingVector(IVectorCohort* pvc, IVectorCohortIndividual* pivci, ISimulationContext* pSim, const suids::suid& nodeSuid)
    {
        IMigrate* pim = pvc->GetIMigrate();
        release_assert(pim);

        id = pivci->GetID();
        from_node_id = pSim->GetNodeExternalID(nodeSuid);
        to_node_id = pSim->GetNodeExternalID(pim->GetMigrationDestination());
    }

// ----------------------------------------
// --- MalariaTransmissionReport Methods
// ----------------------------------------

    MalariaTransmissionReport::MalariaTransmissionReport() 
        : BaseEventReport( _module )
        , m_PrettyFormat(true)
        , outputWritten(false)
        , timeStep(0)
    {
        LOG_DEBUG( "CTOR\n" );
    }

    MalariaTransmissionReport::~MalariaTransmissionReport()
    {
        LOG_DEBUG( "DTOR\n" );
    }

    bool MalariaTransmissionReport::Configure( const Configuration* inputJson )
    {
        initConfigTypeMap( "Pretty_Format", &m_PrettyFormat, "True implies pretty JSON format, false saves space.", false );

        return BaseEventReport::Configure( inputJson );
    }

    void MalariaTransmissionReport::LogVectorMigration(ISimulationContext* pSim, float currentTime, const suids::suid& nodeSuid, IVectorCohort* pvc)
    {
        IVectorCohortIndividual* pivci = pvc->GetCohortIndividual();
        release_assert(pivci);

        VectorStateEnum::Enum state = pvc->GetState();

        if (state == VectorStateEnum::STATE_INFECTIOUS)
        {
            // Need to track infectious mosquitos who are migrating
            MigratingVector mv(pvc, pivci, pSim, nodeSuid);
            LOG_DEBUG_F("Infectious mosquito (id=%d) migrating from node %d to %d\n", mv.id, mv.from_node_id, mv.to_node_id);

            auto found = infectious_mosquitos_current.find(mv.to_node_id);
            if (found == infectious_mosquitos_current.end())
            {
                infectious_mosquitos_current[mv.to_node_id] = InfectiousMosquitos_t();
            }

            infectious_mosquitos_current[mv.to_node_id].push_back(mv.id);
        }
        else if (state == VectorStateEnum::STATE_INFECTED && pvc->GetProgress() == 0)
        {
            // Also need to track newly infected mosquitos who are migrating
            MigratingVector mv(pvc, pivci, pSim, nodeSuid);
            LOG_DEBUG_F("Newly infected mosquito (id=%d) migrating from node %d to %d\n", mv.id, mv.from_node_id, mv.to_node_id);

            Location location = std::make_pair(mv.from_node_id, timeStep);
            infected_mosquito_buffer[mv.id] = location;
        }
    }

    bool MalariaTransmissionReport::notifyOnEvent( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger )
    {
        if( HaveUnregisteredAllEvents() )
        {
            return false ;
        }

        const std::string& StateChange = EventTrigger::pairs::lookup_key( trigger );

        uint32_t id = context->GetSuid().data;
        float age = context->GetAge();

        ExternalNodeId_t node_id = context->GetNodeEventContext()->GetExternalId();
        Location location = std::make_pair(node_id, timeStep);

        LOG_DEBUG_F("==> Notified of %s event by %d-year old individual (suid=%d).\n",
            StateChange.c_str(), (int)(age / DAYSPERYEAR), id);

        if(trigger == EventTrigger::NewInfection)
        {
            // ---------- Find ID of acquired infection
            MalariaIndividualInfo info = MalariaIndividualInfo(context);
            if (info.infections.empty())
            {
                throw IllegalOperationException(
                    __FILE__, __LINE__, __FUNCTION__,
                    "Notified of NewInfection by individual with empty infections list.");
            }
            uint32_t acInfId_ = info.infections.front()->id;  // most recent: IndividualHuman::AcquireNewInfection does push_front

            // ---------- Sample transmitting infectious mosquito from buffer
            uint32_t txId_ = -1;
            std::vector<uint32_t> txInfIds_;
            std::vector<float> txGams_;

            InfectiousMosquitos_t infectious_mosquitos;
            auto found = infectious_mosquitos_previous.find(node_id);
            if (found == infectious_mosquitos_previous.end())
            {
                LOG_WARN_F("No infectious mosquitos at t=%d, node_id=%d.\n",
                    location.second, location.first);
                transmissions.push_back(_new_ Transmission(location, txId_, txInfIds_, txGams_, id, acInfId_));
                return false;
            }
            else
            {
                infectious_mosquitos = found->second;
            }
            LOG_DEBUG_F("%d infectious mosquitos in buffer at t=%d, node_id=%d.\n",
                infectious_mosquitos.size(), location.second, location.first);
            uint64_t mosq_id = infectious_mosquitos.at(context->GetNodeEventContext()->GetRng()->uniformZeroToN16(infectious_mosquitos.size())); // TODO: SuidGenerator + Reduce() for multicore
            LOG_DEBUG_F("Sampled infectious mosquito (id=%d) at t=%d, node_id=%d.\n",
                mosq_id, location.second, location.first);

            // ---------- Sample location of transmitting mosquito's original infection from buffer
            auto infected_found = infected_mosquito_buffer.find(mosq_id);
            if (infected_found == infected_mosquito_buffer.end())
            {
                LOG_WARN_F("No newly infected mosquito with id=%d.\n", mosq_id);
                transmissions.push_back(_new_ Transmission(location, txId_, txInfIds_, txGams_, id, acInfId_));
                return false;
            }
            Location infected_location = infected_found->second;
            LOG_DEBUG_F("Sampled mosquito (id=%d) infected at t=%d, node_id=%d.\n",
                mosq_id, infected_location.second, infected_location.first);

            // ---------- Sample transmitting infectious human from buffer
            auto humans_found = infected_human_buffer.find(infected_location);
            if (humans_found == infected_human_buffer.end())
            {
                LOG_WARN_F("No infectious humans at t=%d, node_id=%d.\n",
                    infected_location.second, infected_location.first);
                transmissions.push_back(_new_ Transmission(location, txId_, txInfIds_, txGams_, id, acInfId_));
                return false;
            }
            const auto& inf_humans = humans_found->second;
            LOG_DEBUG_F("%d infectious humans in buffer at t=%d, node_id=%d.\n",
                inf_humans.size(), infected_location.second, infected_location.first);

            // weighted sampling by infectiousness
            std::vector<float> weights;
            for( const auto &hum: inf_humans )
            {
                weights.push_back( hum->infectiousness );
            }
            std::vector<float> cum_weights(weights.size(), 0);
            std::partial_sum(weights.begin(), weights.end(), cum_weights.begin());
            float total = cum_weights.back();
            LOG_DEBUG_F("Total infectiousness = %0.2f.  Normalizing weights...\n", total);
            std::for_each(cum_weights.begin(), cum_weights.end(), [total](float &w){w /= total;});

            auto up = std::upper_bound(cum_weights.begin(), cum_weights.end(), context->GetNodeEventContext()->GetRng()->e());
            int idx = std::distance(cum_weights.begin(), up);
            const MalariaIndividualInfo& txHuman = *(inf_humans.at(idx));
            txId_ = txHuman.id;
            LOG_DEBUG_F("Transmitting human ID = %d\n", txId_);

            //// ---------- Sample infecting strains from human buffer

            std::vector<MalariaInfectionInfo*> txInfections = txHuman.infections;
            int n_infections = txInfections.size();

            if (n_infections == 0)
            {
               throw IllegalOperationException(__FILE__, __LINE__, __FUNCTION__,
                   "Infectious individual with no infections.");
            }

            std::ostringstream ss;
            for (int idx = 0; idx < n_infections; idx++)
            {
                txInfIds_.push_back(txInfections[idx]->id);
                txGams_.push_back(txInfections[idx]->gametocyte_density);
                ss << txInfIds_[idx] << ":" << int(txGams_[idx]) << " ";
            }
            LOG_DEBUG_F("Transmitting infection densities = [ %s]\n", ss.str().c_str());

            Transmission *t = _new_ Transmission(location, txId_, txInfIds_, txGams_, id, acInfId_);
            transmissions.push_back(t);
        }
        else
        {
            // Infection-sampling events, e.g. Received_Treatment or Received_RCD_Drugs
            MalariaIndividualInfo info = MalariaIndividualInfo(context);
            int n_infections = info.infections.size();

            if (n_infections == 0)
            {
                LOG_DEBUG_F("Notified of %s event by person id=%d with no infections.\n", StateChange.c_str(), id);
                return false;
            }

            std::vector<uint32_t> infIds_;
            std::vector<float> densities_;

            std::ostringstream ss;
            for (int idx = 0; idx < n_infections; idx++)
            {
                infIds_.push_back(info.infections[idx]->id);
                densities_.push_back(info.infections[idx]->asexual_density);
                ss << infIds_[idx] << ":" << int(densities_[idx]) << " ";
            }
            LOG_DEBUG_F("Sampled '%s' individual (suid=%d): infection densities = [ %s]\n",
                StateChange.c_str(), id, ss.str().c_str());

            ClinicalSample *s = _new_ ClinicalSample(location, StateChange, id, infIds_, densities_);
            samples.push_back(s);
        }

        return true;
    }

    bool MalariaTransmissionReport::IsActive() const
    {
        // Same conditions for LogData as the notifyEvent
        return HaveRegisteredAllEvents() && !HaveUnregisteredAllEvents();
    }

    bool MalariaTransmissionReport::IsFinished() const
    {
        return HaveRegisteredAllEvents() && HaveUnregisteredAllEvents();
    }

    bool MalariaTransmissionReport::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return IsActive();
    }

    void MalariaTransmissionReport::LogIndividualData( IIndividualHuman* individual )
    {
        if (!individual->IsInfected())
            return;

        //LOG_VALID_F("\tLogIndividualData for infected %d-year old (suid=%d).\n",
        //    (int)(individual->GetAge() / DAYSPERYEAR), individual->GetSuid().data);

        ExternalNodeId_t node_id = individual->GetParent()->GetExternalID();
        Location location = std::make_pair(node_id, timeStep);

        infected_human_buffer[location].push_back(
            std::unique_ptr<MalariaIndividualInfo>(_new_ MalariaIndividualInfo(individual)));
    }

    InfectiousMosquitos_t MalariaTransmissionReport::BufferInfectiousVectors( INodeEventContext *context )
    {
        INodeContext* pNC = context->GetNodeContext();
        release_assert(pNC);

        return BufferInfectiousVectors(pNC);
    }

    InfectiousMosquitos_t MalariaTransmissionReport::BufferInfectiousVectors(INodeContext *pNC)
    {
        ExternalNodeId_t node_id = pNC->GetExternalID();
        Location location = std::make_pair(node_id, timeStep);

        INodeVector* inv = pNC->GetNodeVector();
        release_assert(inv);

        InfectiousMosquitos_t tmp_infectious;
        for (auto pop : inv->GetVectorPopulationReporting())
        {
            // Get infectious mosquitoes
            std::vector<uint64_t> infectious_suids = pop->GetInfectiousVectorIds();
            if (!infectious_suids.empty())
            {
                LOG_DEBUG_F("%d infectious %s mosquitos, node_id=%d\n", infectious_suids.size(), pop->get_SpeciesID().c_str(), node_id);
                tmp_infectious.insert(tmp_infectious.end(), infectious_suids.begin(), infectious_suids.end());
            }
        }

        if (!tmp_infectious.empty())
        {
            auto found = infectious_mosquitos_current.find(node_id);
            if (found == infectious_mosquitos_current.end())
            {
                LOG_DEBUG_F("Adding first %d mosquitos to node-%d buffer\n", tmp_infectious.size(), node_id);
                infectious_mosquitos_current.insert(std::make_pair(node_id, tmp_infectious));
            }
            else
            {
                LOG_DEBUG_F("Adding %d mosquitos to existing node-%d buffer (size=%d)\n", tmp_infectious.size(), node_id, found->second.size());
                found->second.insert(found->second.end(), tmp_infectious.begin(), tmp_infectious.end());
            }
        }

        return tmp_infectious;
    }

    void MalariaTransmissionReport::LogNodeData( INodeContext * pNC )
    {
        if (!IsActive())
            return;

        ExternalNodeId_t node_id = pNC->GetExternalID();
        Location location = std::make_pair(node_id, timeStep + 1);  // notifyOnEvent called before LogNodeData, so offset by 1.
        LOG_DEBUG_F("LogNodeData for suid=%d (externalId=%d).\n", pNC->GetSuid().data, node_id);

        INodeVector* inv = pNC->GetNodeVector();
        release_assert(inv);

        for (auto pop : inv->GetVectorPopulationReporting())
        {
            // Get newly infected mosquitos
            std::vector<uint64_t> newly_infected_mosquitos = pop->GetNewlyInfectedVectorIds();
            LOG_DEBUG_F("%d newly infected %s mosquitos\n", newly_infected_mosquitos.size(), pop->get_SpeciesID().c_str());
            for (auto mosquito_id : newly_infected_mosquitos)
            {
                infected_mosquito_buffer[mosquito_id] = location;
            }
        }

        BufferInfectiousVectors(pNC);
    }

    void MalariaTransmissionReport::EndTimestep( float currentTime, float dt )
    {
        timeStep++;

        // Write outputs when the reporting interval is finished
        if (IsFinished() && !outputWritten)
        {
            WriteOutput(currentTime);
            outputWritten = true;
        }

        // Infectious mosquito buffer only needs to carry over for one extra timestep
        infectious_mosquitos_current.swap(infectious_mosquitos_previous);
        infectious_mosquitos_current.clear();

        // Let's only clear the infected mosquito and human buffers periodically
        if (timeStep % 10)
            return;

        int threshold = timeStep - 40;  // erase info older than longest plausible mosquito lifetime

        for (auto it = infected_mosquito_buffer.cbegin(); it != infected_mosquito_buffer.cend();)
        {
            Location location = it->second;
            int ts = location.second; // time-step of newly infected mosquito by ID
            if (ts < threshold)
            {
                //LOG_VALID_F("Clearing mosquito (id=%d) infected at t=%d, node_id=%d.\n", it->first, ts, location.first);
                infected_mosquito_buffer.erase(it++);
            }
            else
            {
                ++it;
            }
        }

        for (auto it = infected_human_buffer.cbegin(); it != infected_human_buffer.cend();)
        {
            Location location = it->first;
            int ts = location.second; // time-step of Location of infected human
            if (ts < threshold)
            {
                //LOG_VALID_F("Clearing infected human buffer at t=%d, node_id=%d.\n", ts, location.first);
                infected_human_buffer.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }

    void MalariaTransmissionReport::WriteOutput( float currentTime )
    {
        json::Object       obj_root;
        json::QuickBuilder json_doc(obj_root);

        // Accumulate array of transmissions as JSON
        json::Array tx_array;
        for(auto &transmission : transmissions)
        {
            json::Object tx_obj;
            transmission->WriteJson(tx_obj);
            tx_array.Insert(tx_obj);
        }
        json_doc["transmissions"] = tx_array;

        // sampled infections (treated clinical cases, reactive follow-up, etc.)
        json::Array samp_array;
        for (auto &sample : samples)
        {
            json::Object samp_obj;
            sample->WriteJson(samp_obj);
            samp_array.Insert(samp_obj);
        }
        json_doc["samples"] = samp_array;

        // Filename
        std::ostringstream output_file_name;
        output_file_name << GetBaseOutputFilename() << ".json";

        // Output file
        LOG_INFO_F( "Writing file: %s\n", output_file_name.str().c_str() );
        ofstream ofs;
        ofs.open( FileSystem::Concat( EnvPtr->OutputPath, output_file_name.str() ).c_str() );
        std::string indent_chars((m_PrettyFormat?"    ":""));
        json::Writer::Write(json_doc, ofs, indent_chars, m_PrettyFormat);
        ofs.close();
    }
}
