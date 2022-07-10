/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "MalariaSurveyJSONAnalyzer.h"

#include "FileSystem.h"
#include "Environment.h"
#include "Exceptions.h"
#include "IndividualMalaria.h"
#include "SusceptibilityMalaria.h"
#include "NodeEventContext.h"
#include "INodeContext.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"
#include "ReportUtilities.h"
#include "ReportUtilitiesMalaria.h"

#include "math.h"

//******************************************************************************

//******************************************************************************

SETUP_LOGGING( "MalariaSurveyJSONAnalyzer" )

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
    return new Kernel::MalariaSurveyJSONAnalyzer();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ---------------------------
// --- MalariaPatent Methods
// ---------------------------
namespace Kernel
{
    MalariaPatient::MalariaPatient(int id_, float age_, float local_birthday_)
        : id(id_)
        , node_id(-1)
        , initial_age(age_)
        , local_birthday(local_birthday_)
    {
    }

    MalariaPatient::~MalariaPatient()
    {
    }

    void MalariaPatient::Serialize( json::Object& root )
    {
        LOG_DEBUG("Serializing MalariaPatient\n");

        json::QuickBuilder mp_json(root);
        LOG_DEBUG("Inserting simple variables\n");
        mp_json["id"]             = json::Number(id);
        mp_json["node_id"]        = json::Number(node_id);
        mp_json["initial_age"]    = json::Number(initial_age);
        mp_json["local_birthday"] = json::Number(local_birthday);

        json::Array arr_strains;
        for (auto& inner : strain_ids)
        {
            json::Array arr_pair;
            arr_pair.Insert(json::Number(inner.first));
            arr_pair.Insert(json::Number(inner.second));
            arr_strains.Insert(arr_pair);
        }
        mp_json["strain_ids"] = arr_strains;

        LOG_DEBUG("Inserting array variables\n");
        ReportUtilities::SerializeVector( root, "ip_data",                        ip_data                          );
        ReportUtilities::SerializeVector( root, "true_asexual_parasites",         true_asexual_density             );
        ReportUtilities::SerializeVector( root, "true_gametocytes",               true_gametocyte_density          );
        ReportUtilities::SerializeVector( root, "smeared_true_asexual_parasites", smeared_true_asexual_density     );
        ReportUtilities::SerializeVector( root, "smeared_true_gametocytes",       smeared_true_gametocyte_density  );
        ReportUtilities::SerializeVector( root, "asexual_parasites",              asexual_parasite_density         );
        ReportUtilities::SerializeVector( root, "gametocytes",                    gametocyte_density               );
        ReportUtilities::SerializeVector( root, "smeared_asexual_parasites",      smeared_asexual_parasite_density );
        ReportUtilities::SerializeVector( root, "smeared_gametocytes",            smeared_gametocyte_density       );
        ReportUtilities::SerializeVector( root, "infectiousness",                 infectiousness                   );
        ReportUtilities::SerializeVector( root, "infectiousness_smeared",         infectiousness_smeared           );
        ReportUtilities::SerializeVector( root, "infectiousness_age_scaled",      infectiousness_age_scaled        );
        ReportUtilities::SerializeVector( root, "pos_asexual_fields",             pos_asexual_fields               );
        ReportUtilities::SerializeVector( root, "pos_gametocyte_fields",          pos_gametocyte_fields            );
        ReportUtilities::SerializeVector( root, "temps",                          fever                            );
        ReportUtilities::SerializeVector( root, "rdt",                            rdt                              );
    }

    std::vector< std::pair<uint32_t,uint64_t> > DeserializeStrainIds( json::Object& root )
    {
        std::vector< std::pair<uint32_t,uint64_t> > value_list;

        json::QuickInterpreter q_int(root);
        json::Array json_array = q_int["strain_ids"].As<json::Array>();
        for( unsigned int i = 0 ; i < json_array.Size(); ++i )
        {
            uint32_t first  = static_cast<uint32_t>(q_int["strain_ids"][i][0].As<json::Number>());
            uint64_t second = static_cast<uint64_t>(q_int["strain_ids"][i][1].As<json::Number>());
            value_list.push_back( std::make_pair( first, second ) );
        }
        return value_list;
    }

