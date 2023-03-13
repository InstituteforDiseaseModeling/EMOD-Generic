/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ConfigParams.h"
#include <iomanip>
#include <stdio.h>
#include "Climate.h"
#include "ClimateKoppen.h"
#include "ClimateByData.h"
#include "ClimateConstant.h"
#include "Common.h"
#include "Debug.h"
#include "Environment.h"
#include "FileSystem.h"
#include "Exceptions.h"
#include "Log.h"
#include "RANDOM.h"
#include "ISimulationContext.h"
#include "INodeContext.h"
#include "IdmDateTime.h"

using namespace std;
using namespace json;

SETUP_LOGGING( "Climate" )

// Used with ParseMetadataForFile
#define METADATA            "Metadata"
#define ID_REFERENCE        "IdReference"
#define UPDATE_RESOLUTION   "UpdateResolution"
#define DATAVALUE_COUNT     "DatavalueCount"
#define SCHEMA_VERSION      "WeatherSchemaVersion"
#define CELL_COUNT          "WeatherCellCount"
#define NODE_COUNT          "NodeCount"
#define DTK_NODE_COUNT      "NumberDTKNodes"
#define NODE_OFFSETS        "NodeOffsets"

namespace Kernel {

    const float Climate::min_airtemp  = -55;      // Celsius
    const float Climate::max_airtemp  =  45;      // Celsius
    const float Climate::min_landtemp = -55;      // Celsius
    const float Climate::max_landtemp =  60;      // Celsius
    const float Climate::max_rainfall =  0.150F;  // meters/day

    Climate::Climate(ClimateUpdateResolution::Enum update_resolution, INodeContext * _parent)
        : m_airtemperature(-FLT_MAX)
        , m_landtemperature(-FLT_MAX)
        , m_accumulated_rainfall(FLT_MAX)
        , m_humidity(-FLT_MAX)
        , resolution_correction(0.0f)
        , parent(_parent)
    {
        switch(update_resolution)
        {
            case ClimateUpdateResolution::CLIMATE_UPDATE_YEAR:       resolution_correction = 1.0f / DAYSPERYEAR; break;
            case ClimateUpdateResolution::CLIMATE_UPDATE_MONTH:      resolution_correction = 1.0f / IDEALDAYSPERMONTH; break;
            case ClimateUpdateResolution::CLIMATE_UPDATE_WEEK:       resolution_correction = 1.0f / DAYSPERWEEK; break;
            case ClimateUpdateResolution::CLIMATE_UPDATE_DAY:        resolution_correction = 1.0f; break;
            case ClimateUpdateResolution::CLIMATE_UPDATE_HOUR:       resolution_correction = HOURSPERDAY; break;
            default:                                                 resolution_correction = 0.0f; break;
        }
    }

    void Climate::UpdateWeather( float time, float dt, RANDOMBASE* pRNG )
    {
        if(ClimateConfig::GetClimateParams()->enable_climate_stochasticity)
        {
            AddStochasticity( pRNG );
        }

        // cap values to within physically-possible bounds
        if(m_humidity > 1)
            m_humidity = 1;
        else if(m_humidity < 0)
            m_humidity = 0;

        if(m_accumulated_rainfall < 0)
            m_accumulated_rainfall = 0;
    }

    void Climate::AddStochasticity( RANDOMBASE* pRNG )
    {
        const ClimateParams* cp = ClimateConfig::GetClimateParams();

        // air-temp
        if(cp->airtemperature_variance != 0.0)
            m_airtemperature += float( pRNG->eGauss() * cp->airtemperature_variance ); // varies as a Gaussian with stdev as specified in degree C

        // land-temp
        if(cp->landtemperature_variance != 0.0)
            m_landtemperature += float( pRNG->eGauss() * cp->landtemperature_variance ); // varies as a Gaussian with stdev as specified in degree C

        //rainfall
        if(cp->rainfall_variance_enabled)
            if(m_accumulated_rainfall > 0.0)
                m_accumulated_rainfall = float( pRNG->expdist(1.0 / m_accumulated_rainfall) ); // varies over exponential distribution with mean of calculated rainfall value

        // humidity
        if(cp->humidity_variance != 0.0)
            m_humidity += float( pRNG->eGauss() * cp->humidity_variance ); // varies as a Gaussian with stdev as specified in %
    }

