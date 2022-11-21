/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "MalariaPatientJSONReport.h"

#include "FileSystem.h"
#include "Environment.h"
#include "Exceptions.h"
#include "IndividualMalaria.h"
#include "../interventions/IDrug.h"
#include "IdmDateTime.h"
#include "INodeContext.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"

#include "FactorySupport.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING( "MalariaPatientJSONReport" )

static const char*       _sim_types[] = {"MALARIA_SIM", nullptr};
static const std::string _report_name = "MalariaPatientReport.json";

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
    return new MalariaPatientJSONReport();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

using namespace Kernel ;

// ---------------------------
// --- MalariaPatent Methods
// ---------------------------

MalariaPatient::MalariaPatient(int id_, float age_, float birthday_)
    : id(id_)
    , initial_age(age_)
    , birthday(birthday_)
    , n_drug_treatments(0)
{
}

MalariaPatient::~MalariaPatient()
{
}

void MalariaPatient::JSerialize( json::Object& root )
{
    LOG_DEBUG("Serializing MalariaPatient\n");

    LOG_DEBUG("Inserting simple variables\n");
    json::QuickBuilder json_obj(root);
    json_obj["id"]          = json::Number(id);
    json_obj["initial_age"] = json::Number(initial_age);
    json_obj["birthday"]    = json::Number(birthday);

    LOG_DEBUG("Inserting array variables\n");
    SerializeChannel("true_asexual_parasites", true_asexual_density, root);
    SerializeChannel("true_gametocytes", true_gametocyte_density, root);
    SerializeChannel("asexual_parasites", asexual_parasite_density, root);
    SerializeChannel("gametocytes", gametocyte_density, root);
    SerializeChannel("infected_mosquito_fraction", infectiousness, root);
    SerializeChannel("hemoglobin", hemoglobin, root);
    SerializeChannel("temps", fever, root);
    SerializeChannel("treatment", drug_treatments, root);
    SerializeChannel("asexual_positive_fields", pos_fields_of_view, root);
    SerializeChannel("gametocyte_positive_fields", gametocyte_pos_fields_of_view, root);
}

void MalariaPatient::SerializeChannel( std::string channel_name, std::vector<float> &channel_data, json::Object& root )
{
    json::QuickBuilder json_obj(root);

    json::Array arr_data;
    for(size_t k1=0; k1<channel_data.size(); k1++)
    {
        arr_data.Insert(json::Number(channel_data[k1]));
    }
    json_obj[channel_name.c_str()] = arr_data;
}

void MalariaPatient::SerializeChannel( std::string channel_name, std::vector<std::string> &channel_data, json::Object& root )
{
    json::QuickBuilder json_obj(root);

    json::Array arr_data;
    for(size_t k1=0; k1<channel_data.size(); k1++)
    {
        arr_data.Insert(json::String(channel_data[k1]));
    }
    json_obj[channel_name.c_str()] = arr_data;
}

// ----------------------------------------
// --- MalariaPatientJSONReport Methods
// ----------------------------------------

MalariaPatientJSONReport::MalariaPatientJSONReport()
    : BaseReport()
    , report_name( _report_name )
    , simtime(0.0f)
    , ntsteps(0)
    , patient_map()
{
    LOG_DEBUG( "CTOR\n" );
}

MalariaPatientJSONReport::~MalariaPatientJSONReport()
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

void MalariaPatientJSONReport::Initialize( unsigned int nrmSize )
{
    LOG_DEBUG( "Initialize\n" );
}

void MalariaPatientJSONReport::BeginTimestep()
{
    LOG_DEBUG( "BeginTimestep\n" );
    ntsteps += 1;
}

void MalariaPatientJSONReport::LogNodeData( INodeContext * pNC )
{
    LOG_DEBUG( "LogNodeData\n" );
    simtime = pNC->GetTime().time;
}

bool MalariaPatientJSONReport::IsCollectingIndividualData( float currentTime, float dt ) const
{
    return true;
}