    void MalariaPatient::Deserialize( json::Object& root )
    {
        json::QuickInterpreter q_int(root);
        id             = static_cast<int>(q_int["id"].As<json::Number>());
        node_id        = static_cast<int>(q_int["node_id"].As<json::Number>());
        initial_age    = static_cast<float>(q_int["initial_age"].As<json::Number>());
        local_birthday = static_cast<float>(q_int["local_birthday"].As<json::Number>());

        strain_ids = DeserializeStrainIds( root );

        ReportUtilities::DeserializeVector( root, false, "ip_data",                        ip_data                          );
        ReportUtilities::DeserializeVector( root, false, "true_asexual_parasites",         true_asexual_density             );
        ReportUtilities::DeserializeVector( root, false, "true_gametocytes",               true_gametocyte_density          );
        ReportUtilities::DeserializeVector( root, false, "smeared_true_asexual_parasites", smeared_true_asexual_density     );
        ReportUtilities::DeserializeVector( root, false, "smeared_true_gametocytes",       smeared_true_gametocyte_density  );
        ReportUtilities::DeserializeVector( root, false, "asexual_parasites",              asexual_parasite_density         );
        ReportUtilities::DeserializeVector( root, false, "gametocytes",                    gametocyte_density               );
        ReportUtilities::DeserializeVector( root, false, "smeared_asexual_parasites",      smeared_asexual_parasite_density );
        ReportUtilities::DeserializeVector( root, false, "smeared_gametocytes",            smeared_gametocyte_density       );
        ReportUtilities::DeserializeVector( root, false, "infectiousness",                 infectiousness                   );
        ReportUtilities::DeserializeVector( root, false, "infectiousness_smeared",         infectiousness_smeared           );
        ReportUtilities::DeserializeVector( root, false, "infectiousness_age_scaled",      infectiousness_age_scaled        );
        ReportUtilities::DeserializeVector( root, false, "pos_asexual_fields",             pos_asexual_fields               );
        ReportUtilities::DeserializeVector( root, false, "pos_gametocyte_fields",          pos_gametocyte_fields            );
        ReportUtilities::DeserializeVector( root, false, "temps",                          fever                            );
        ReportUtilities::DeserializeVector( root, false, "rdt",                            rdt                              );
    }

// ----------------------------------------
// --- MalariaPatientMap Methods
// ----------------------------------------

    MalariaPatientMap::MalariaPatientMap()
    {
    }

    MalariaPatientMap::~MalariaPatientMap()
    {
    }

    void MalariaPatientMap::Clear()
    {
        for( auto& entry: m_Map )
        {
            delete entry.second;
        }
        m_Map.clear();
    }

    template <typename T>
    void UpdateVector( std::vector<T>& existing_vector, std::vector<T>& new_vector )
    {
        for( int i = existing_vector.size(); i < new_vector.size() ; ++i )
        {
            existing_vector.push_back( new_vector[i] );
        }
    }

    void MalariaPatientMap::Update( const IIntervalData& rOther )
    {
        const MalariaPatientMap& rOtherMap = static_cast< const MalariaPatientMap& >(rOther);

        for( auto& other_entry : rOtherMap.m_Map )
        {
            MalariaPatient* new_patient = other_entry.second;
            MalariaPatient* existing_patient = FindPatient( new_patient->id );

            if( existing_patient == nullptr )
            {
                existing_patient = new MalariaPatient(new_patient->id,-1,-1);
                Add( existing_patient );
            }

            // set in constructor
            //existing_patient->id             = new_patient->id;

            existing_patient->node_id        = new_patient->node_id;
            existing_patient->initial_age    = new_patient->initial_age;
            existing_patient->local_birthday = new_patient->local_birthday;
            existing_patient->strain_ids     = new_patient->strain_ids;

            UpdateVector( existing_patient->ip_data,                          new_patient->ip_data                          );
            UpdateVector( existing_patient->true_asexual_density,             new_patient->true_asexual_density             );
            UpdateVector( existing_patient->true_gametocyte_density,          new_patient->true_gametocyte_density          );
            UpdateVector( existing_patient->smeared_true_asexual_density,     new_patient->smeared_true_asexual_density     );
            UpdateVector( existing_patient->smeared_true_gametocyte_density,  new_patient->smeared_true_gametocyte_density  );
            UpdateVector( existing_patient->asexual_parasite_density ,        new_patient->asexual_parasite_density         );
            UpdateVector( existing_patient->gametocyte_density,               new_patient->gametocyte_density               );
            UpdateVector( existing_patient->smeared_asexual_parasite_density, new_patient->smeared_asexual_parasite_density );
            UpdateVector( existing_patient->smeared_gametocyte_density,       new_patient->smeared_gametocyte_density       );
            UpdateVector( existing_patient->infectiousness,                   new_patient->infectiousness                   );
            UpdateVector( existing_patient->infectiousness_smeared,           new_patient->infectiousness_smeared           );
            UpdateVector( existing_patient->infectiousness_age_scaled,        new_patient->infectiousness_age_scaled        );
            UpdateVector( existing_patient->pos_asexual_fields,               new_patient->pos_asexual_fields               );
            UpdateVector( existing_patient->pos_gametocyte_fields,            new_patient->pos_gametocyte_fields            );
            UpdateVector( existing_patient->fever,                            new_patient->fever                            );
            UpdateVector( existing_patient->rdt,                              new_patient->rdt                              );
        }
    }

