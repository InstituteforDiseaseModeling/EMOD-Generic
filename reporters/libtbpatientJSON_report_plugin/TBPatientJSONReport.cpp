/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "TBPatientJSONReport.h"

#include "BoostLibWrapper.h"
#include "Environment.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "IndividualCoInfection.h"
#include "../interventions/IDrug.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"

#include "FactorySupport.h"

using namespace json;
using namespace Kernel;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Name for logging, CustomReport.json, and DLL GetType()
SETUP_LOGGING( "TBPatientJSONReport" ) // <<< Name of this file

// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = {"TB_SIM", nullptr}; // <<< Types of simulation the report is to be used with

// Output file name
static const std::string _report_name = "TBPatientReport.json"; // <<< Filename to put data into

report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new TBPatientJSONReport()); // <<< Report to create
};

DllInterfaceHelper DLL_HELPER( _module, _sim_types, rif );

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// ------------------------------
// --- DLL Interface Methods
// ---
// --- The DTK will use these methods to establish communication with the DLL.
// ------------------------------

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

DTK_DLLEXPORT char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void __cdecl
GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char * __cdecl
GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT void __cdecl
GetReportInstantiator( Kernel::report_instantiator_function_t* pif )
{
    DLL_HELPER.GetReportInstantiator( pif );
}

#ifdef __cplusplus
}
#endif

// ---------------------------
// --- TBPatient Methods
// ---------------------------

TBPatient::TBPatient(int id_, float age_)
    : id(id_)
    , initial_age(age_)
    , n_drug_treatments(0)
{
}

TBPatient::~TBPatient()
{
}

// ----------------------------------------
// --- TBPatientJSONReport Methods
// ----------------------------------------

TBPatientJSONReport::TBPatientJSONReport()
{
    LOG_DEBUG( "CTOR\n" );
}

TBPatientJSONReport::~TBPatientJSONReport()
{
    LOG_DEBUG( "DTOR\n" );
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!! Commenting out the code to delete all the patients because it can take
    // !!! a long time to delete them.  If we are doing this, we are trying to delete
    // !!! the report and exit the simulation.  Hurry up and exit.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //for( auto& entry : patient_map )
    //{
    //    delete entry.second ;
    //    entry.second = nullptr ;
    //}
    patient_map.clear();
}

void TBPatientJSONReport::Initialize( unsigned int nrmSize )
{
    LOG_DEBUG( "Initialize\n" );
    report_name = _report_name;
}

void TBPatientJSONReport::BeginTimestep()
{
    LOG_DEBUG( "BeginTimestep\n" );
}

void TBPatientJSONReport::LogNodeData( Kernel::INodeContext * pNC )
{
    LOG_DEBUG( "LogNodeData\n" );
    //pNode->GetTime();
}

