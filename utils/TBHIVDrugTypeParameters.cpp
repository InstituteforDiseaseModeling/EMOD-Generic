/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "TBDrugTypeParameters.h"
#include "TBHIVDrugTypeParameters.h"
#include "Exceptions.h"
#include "Log.h"

SETUP_LOGGING("TBHIVDTP")


namespace Kernel
{
    TBHIVDrugTypeParameters::TBHIVDrugTypeParameters(const std::string& tb_drug_name)
        : TBDrugTypeParameters(tb_drug_name)
        , TB_drug_inactivation_rate_mdr(0.0f)
        , TB_drug_inactivation_rate_hiv(0.0f)
        , TB_drug_cure_rate_mdr(0.0f)
        , TB_drug_cure_rate_hiv(0.0f)
        , TB_drug_resistance_rate_hiv(0.0f)
        , TB_drug_relapse_rate_mdr(0.0f)
        , TB_drug_relapse_rate_hiv(0.0f)
        , TB_drug_mortality_rate_mdr(0.0f)
        , TB_drug_mortality_rate_hiv(0.0f)
        , TB_reduced_acquire(0.0f)
        , TB_reduced_transmit(0.0f)
    {
        initConfigTypeMap( "TB_Drug_Inactivation_Rate_HIV", &TB_drug_inactivation_rate_hiv, TB_Drug_Inactivation_Rate_HIV_DESC_TEXT,  0.0f, 1.0f, 1.0f, "Enable_Coinfection" );
        initConfigTypeMap( "TB_Drug_Inactivation_Rate_MDR", &TB_drug_inactivation_rate_mdr, TB_Drug_Inactivation_Rate_MDR_DESC_TEXT,  0.0f, 1.0f, 1.0f, "Enable_Coinfection" );

        initConfigTypeMap( "TB_Drug_Cure_Rate_HIV",         &TB_drug_cure_rate_hiv,         TB_Drug_Cure_Rate_HIV_DESC_TEXT,          0.0f, 1.0f, 1.0f, "Enable_Coinfection" );
        initConfigTypeMap( "TB_Drug_Cure_Rate_MDR",         &TB_drug_cure_rate_mdr,         TB_Drug_Cure_Rate_MDR_DESC_TEXT,          0.0f, 1.0f, 1.0f, "Enable_Coinfection" );

        initConfigTypeMap( "TB_Drug_Resistance_Rate_HIV",   &TB_drug_resistance_rate_hiv,   TB_Drug_Resistance_Rate_HIV_DESC_TEXT,    0.0f, 1.0f, 0.0f, "Enable_Coinfection" );

        initConfigTypeMap( "TB_Drug_Relapse_Rate_MDR",      &TB_drug_relapse_rate_mdr,      TB_Drug_Relapse_Rate_MDR_DESC_TEXT,       0.0f, 1.0f, 0.0f, "Enable_Coinfection" );
        initConfigTypeMap( "TB_Drug_Relapse_Rate_HIV",      &TB_drug_relapse_rate_hiv,      TB_Drug_Relapse_Rate_HIV_DESC_TEXT,       0.0f, 1.0f, 0.0f, "Enable_Coinfection" );

        initConfigTypeMap( "TB_Drug_Mortality_Rate_MDR",    &TB_drug_mortality_rate_mdr,    TB_Drug_Mortality_Rate_MDR_DESC_TEXT,     0.0f, 1.0f, 0.0f, "Enable_Coinfection" );
        initConfigTypeMap( "TB_Drug_Mortality_Rate_HIV",    &TB_drug_mortality_rate_hiv,    TB_Drug_Mortality_Rate_HIV_DESC_TEXT,     0.0f, 1.0f, 0.0f, "Enable_Coinfection" );

        initConfigTypeMap( "TB_Reduced_Transmit",           &TB_reduced_transmit,           TB_Reduced_Transmit_TBHIV_DESC_TEXT,      0.0f, 1.0f, 0.0f, "Enable_Coinfection" );
        initConfigTypeMap( "TB_Reduced_Acquire",            &TB_reduced_acquire,            TB_Reduced_Acquire_TBHIV_DESC_TEXT,       0.0f, 1.0f, 0.0f, "Enable_Coinfection" );
    }

    TBHIVDrugTypeParameters::~TBHIVDrugTypeParameters()
    { }

    QueryResult TBHIVDrugTypeParameters::QueryInterface(iid_t iid, void **ppvObject)
    {
        throw NotYetImplementedException(  __FILE__, __LINE__, __FUNCTION__, "Should not get here" );
    }



    // ***** TBHIVDrugCollection is a container for 0 or more TBHIVDrugTypeParameters *****
    BEGIN_QUERY_INTERFACE_BODY(TBHIVDrugCollection)
    END_QUERY_INTERFACE_BODY(TBHIVDrugCollection)

    TBHIVDrugCollection::TBHIVDrugCollection()
        : JsonConfigurable()
        , tbhiv_drug_map()
    { }

    TBHIVDrugCollection::~TBHIVDrugCollection()
    { }

    json::QuickBuilder TBHIVDrugCollection::GetSchema()
    {
        std::string idm_type_schema    = "idmType:TBHIVDrugCollection";
        std::string object_schema_name = "<TBHIVDrugName>";

        TBHIVDrugTypeParameters tbhivdp(object_schema_name);

        json::QuickBuilder schema(GetSchemaBase());
        auto tn = JsonConfigurable::_typename_label();
        auto ts = JsonConfigurable::_typeschema_label();
        schema[tn] = json::String(idm_type_schema);
        schema[ts] = json::Object();
        schema[ts][object_schema_name] = tbhivdp.GetSchema().As<Object>();
        schema["default"] = json::Object();

        return schema;
    }

    void TBHIVDrugCollection::ConfigureFromJsonAndKey(const Configuration* inputJson, const std::string& key)
    {
        const auto& json_object = json_cast<const json::Object&>((*inputJson)[key]);

        for(auto obj_member = json_object.Begin(); obj_member != json_object.End(); ++obj_member)
        {
            std::string     tbhiv_drug_name = (*obj_member).name;
            Configuration*  p_config_params = Configuration::CopyFromElement((*obj_member).element, inputJson->GetDataLocation());

            tbhiv_drug_map[tbhiv_drug_name] = _new_ TBHIVDrugTypeParameters(tbhiv_drug_name);
            tbhiv_drug_map[tbhiv_drug_name]->Configure(p_config_params);

            delete p_config_params;
        }
    }

    size_t TBHIVDrugCollection::size() const
    {
        return tbhiv_drug_map.size();
    }

    TBHIVDrugTypeParameters* TBHIVDrugCollection::operator[](const std::string& drug_name) const
    {
        if(!tbhiv_drug_map.count(drug_name))
        {
            std::ostringstream msg;
            msg << "Drug name " << drug_name << " not in TBHIV_Drug_Params.";
            throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        return tbhiv_drug_map.at(drug_name);
    }
}