    ClimateFactory* ClimateFactory::CreateClimateFactory(const string idreference, ISimulationContext* parent_sim)
    {
        ClimateFactory* factory = _new_ ClimateFactory(parent_sim);
        if(!factory->Initialize(idreference))
        {
            delete factory;
            factory = nullptr;
        }

        return factory;
    }

    ClimateFactory::ClimateFactory(ISimulationContext* parent_sim)
        : num_datavalues(0)
        , num_nodes(0)
        , num_badnodes(0)
        , parent(parent_sim)
    { }

    const ClimateParams* ClimateFactory::GetParams()
    {
        return ClimateConfig::GetClimateParams();
    }

    bool ClimateFactory::Initialize(const string idreference)
    {
        LOG_INFO( "Initialize\n" );

        const ClimateParams* cp = GetParams();

        try
        {
            if(cp->climate_structure == ClimateStructure::CLIMATE_OFF)
                return true;

            // prepare any input files, etc

            switch( cp->climate_structure )
            {
                case ClimateStructure::CLIMATE_CONSTANT:
                // nothing to do here...
                break;

                case ClimateStructure::CLIMATE_KOPPEN:
                {
                num_nodes = -1;

                if( cp->climate_koppen_filename == "" )
                {
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Climate_Model", "ClimateStructure::CLIMATE_KOPPEN:", "climate_koppen_filename", "<empty>" );
                }
                std::string koppen_filepath = Environment::FindFileOnPath( cp->climate_koppen_filename  );
                ParseMetadataForFile(koppen_filepath, idreference, nullptr, &num_nodes, koppentype_offsets);

                if(!OpenClimateFile(koppen_filepath, num_nodes * sizeof(int), climate_koppentype_file))
                    return false;

                num_datavalues = 1;
                }
                break;

                case ClimateStructure::CLIMATE_BY_DATA:
                {
                // Parse metadata for all input files

                num_datavalues = -1;

                // num_nodes = -1;
                // We no longer require climate files to have identical structure nor are they
                // required to have unique entries for each node (i.e. multiple simulation nodes
                // may utilize the same data if, e.g., the simulation node size is smaller than the
                // resolution of the climate cell(s).
                int32_t num_airtemp_entries = -1;
                int32_t num_landtemp_entries = -1;
                int32_t num_rainfall_entries = -1;
                int32_t num_humidity_entries = -1;

                if( cp->climate_airtemperature_filename == "" )
                {
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Climate_Model", "ClimateStructure::CLIMATE_BY_DATA:", "climate_airtemperature_filename", "<empty>" );
                }
                std::string airtemp_filepath = Environment::FindFileOnPath( cp->climate_airtemperature_filename );
                ParseMetadataForFile(airtemp_filepath, idreference, &num_datavalues, &num_airtemp_entries, airtemperature_offsets);

                if( cp->climate_landtemperature_filename == "" )
                {
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Climate_Model", "ClimateStructure::CLIMATE_BY_DATA:", "climate_landtemperature_filename", "<empty>" );
                }
                std::string landtemp_filepath = Environment::FindFileOnPath( cp->climate_landtemperature_filename );
                ParseMetadataForFile(landtemp_filepath, idreference, &num_datavalues, &num_landtemp_entries, landtemperature_offsets);

                if( cp->climate_rainfall_filename == "" )
                {
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Climate_Model", "ClimateStructure::CLIMATE_BY_DATA:", "climate_rainfall_filename", "<empty>" );
                }
                std::string rainfall_filepath = Environment::FindFileOnPath( cp->climate_rainfall_filename );
                ParseMetadataForFile(rainfall_filepath, idreference, &num_datavalues, &num_rainfall_entries, rainfall_offsets);

                if( cp->climate_relativehumidity_filename == "" )
                {
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Climate_Model", "ClimateStructure::CLIMATE_BY_DATA:", "climate_relativehumidity_filename", "<empty>" );
                }
                std::string humidity_filepath = Environment::FindFileOnPath( cp->climate_relativehumidity_filename );
                ParseMetadataForFile(humidity_filepath, idreference, &num_datavalues, &num_humidity_entries, humidity_offsets);

                // open all input files

                if(!OpenClimateFile(airtemp_filepath, num_datavalues * num_airtemp_entries * sizeof(float), climate_airtemperature_file))
                    return false;
                if(!OpenClimateFile(landtemp_filepath, num_datavalues * num_landtemp_entries * sizeof(float), climate_landtemperature_file))
                    return false;
                if(!OpenClimateFile(rainfall_filepath, num_datavalues * num_rainfall_entries * sizeof(float), climate_rainfall_file))
                    return false;
                if(!OpenClimateFile(humidity_filepath, num_datavalues * num_humidity_entries * sizeof(float), climate_humidity_file))
                    return false;
                }
                break;

                default:
                    throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "Climate_Model", cp->climate_structure, ClimateStructure::pairs::lookup_key( cp->climate_structure ) );
            }
        }
        catch (Exception &e)
        {
            // ERROR: "Exception during climate initialization:\n");
            // ERROR: ("%s\n", e.what());
            throw InitializationException( __FILE__, __LINE__, __FUNCTION__, e.what() );
        }

