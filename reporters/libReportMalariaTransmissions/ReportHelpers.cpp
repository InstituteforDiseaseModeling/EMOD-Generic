/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportHelpers.h"

#include "Log.h"
#include "MalariaContexts.h"
#include "IIndividualHuman.h"
#include "IndividualEventContext.h"
#include "ReportUtilities.h"

SETUP_LOGGING("MalariaTransmissionReport")

namespace Kernel
{
    MalariaInfectionInfo::MalariaInfectionInfo(IInfection* infection)
    {
        id = infection->GetSuid().data;

        IInfectionMalaria* infection_malaria = NULL;
        if (s_OK != infection->QueryInterface(GET_IID(IInfectionMalaria), (void**)&infection_malaria) )
        {
            throw QueryInterfaceException(
                __FILE__, __LINE__, __FUNCTION__,
                "infection", "IInfectionMalaria", "IInfection");
        }

        asexual_density = infection_malaria->get_asexual_density();
        gametocyte_density = infection_malaria->get_mature_gametocyte_density();
    }


    MalariaIndividualInfo::MalariaIndividualInfo(IIndividualHuman* individual)
    {
        SetProperties(individual);
    }

    MalariaIndividualInfo::MalariaIndividualInfo(IIndividualHumanEventContext* context)
    {
        IIndividualHuman * individual = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHuman), (void**)&individual) )
        {
            throw QueryInterfaceException(
                __FILE__, __LINE__, __FUNCTION__,
                "context", "IIndividualHuman", "IIndividualHumanEventContext");
        }

        SetProperties(individual);
    }

    MalariaIndividualInfo::~MalariaIndividualInfo()
    {
        for (auto inf: infections)
        {
            delete inf;
        }
    }

    void MalariaIndividualInfo::SetProperties(IIndividualHuman* individual)
    {
        id = individual->GetSuid().data;
        age = individual->GetAge();
        infectiousness = individual->GetInfectiousness();

        const infection_list_t& infection_list = individual->GetInfections();
        for (auto inf: infection_list)
        {
            infections.push_back(_new_ MalariaInfectionInfo(inf));
        }
    }


    Transmission::Transmission(Location location_, uint32_t txId_, std::vector<uint32_t> txInfIds_, std::vector<float> txGams_, uint32_t acId_, uint32_t acInfId_)
        : location(location_)
        , transmitIndividualId(txId_)
        , transmitInfectionIds(txInfIds_)
        , transmitGametocyteDensities(txGams_)
        , acquireIndividualId(acId_)
        , acquireInfectionId(acInfId_)
    {}

    void Transmission::WriteJson(json::Object& root)
    {
        json::QuickBuilder json_doc(root);
        json_doc["timestep"]      = json::Number(location.second);
        json_doc["node_id"]       = json::Number(location.first);

        json_doc["acquireIndividualId"]   = json::Number(acquireIndividualId);
        json_doc["acquireInfectionId"]    = json::Number(acquireInfectionId);
        json_doc["transmitIndividualId"]  = json::Number(transmitIndividualId);

        json::Array tx_ids;
        for(size_t k1=0; k1<transmitInfectionIds.size(); k1++)
        {
            tx_ids.Insert(json::Number(transmitInfectionIds[k1]));
        }
        json_doc["transmitInfectionIds"] = tx_ids;

        json::Array tx_dense;
        for(size_t k1=0; k1<transmitGametocyteDensities.size(); k1++)
        {
            tx_dense.Insert(json::Number(transmitGametocyteDensities[k1]));
        }
        json_doc["transmitGametocyteDensities"] = tx_dense;
    }


    ClinicalSample::ClinicalSample(Location location_, std::string event_, uint32_t id_, std::vector<uint32_t> infIds_, std::vector<float> densities_)
        : location(location_)
        , sample_event(event_)
        , individualId(id_)
        , infectionIds(infIds_)
        , parasiteDensities(densities_)
    {}

    void ClinicalSample::WriteJson(json::Object& root)
    {
        json::QuickBuilder json_doc(root);
        json_doc["timestep"]      = json::Number(location.second);
        json_doc["node_id"]       = json::Number(location.first);

        json_doc["individualId"]  = json::Number(individualId);
        json_doc["sample_event"]  = json::String(sample_event.c_str());

        json::Array inf_ids;
        for(size_t k1=0; k1<infectionIds.size(); k1++)
        {
            inf_ids.Insert(json::Number(infectionIds[k1]));
        }
        json_doc["infectionIds"] = inf_ids;

        json::Array        para_dense;
        for(size_t k1=0; k1<parasiteDensities.size(); k1++)
        {
            para_dense.Insert(json::Number(parasiteDensities[k1]));
        }
        json_doc["parasiteDensities"] = para_dense;
    }
}