    void MalariaPatientMap::Serialize( json::Object& root )
    {
        json::QuickBuilder mpm_json(root);

        json::Array arr_mpm;
        for( auto &id_patient_pair: m_Map )
        {
            MalariaPatient* patient = id_patient_pair.second;
            release_assert( patient );
            json::Object mpm_data;
            patient->Serialize(mpm_data);
            arr_mpm.Insert(mpm_data);
        }
        mpm_json["patient_array"] = arr_mpm;
    }

    void MalariaPatientMap::Deserialize( json::Object& root )
    {
        json::QuickInterpreter q_int(root);

        json::Array json_array = q_int["patient_array"].As<json::Array>();

        for( size_t i = 0 ; i < json_array.Size() ; ++i )
        {
            MalariaPatient* new_patient = new MalariaPatient(-1,-1,-1);
            json::Object mp_obj = q_int["patient_array"][i].As<json::Object>();
            new_patient->Deserialize( mp_obj );
            Add( new_patient );
        }
    }

    MalariaPatient* MalariaPatientMap::FindPatient( uint32_t id )
    {
        MalariaPatient* p_patient = nullptr;
        std::map<uint32_t,MalariaPatient*>::iterator it = m_Map.find( id );
        if( it != m_Map.end() )
        {
            p_patient = it->second;
        }
        return p_patient;
    }

    void MalariaPatientMap::Add( MalariaPatient* pPatient )
    {
        m_Map.insert( std::make_pair( pPatient->id, pPatient ) );
    }

// ----------------------------------------
// --- MalariaSurveyJSONAnalyzer Methods
// ----------------------------------------

#define Survey_IP_Key_To_Collect_DESC_TEXT "Name of the Individual Property Key whose value to collect.  Empty string means collect values for all IPs."

    MalariaSurveyJSONAnalyzer::MalariaSurveyJSONAnalyzer()
        : BaseEventReportIntervalOutput( _module, true, new MalariaPatientMap(), new MalariaPatientMap() ) // true => one file per report
        , m_IPKeyToCollect()
        , m_pPatientMap(nullptr)
    {
    }

    MalariaSurveyJSONAnalyzer::~MalariaSurveyJSONAnalyzer()
    {
    }

    bool MalariaSurveyJSONAnalyzer::Configure( const Configuration* inputJson )
    {
        initConfigTypeMap( "IP_Key_To_Collect", &m_IPKeyToCollect, Survey_IP_Key_To_Collect_DESC_TEXT, "" );

        bool configured = BaseEventReportIntervalOutput::Configure( inputJson );
        if( configured )
        {
            m_pPatientMap = static_cast<MalariaPatientMap*>(m_pIntervalData);
        }
        return configured;
    }

    void MalariaSurveyJSONAnalyzer::Initialize( unsigned int nrmSize )
    {
        if( !m_IPKeyToCollect.empty() )
        {
            // If user provides a key, ensure it is valid.
            IPKey key( m_IPKeyToCollect );
            release_assert( key.IsValid() ); // an exception should be thrown in the line above
        }

        BaseEventReportIntervalOutput::Initialize( nrmSize );
    }