void TBPatientJSONReport::LogIndividualData( Kernel::IIndividualHuman* individual )
{
    LOG_DEBUG( "LogIndividualData\n" );

    // individual identifying info
    int id           = individual->GetSuid().data;
    double mc_weight = individual->GetMonteCarloWeight();
    double age       = individual->GetAge();

    // get TB contexts
    const Kernel::IndividualHumanCoInfection* individual_TB = static_cast<const Kernel::IndividualHumanCoInfection*>(individual);

    // get the correct existing patient or insert a new one
    TBPatient* patient = NULL;
    patient_map_t::const_iterator it = patient_map.find(id);
    if ( it == patient_map.end() )
    {
        patient = new TBPatient(id, age);
        patient_map.insert( std::make_pair(id, patient) );
    }
    else
    {
        patient = it->second;
    }

    // push back today's disease variables for infected individuals
    if ( individual->IsInfected() )
    {
        LOG_DEBUG_F("Individual %d with mc_weight %f at age %f is infected.\n", id, mc_weight, age);
        status_t current_status;
        int current_time = individual_TB->GetTime();

        //add, rename or delete additional types of events here
        current_status["a"]   = individual_TB->HasActiveInfection();
        current_status["l"]   = individual_TB->HasLatentInfection();
        current_status["pr"]  = individual_TB->HasPendingRelapseInfection();
        current_status["imm"] = individual_TB->IsImmune();
        current_status["mdr"] = individual_TB->IsMDR();
        current_status["sp"]  = individual_TB->IsSmearPositive();
        current_status["tx"]  = individual_TB->IsOnTreatment();
        current_status["ev"]  = individual_TB->IsEvolvedMDR();
        current_status["tn"]  = individual_TB->IsTreatmentNaive();
        current_status["ft"]  = individual_TB->HasFailedTreatment();
        current_status["fp"]  = individual_TB->IsFastProgressor();
        current_status["ep"]  = individual_TB->IsExtrapulmonary();
        current_status["api"] = individual_TB->HasActivePresymptomaticInfection();

        for (status_t::iterator iter = current_status.begin(); iter != current_status.end(); ++iter)
        {
            string event_key = iter->first;
            bool event_value = iter->second;

            if (patient->previous_status.find(event_key) == patient->previous_status.end()) //couldn't find, add the key to previous status and record as a new event
            {
                if (event_value) //assuming initial value is false
                {
                    Event new_event;
                    new_event.type = event_key;
                    new_event.t = current_time;
                    new_event.previous = false;
                    new_event.current = event_value;
                    patient->events.push_back(new_event);
                }
                patient->previous_status[event_key]=event_value;
            }
            else //compare value. if different, record as a new event
            {
                if (event_value != patient->previous_status[event_key])
                {
                    Event new_event;
                    new_event.type = event_key;
                    new_event.t = current_time;
                    new_event.previous = patient->previous_status[event_key];
                    new_event.current = event_value;
                    patient->events.push_back(new_event);
                }
                patient->previous_status[event_key]=event_value;
            }
        }

        // New drugs
        std::list<void*> drug_list = individual->GetInterventionsContext()->GetInterventionsByInterface( GET_IID(IDrug) );
        LOG_DEBUG_F( "Drug doses distributed = %d\n", drug_list.size() );

        if ( !drug_list.empty() && patient->drug_treatments.size() == 0 )
        {
            LOG_DEBUG_F("Individual (suid=%d) has been distributed %d drugs, but only just became infected.\n", patient->id, drug_list.size());
        }

        if ( drug_list.size() > patient->n_drug_treatments )
        {
            patient->n_drug_treatments++;
            IDrug* p_drug = static_cast<IDrug*>(drug_list.back());
            std::string new_drug_type = p_drug->GetDrugName();
            //patient->drug_treatments.push_back(new_drug_type);
            patient->drug_treatments.push_back("");
        }
        else
        {
            patient->drug_treatments.push_back(""); // empty string means no intervention in the MATLAB structure array format
        }
    }
}

void TBPatientJSONReport::EndTimestep( float currentTime, float dt )
{
    LOG_DEBUG( "EndTimestep\n" );
}

// TODO: are we ever going to want to use this on multi-core?  Lot's of data output!
void TBPatientJSONReport::Reduce()
{
    LOG_DEBUG( "Reduce\n" );
}

std::string TBPatientJSONReport::GetReportName() const
{
    return report_name;
}

void TBPatientJSONReport::Finalize()
{
    std::stringstream output_file_name;
    output_file_name << _report_name;
    LOG_INFO_F( "Writing file: %s\n", output_file_name.str().c_str() );

    Element elementRoot = String();
    QuickBuilder qb(elementRoot);

    // populate the structure matrix with individual patient data
    int idx = 0;
    for (auto &id_patient_pair : patient_map)
    {
        const TBPatient* patient = id_patient_pair.second;
        if (patient->events.size() > 0)
        {
            qb["patient_array"][idx]["id"         ] = Number(patient->id         );
            qb["patient_array"][idx]["initial_age"] = Number(patient->initial_age);

            for (int idx2=0; idx2 < patient->events.size(); idx2++)
            {
                qb["patient_array"][idx]["t"       ][idx2] = Number(patient->events[idx2].t       );
                qb["patient_array"][idx]["type"    ][idx2] = String(patient->events[idx2].type    );
                qb["patient_array"][idx]["previous"][idx2] = Number(patient->events[idx2].previous);
                qb["patient_array"][idx]["current" ][idx2] = Number(patient->events[idx2].current );
            }
            idx++;
        }
    }

    // write to an internal buffer first... if we write directly to the network share, performance is slow
    // (presumably because it's doing a bunch of really small writes of all the JSON elements instead of one
    // big write)
    ostringstream oss;
    Writer::Write(elementRoot, oss);

    ofstream report_json;
    FileSystem::OpenFileForWriting( report_json, FileSystem::Concat( EnvPtr->OutputPath, output_file_name.str() ).c_str() );

    report_json << oss.str();
    report_json.close();
}