        return true;
    }

    int ReadIntegerFromConfig( const json::QuickInterpreter& json, const char* key, const string& filename )
    {
        int value = 0;

        if ( json.Exist(key) )
        {
            try
            {
                value = int( json[key].As<json::Number>() );
            }
            catch (json::Exception&)
            {
                std::ostringstream msg;
                msg << "Value for key '" << key << "' in file '" << filename.c_str() << "' should be numeric.";
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
        }
        else
        {
            std::ostringstream msg;
            msg << "Key \"" << key << "\" not found in file '" << filename.c_str() << "'" << std::endl;
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        return value;
    }

    string ReadStringFromConfig( const json::QuickInterpreter& json, const char* key, const string& filename )
    {
        string value;

        if ( json.Exist(key) )
        {
            try
            {
                value = json[key].As<json::String>();
            }
            catch (json::Exception&)
            {
                std::ostringstream msg;
                msg << "Value for key '" << key << "' in file '" << filename.c_str() << "' should be a string.";
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
        }
        else
        {
            std::ostringstream msg;
            msg << "Key \"" << key << "\" not found in file '" << filename.c_str() << "'" << std::endl;
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        return value;
    }

    bool ClimateFactory::ParseMetadataForFile(
        string data_filepath,
        string idreference,
        int * const pNumDatavalues,
        int * const pNumEntries,
        std::unordered_map<uint32_t, uint32_t> &node_offsets
    )
    {
        LOG_DEBUG_F( "%s: %s\n", __FUNCTION__, data_filepath.c_str() );
        release_assert(pNumEntries);

        string metadata_filepath = data_filepath + ".json";

        Configuration* config = Configuration::Load(metadata_filepath);

        if (config == nullptr)
        {
            throw FileIOException( __FILE__, __LINE__, __FUNCTION__, metadata_filepath.c_str() );
        }

        auto metadata = (*config)[METADATA];

        string json_id_reference( ReadStringFromConfig( metadata, ID_REFERENCE, metadata_filepath ) );
        string idreference_lower(idreference);  // Make a copy to transform so we do not modify the original.
        std::transform(idreference_lower.begin(), idreference_lower.end(), idreference_lower.begin(), ::tolower);
        std::transform(json_id_reference.begin(), json_id_reference.end(), json_id_reference.begin(), ::tolower);
        if (json_id_reference != idreference_lower)
        {
            std::ostringstream msg;
            msg << "IdReference used to generate climate file " << data_filepath << " doesn't match the IdReference used for the demographics" << std::endl;
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        if(GetParams()->climate_structure != ClimateStructure::CLIMATE_KOPPEN)
        {
            string str_clim_res( ReadStringFromConfig( metadata, UPDATE_RESOLUTION, metadata_filepath ));
            int md_updateres = ClimateUpdateResolution::pairs::lookup_value(str_clim_res.c_str());

            if(md_updateres == -1 || (GetParams()->climate_update_resolution != ClimateUpdateResolution::Enum(md_updateres)))
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Climate_Update_Resolution", 
                                                        ClimateUpdateResolution::pairs::lookup_key(GetParams()->climate_update_resolution).c_str(),
                                                        (std::string("metadata from ") + metadata_filepath).c_str(), str_clim_res.c_str() );
            }
        }

        if(pNumDatavalues != nullptr)
        {
            int md_datavalues = ReadIntegerFromConfig( metadata, DATAVALUE_COUNT, metadata_filepath );

            if(*pNumDatavalues == -1)
                *pNumDatavalues = md_datavalues;
            else if(*pNumDatavalues != md_datavalues)
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "num_datavalues", *pNumDatavalues, "md_datavalues", md_datavalues );
            }
        }

        // Look for "Metadata" : { "WeatherSchemaVersion" } key. If present, use "WeatherCellCount" for md_num_entries
        // and "NumberDTKNodes" for md_num_offsets.

        int md_num_entries = -1;
        int md_num_offsets = -1;

        if ( (*config)[METADATA].Exist(SCHEMA_VERSION) )
        {
            std::string schema_version( ReadStringFromConfig( metadata, SCHEMA_VERSION, metadata_filepath ) );
            if ( schema_version == "2.0" )
            {
                LOG_INFO( "Found 'WeatherSchemaVersion' \"2.0\" in climate file metadata. Using 'WeatherCellCount' and 'NumberDTKNodes'\n" );
                md_num_entries = ReadIntegerFromConfig( metadata, CELL_COUNT, metadata_filepath );
                md_num_offsets = ReadIntegerFromConfig( metadata, DTK_NODE_COUNT, metadata_filepath );
            }
            else
            {
                // Could use GeneralConfigurationException here, perhaps.
                std::ostringstream msg;
                msg << "Unsupported 'WeatherSchemaVersion': " << schema_version;
                throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
        }
        else // Fallback to "original" behavior.
        {
            LOG_WARN( "No 'WeatherSchemaVersion' found in climate file metadata. Falling back to 'NodeCount' and 'NumberDTKNodes'\n" );
            md_num_entries = ReadIntegerFromConfig( metadata, NODE_COUNT, metadata_filepath );
            md_num_offsets = md_num_entries;

            // "Slim" climate files map multiple DTK nodes to a single climate cell (node).
            // "NodeCount" gives the number of climate cells (nodes).
            // NumberDTKNodes (if present) gives the number of entries in the NodeOffsets string.
            // Could use try/catch here, but lacking the NumberDTKNodes (DTK_NODE_COUNT) key is allowed (i.e. not an exception).
            if ((*config)[METADATA].Exist(DTK_NODE_COUNT)) {
                md_num_offsets = ReadIntegerFromConfig( metadata, DTK_NODE_COUNT, metadata_filepath );
            }
        }

        if (*pNumEntries == -1)
        {
            *pNumEntries = md_num_entries;
        }

        string offsets_str = ReadStringFromConfig( *config, NODE_OFFSETS, metadata_filepath );
        if ( offsets_str.length() / 16 < md_num_offsets)
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "offsets_str.length() / 16", int( offsets_str.length() / 16 ), "*md_num_offsets", md_num_offsets);
        }

        uint32_t nodeid = 0, offset = 0;

        for(int n = 0; n < md_num_offsets; n++)
        {
#ifdef _MSC_VER
            sscanf_s(offsets_str.substr(n * 16, 8).c_str(), "%x", &nodeid);
            sscanf_s(offsets_str.substr((n * 16) + 8, 8).c_str(), "%x", &offset);
#else
            sscanf(offsets_str.substr(n * 16, 8).c_str(), "%x", &nodeid);
            sscanf(offsets_str.substr((n * 16) + 8, 8).c_str(), "%x", &offset);
#endif
            node_offsets[nodeid] = offset;
        }

        delete config;
        return true;
    }

    bool ClimateFactory::OpenClimateFile(string filepath, uint32_t expected_size, std::ifstream &file)
    {
        FileSystem::OpenFileForReading( file, filepath.c_str(), true );

        // "Slim" climate files point several nodes to the same data, thus the size may be less than
        // expected_size (generally num_datavalues * num_nodes * sizeof(float)).
        file.seekg(0, ios::end);
        int filelen = (int)file.tellg();

        if(filelen != expected_size)
        {
            ostringstream msg;
            msg << "Expected climate file '"
                << filepath.c_str()
                << "' to be "
                << expected_size << " bytes but is actually "
                << filelen << " bytes long.";
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
        }

        file.seekg(0, ios::beg);

        return true;
    }

    Climate* ClimateFactory::CreateClimate( INodeContext *parent_node, float altitude, float latitude, RANDOMBASE* pRNG )
    {
        LOG_DEBUG( "CreateClimate\n" );
        Climate* new_climate = nullptr;

        release_assert(parent_node);

        float start_time = parent->GetParams()->sim_time_start;
        uint32_t nodeid  = parent_node->GetExternalID();

        LOG_DEBUG_F( "Processing nodeid %d\n", nodeid );

        switch(GetParams()->climate_structure )
        {
            case ClimateStructure::CLIMATE_CONSTANT:
                new_climate = ClimateConstant::CreateClimate( ClimateUpdateResolution::CLIMATE_UPDATE_DAY, parent_node, start_time, pRNG );
                break;

            case ClimateStructure::CLIMATE_KOPPEN:
            {
                if(koppentype_offsets.count(nodeid) == 0)
                {
                    throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "(koppentype_offsets.count(nodeid)", 0, "nodeid", nodeid );
                }

                // seek to where we expect the data for that node
                climate_koppentype_file.seekg(koppentype_offsets[nodeid], std::ios::beg);

                // now read in koppen-type of climate
                int koppen_type;
                climate_koppentype_file.read((char *)(&koppen_type), sizeof(koppen_type));

                new_climate = ClimateKoppen::CreateClimate( ClimateUpdateResolution::CLIMATE_UPDATE_MONTH,
                                                            parent_node,
                                                            koppen_type,
                                                            altitude,
                                                            latitude,
                                                            start_time,
                                                            pRNG );
            }
            break;

            case ClimateStructure::CLIMATE_BY_DATA:
            {
                if(landtemperature_offsets.count(nodeid) == 0 ||
                    airtemperature_offsets.count(nodeid) == 0 ||
                    rainfall_offsets.count(nodeid) == 0 ||
                    humidity_offsets.count(nodeid) == 0)
                {
                    //std::cerr << "Error: Couldn't find offset for NodeID " << nodeid << " in ClimateByData files" << endl;
                    LOG_INFO_F( "landtemperature_offsets.count(nodeid) = %d, airtemperature_offsets.count(nodeid) = %d, rainfall_offsets.count(nodeid) = %d, humidity_offsets.count(nodeid) = %d\n",
                                landtemperature_offsets.count(nodeid), airtemperature_offsets.count(nodeid), rainfall_offsets.count(nodeid), humidity_offsets.count(nodeid)
                              );
                    ostringstream msg;
                    msg << "Didn't find data for demographics node "
                        << nodeid
                        << " ("
                        << std::hex << std::uppercase << std::setfill('0') << setw(8) << nodeid << std::dec
                        << ") in the following file(s):"
                        << endl;
                    if (airtemperature_offsets.count(nodeid) == 0)  { msg << "\tair temperature" << endl; }
                    if (landtemperature_offsets.count(nodeid) == 0) { msg << "\tland temperature" << endl; }
                    if (rainfall_offsets.count(nodeid) == 0)        { msg << "\trainfall" << endl; }
                    if (humidity_offsets.count(nodeid) == 0)        { msg << "\trelative humidity" << endl; }
                    throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
                }

                climate_airtemperature_file.seekg(airtemperature_offsets[nodeid], std::ios::beg);
                climate_landtemperature_file.seekg(landtemperature_offsets[nodeid], std::ios::beg);
                climate_rainfall_file.seekg(rainfall_offsets[nodeid], std::ios::beg);
                climate_humidity_file.seekg(humidity_offsets[nodeid], std::ios::beg);

                new_climate = ClimateByData::CreateClimate( GetParams()->climate_update_resolution,
                                                            parent_node,
                                                            num_datavalues,
                                                            climate_airtemperature_file,
                                                            climate_landtemperature_file,
                                                            climate_rainfall_file,
                                                            climate_humidity_file,
                                                            start_time,
                                                            pRNG );
            }
            break;

            default:
            {
            }
        }

        if(new_climate != nullptr && !new_climate->IsPlausible())
            num_badnodes++;

        return new_climate;
    }

    ClimateFactory::~ClimateFactory()
    {
        if(num_badnodes > 0)
            LOG_WARN_F("WARNING: Detected %d nodes with suspicious climate values\n", num_badnodes); 

        if (climate_airtemperature_file.is_open())
            climate_airtemperature_file.close();
        if (climate_landtemperature_file.is_open())
            climate_landtemperature_file.close();
        if (climate_rainfall_file.is_open())
            climate_rainfall_file.close();
        if (climate_humidity_file.is_open())
            climate_humidity_file.close();
        if (climate_koppentype_file.is_open())
            climate_koppentype_file.close();
    }


    Climate::~Climate() { }

    void
    Climate::SetContextTo(INodeContext* _parent) { parent = _parent; }
}