void MalariaPatientJSONReport::LogIndividualData( IIndividualHuman* individual )
{
    LOG_DEBUG( "LogIndividualData\n" );

    // individual identifying info
    int id           = individual->GetSuid().data;
    double mc_weight = individual->GetMonteCarloWeight();
    double age       = individual->GetAge();

    // get malaria contexts
    const IndividualHumanMalaria* individual_malaria = static_cast<const IndividualHumanMalaria*>(individual);
    IMalariaSusceptibility* susceptibility_malaria = individual_malaria->GetMalariaSusceptibilityContext();

    // get the correct existing patient or insert a new one
    MalariaPatient* patient = NULL;
    patient_map_t::const_iterator it = patient_map.find(id);
    if ( it == patient_map.end() )
    {
        patient = new MalariaPatient(id, age, simtime-age);
        patient_map.insert( std::make_pair(id, patient) );
    }
    else
    {
        patient = it->second;
    }

    // Push back today's disease variables for infected individuals
    float max_fever = susceptibility_malaria->GetMaxFever();
    patient->fever.push_back( max_fever > 0 ? max_fever + 37.0f : -1.0f );
    patient->hemoglobin.push_back( susceptibility_malaria->GetHemoglobin() );
    patient->infectiousness.push_back( individual->GetInfectiousness() * 100.0f ); // (100.0f = turn fraction into percentage)

    // True values in model
    patient->true_asexual_density.push_back( susceptibility_malaria->get_parasite_density() ); // Getting this directly will make the value one day earlier than the other three densities
    patient->true_gametocyte_density.push_back( individual_malaria->GetGametocyteDensity() );

    // Values incorporating variability and sensitivity of blood test
    patient->asexual_parasite_density.push_back( individual_malaria->CheckParasiteCountWithTest( MALARIA_TEST_BLOOD_SMEAR ) );
    patient->gametocyte_density.push_back( individual_malaria->CheckGametocyteCountWithTest( MALARIA_TEST_BLOOD_SMEAR ) );

    // Positive fields of view (out of 200 views in Garki-like setup)
    int pos_fields = 0;
    int gam_pos_fields = 0;
    individual_malaria->CountPositiveSlideFields(individual->GetParent()->GetRng(), 200, 1.0f/400, pos_fields, gam_pos_fields);
    patient->pos_fields_of_view.push_back(float(pos_fields));
    patient->gametocyte_pos_fields_of_view.push_back(float(gam_pos_fields));

    // New drugs
    std::list<IDrug*> drug_list = individual->GetInterventionsContext()->GetDrugInterventions();
    LOG_DEBUG_F( "Drug doses distributed = %d\n", drug_list.size() );

    int new_drugs = drug_list.size() - patient->n_drug_treatments;
    patient->n_drug_treatments += new_drugs;
    std::string new_drug_names = "";

    while(new_drugs > 0)
    {
        IDrug* p_drug = drug_list.back();
        new_drug_names += p_drug->GetDrugName();
        drug_list.pop_back();
        new_drugs--;
        if (new_drugs == 0) break;
        new_drug_names += " + ";
    }

    patient->drug_treatments.push_back(new_drug_names);
}

void MalariaPatientJSONReport::EndTimestep( float currentTime, float dt )
{
    LOG_DEBUG( "EndTimestep\n" );
}

// TODO: are we ever going to want to use this on multi-core?  Lot's of data output!
void MalariaPatientJSONReport::Reduce()
{
    LOG_DEBUG( "Reduce\n" );
}

std::string MalariaPatientJSONReport::GetReportName() const
{
    return report_name;
}

void MalariaPatientJSONReport::Finalize()
{
    // Open output file
    std::ostringstream output_file_name;
    output_file_name << _report_name;
    LOG_INFO_F( "Writing file: %s\n", output_file_name.str().c_str() );
    ofstream ofs;
    FileSystem::OpenFileForWriting( ofs, FileSystem::Concat( EnvPtr->OutputPath, output_file_name.str() ).c_str() );

    // Accumulate array of patients as JSON
    json::Object obj_root;
    json::QuickBuilder json_doc(obj_root);

    json::Array arr_patient;
    for(auto &id_patient_pair: patient_map)
    {
        json::Object dat_patient;
        MalariaPatient* patient = id_patient_pair.second;
        patient->JSerialize(dat_patient);
        arr_patient.Insert(dat_patient);
    }
    json_doc["patient_array"] = arr_patient;
    json_doc["ntsteps"]       = json::Number(ntsteps);

    // Write output to file
    json::Writer::Write(json_doc, ofs, "", false);
    ofs.close();
}