    bool MalariaSurveyJSONAnalyzer::notifyOnEvent( IIndividualHumanEventContext *context, const EventTrigger::Enum& trigger )
    {
        if( HaveUnregisteredAllEvents() )
        {
            return false ;
        }

        LOG_DEBUG_F( "MalariaSurveyAnalyzer notified of event by %d-year old individual.\n", (int) (context->GetAge() / DAYSPERYEAR) );

        // individual context for suid
        IIndividualHumanContext * iindividual = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanContext), (void**)&iindividual) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanContext", "IIndividualHumanEventContext");
        }

        int id           = iindividual->GetSuid().data;
        double mc_weight = context->GetMonteCarloWeight();
        double age       = context->GetAge();


        std::string ip_info;
        IPKeyValueContainer* p_props = iindividual->GetEventContext()->GetProperties();
        if( m_IPKeyToCollect.empty() )
        {
            ip_info = p_props->ToString();
        }
        else
        {
            ip_info = p_props->Get( IPKey( m_IPKeyToCollect ) ).ToString();
        }

        // get malaria contexts
        IMalariaHumanContext * individual_malaria = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IMalariaHumanContext), (void**)&individual_malaria) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IMalariaHumanContext", "IIndividualHumanEventContext");
        }
        IMalariaSusceptibility* susceptibility_malaria = individual_malaria->GetMalariaSusceptibilityContext();

        // get the correct existing patient or insert a new one
        MalariaPatient* patient = m_pPatientMap->FindPatient(id);

        if( patient == nullptr )
        {
            patient = new MalariaPatient(id, age, m_interval_timer-age); // birthday relative to current interval (i.e. output file)
            m_pPatientMap->Add( patient );

            patient->strain_ids = individual_malaria->GetInfectingStrainIds();
            INodeEventContext* node_context = context->GetNodeEventContext();
            patient->node_id = node_context->GetExternalId();
        }

        patient->ip_data.push_back( ip_info );

        // Push back today's disease variables
        float max_fever = susceptibility_malaria->GetMaxFever();
        patient->fever.push_back( max_fever > 0 ? max_fever + 37.0f : -1.0f );

        // GetInfectiousness is an inline function of IndividualHuman (not belonging to any queriable interface presently)
        float infectiousness = static_cast<IndividualHuman*>(context)->GetInfectiousness();
        float infectiousness_smeared = ReportUtilitiesMalaria::BinomialInfectiousness( iindividual->GetParent()->GetRng(), infectiousness );
        float infectiousness_age_scaled = infectiousness * SusceptibilityVector::SurfaceAreaBitingFunction( age );
        patient->infectiousness.push_back( infectiousness );
        patient->infectiousness_smeared.push_back( infectiousness_smeared );
        patient->infectiousness_age_scaled.push_back( infectiousness_age_scaled );

        // True values in model
        patient->true_asexual_density.push_back( susceptibility_malaria->get_parasite_density() );
        patient->true_gametocyte_density.push_back( individual_malaria->GetGametocyteDensity() );

        // Smeared true values in model
        patient->smeared_true_asexual_density.push_back( ReportUtilitiesMalaria::NASBADensityWithUncertainty( iindividual->GetParent()->GetRng(), susceptibility_malaria->get_parasite_density()));
        patient->smeared_true_gametocyte_density.push_back( ReportUtilitiesMalaria::NASBADensityWithUncertainty( iindividual->GetParent()->GetRng(), individual_malaria->GetGametocyteDensity()));

        // Values incorporating variability and sensitivity of blood test
        patient->asexual_parasite_density.push_back( individual_malaria->CheckParasiteCountWithTest( MALARIA_TEST_BLOOD_SMEAR ) );
        patient->gametocyte_density.push_back( individual_malaria->CheckGametocyteCountWithTest( MALARIA_TEST_BLOOD_SMEAR ) );
        patient->rdt.push_back( individual_malaria->CheckParasiteCountWithTest( MALARIA_TEST_NEW_DIAGNOSTIC ) );

        // Positive fields of view (out of 200 views in Garki-like setup)
        int positive_asexual_fields = 0;
        int positive_gametocyte_fields = 0;
        individual_malaria->CountPositiveSlideFields( iindividual->GetParent()->GetRng(), 200, (float)(1.0/400.0), positive_asexual_fields, positive_gametocyte_fields);
        patient->pos_asexual_fields.push_back( (float)positive_asexual_fields );
        patient->pos_gametocyte_fields.push_back( (float)positive_gametocyte_fields );
        LOG_DEBUG_F("(a,g) = (%d,%d)\n", (int)positive_asexual_fields, (int)positive_gametocyte_fields);

        // Values incorporating variability and sensitivity of blood test with uncertainty
        patient->smeared_asexual_parasite_density.push_back(ReportUtilitiesMalaria::FieldsOfViewToDensity((float)positive_asexual_fields));
        patient->smeared_gametocyte_density.push_back(ReportUtilitiesMalaria::FieldsOfViewToDensity((float)positive_gametocyte_fields));

        return true;
    }

    void MalariaSurveyJSONAnalyzer::SerializeOutput( float currentTime, json::Object& root )
    {
        json::QuickBuilder msja_json(root);
        msja_json["ntsteps"] = json::Number(m_reporting_interval);

        release_assert( m_pPatientMap );
        m_pPatientMap->Serialize( root );
    }
}
