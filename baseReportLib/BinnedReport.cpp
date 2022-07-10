/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "BinnedReport.h"

#include <map>
#include <string>
#include <ctime>
#include <cmath>

#include "Debug.h"
#include "Environment.h"
#include "FileSystem.h"
#include "Exceptions.h"
#include "Sugar.h"
#include "IIndividualHuman.h"
#include "ProgVersion.h"

using namespace std;

SETUP_LOGGING( "BinnedReport" )

static const std::string _report_name = "BinnedReport.json";

static const char* _axis_labels[] = { "Age" };

namespace Kernel {

const char * BinnedReport::_pop_label = "Population";
const char * BinnedReport::_infected_label = "Infected";
const char * BinnedReport::_new_infections_label = "New Infections";

Kernel::IReport*
BinnedReport::CreateReport()
{
    return new BinnedReport();
}

BinnedReport::BinnedReport()
: BinnedReport( _report_name )
{
}

// Derived constructor calls base constructor to initialized reduced timesteps etc. 
BinnedReport::BinnedReport( const std::string& rReportName )
    : BaseChannelReport( rReportName )
    , num_timesteps(0)
    , num_axes(0)
    , axis_labels()
    , num_bins_per_axis()
    , num_total_bins(0)
    , values_per_axis()
//    , friendly_names_per_axis()
    , population_bins(nullptr)
    , infected_bins(nullptr)
    , new_infections_bins(nullptr)
    , p_output_augmentor(nullptr)
    , _age_bin_upper_values(nullptr)
{
    LOG_DEBUG( "BinnedReport ctor\n" );

    // These __ variables exist for super-easy intialization/specification by humans and don't persist past the ctor.
    // We don't want these as static consts outside the class, but ultimately as members so that each sim type can define their own age boundaries.
    
    float __age_bin_upper_values[] = { 1825.0,  3650.0,  5475.0,  7300.0,  9125.0, 10950.0, 12775.0, 14600.0, 16425.0, 18250.0, 20075.0, 21900.0, 23725.0, 25550.0, 27375.0, 29200.0, 31025.0, 32850.0, 34675.0, 36500.0, 999999.0 };
    char * __age_bin_friendly_names[] = { "<5",   "5-9", "10-14", "15-19", "20-24", "25-29", "30-34", "35-39", "40-44", "45-49", "50-54", "55-59", "60-64", "65-69", "70-74", "75-79", "80-84", "85-89", "90-94", "95-99", ">100" };
    _num_age_bins = sizeof( __age_bin_upper_values )/sizeof(float); 

    // Now let's actually initialize the single underscore vector variables we're going to use (the "tedious" way)
    // NOTE: 100 picked as "hopefully we won't need any bigger than this"

    _age_bin_friendly_names.resize( _num_age_bins );
    _age_bin_upper_values = new float[100];
    memset( _age_bin_upper_values, 0, sizeof( float ) * 100 );
    // It can be fun to use 1-line STL initializers, but sometimes readability is more important
    for( int idx = 0; idx < _num_age_bins ; idx++ )
    {
        _age_bin_upper_values[idx] = __age_bin_upper_values[idx];
        _age_bin_friendly_names[idx] = __age_bin_friendly_names[idx];
    }
    
}

BinnedReport::~BinnedReport()
{
    delete[] population_bins;
    delete[] infected_bins;
    delete[] new_infections_bins;
    //p_output_augmentor - do not delete since this class does not own it
}

#ifndef WIN32
#define _countof(a) (sizeof(a)/sizeof(*(a)))
#endif
void BinnedReport::Initialize( unsigned int nrmSize )
{
    for( unsigned int idx=0; idx<sizeof(_num_bins_per_axis)/sizeof(int); idx++ )
    {
        _num_bins_per_axis[idx] = _num_age_bins;
    }
    
    num_timesteps = 0;
    _nrmSize = nrmSize;
    release_assert( _nrmSize );

    // wish we could just use C++11 initializer lists here, but alas... not yet implemented :(
    axis_labels = std::vector<std::string>(_axis_labels, _axis_labels + (sizeof(_axis_labels) / sizeof(char*)));
    num_bins_per_axis = std::vector<int>(_num_bins_per_axis, _num_bins_per_axis + (sizeof(_num_bins_per_axis) / sizeof(int)));

    num_axes = _countof(_axis_labels);

    num_total_bins = 1;
    for (int i : num_bins_per_axis)
        num_total_bins *= i;

    values_per_axis.resize( num_axes );
    for( int axis_idx=0; axis_idx < num_axes; axis_idx++ )
    {
        for( int idx=0; idx< num_bins_per_axis[axis_idx]; idx++ )
        {
            values_per_axis[axis_idx].push_back( _age_bin_upper_values[idx] );
        }
    }


    initChannelBins();
}

void BinnedReport::initChannelBins()
{
    LOG_DEBUG_F( "num_total_bins = %d\n", num_total_bins );
    population_bins     = new float[num_total_bins];
    infected_bins       = new float[num_total_bins];
    new_infections_bins = new float[num_total_bins];
    
    clearChannelsBins();
}

void BinnedReport::clearChannelsBins()
{
    memset(population_bins    , 0, num_total_bins * sizeof(float));
    memset(infected_bins      , 0, num_total_bins * sizeof(float));
    memset(new_infections_bins, 0, num_total_bins * sizeof(float));
}

void BinnedReport::BeginTimestep()
{
    channelDataMap.IncreaseChannelLength( num_total_bins );

    num_timesteps++;
}

void BinnedReport::EndTimestep( float currentTime, float dt )
{
    Accumulate(_pop_label, population_bins);
    Accumulate(_infected_label, infected_bins);
    Accumulate(_new_infections_label, new_infections_bins);
    
    clearChannelsBins();
}

void BinnedReport::Accumulate(std::string channel_name, float bin_data[])
{
    for(int i = 0; i < num_total_bins; i++)
    {
        // NOTE: We have to call Accumulate() here even if there's nothing to accumulate, because 
        // we need to make sure the channels are added.  Could potentially check outside the for-loop
        // on timestep 0, and add a channel that isn't there...?
        // if(bin_data[i] > 0)
            Accumulate(channel_name, bin_data[i], i);
    }
}

void BinnedReport::Accumulate(std::string channel_name, float value, int bin_index)
{
    if ( bin_index < 0 || bin_index >= num_total_bins)
    {
        throw Kernel::OutOfRangeException( __FILE__, __LINE__, __FUNCTION__, "bin_index", bin_index, num_total_bins );
    }

    // initialize the vectors if this is the first time. assume we are at t=0 and the map has no keys.
    if( !channelDataMap.HasChannel( channel_name ) )
    {
        channelDataMap.IncreaseChannelLength( channel_name, num_total_bins );
    }
    int length = channelDataMap.GetChannelLength() ;
    if( length == 0) 
    {
        channelDataMap.IncreaseChannelLength( num_total_bins );
        length = channelDataMap.GetChannelLength() ;
    }

    // accumulate value at last timestep for the appropriate bin_index
    int index = length - num_total_bins + bin_index ;
    channelDataMap.Accumulate( channel_name, index, value );
}

void BinnedReport::LogNodeData( Kernel::INodeContext * pNC )
{
    // Nothing in this function.  All the updates will be done at the individual level in LogIndividualData(IndividualHuman*)
    LOG_DEBUG( "LogNodeData.\n" );
}

int BinnedReport::calcBinIndex( Kernel::IIndividualHuman* individual)
{
    float age = float(individual->GetAge());
    //bool isFemale      = (individual->GetGender() == FEMALE);

    // Calculate bin
    int agebin = lower_bound( values_per_axis[0].begin(), values_per_axis[0].end(), age ) - values_per_axis[0].begin();
    //int bin_index = ( age_bin_upper_edges.size() * isFemale ) + agebin;
    int bin_index = agebin;

    release_assert( bin_index < num_total_bins );
    return bin_index;
}

void  BinnedReport::LogIndividualData( Kernel::IIndividualHuman* individual )
{
    LOG_DEBUG( "LogIndividualData\n" );

    float mc_weight = float(individual->GetMonteCarloWeight());

    int bin_index = calcBinIndex(individual);

    population_bins[bin_index] += mc_weight;

    if (individual->IsInfected())
    {
        infected_bins[bin_index] += mc_weight;
    }

    NewInfectionState::_enum nis = individual->GetNewInfectionState();

    if(nis == NewInfectionState::NewAndDetected || nis == NewInfectionState::NewInfection)
    {
        new_infections_bins[bin_index] += mc_weight;
    }
}

void BinnedReport::Finalize()
{
    LOG_DEBUG( "Finalize\n" );

    postProcessAccumulatedData();

    std::map<std::string, std::string> units_map;
    populateSummaryDataUnitsMap(units_map);

    // Add some header stuff to InsetChart.json.
    // { "Header":
    //   { "DateTime" },
    //   { "DTK_Version" },
    //   { "Report_Version" },
    //   { "Timesteps" },      # of timestamps in data
    //   { "Channels" }        # of channels in data
    // }
    time_t now = time(nullptr);
#ifdef WIN32
    tm now2;
    localtime_s(&now2,&now);
    char timebuf[26];
    asctime_s(timebuf,26,&now2);
    std::string now3 = std::string(timebuf);
#else
    tm* now2 = localtime(&now);
    std::string now3 = std::string(asctime(now2));
#endif

    ProgDllVersion pv;
    ostringstream dtk_ver;
    dtk_ver << pv.getRevisionNumber() << " " << pv.getSccsBranch() << " " << pv.getBuildDate();

    // Document root
    json::Object       obj_root;
    json::QuickBuilder json_doc(obj_root);

    // "Header": {}
    json::Object       obj_header;
    json::QuickBuilder json_header(obj_header);

    json_header["DateTime"]       = json::String(now3.substr(0,now3.length()-1).c_str());
    json_header["DTK_Version"]    = json::String(dtk_ver.str().c_str());
    json_header["Report_Version"] = json::String("2.1");

    unsigned int timesteps = 0;
    if( !channelDataMap.IsEmpty() && num_total_bins > 0 )
    {
        timesteps = int(double(channelDataMap.GetChannelLength()) / double(num_total_bins));
    }
    json_header["Timesteps"]      = json::Number((int)timesteps);

    if( p_output_augmentor != nullptr )
    {
        p_output_augmentor->AddDataToHeader( obj_header );
    }

    // "Subchannel Metadata": {}
    json::Object       obj_meta;
    json::QuickBuilder json_meta(obj_meta);

    // "AxisLabels": [[data]]
    json::Array arr_labels1, arr_labels2;
    for(size_t k1 = 0; k1 < axis_labels.size(); k1++)
    {
        arr_labels2.Insert(json::String(axis_labels[k1]));
    }
    arr_labels1.Insert(arr_labels2);
    json_meta["AxisLabels"] = arr_labels1;

    // "NumBinsPerAxis": [[data]]
    json::Array        arr_numbin1, arr_numbin2;
    for(size_t k1 = 0; k1 < num_bins_per_axis.size(); k1++)
    {
        arr_numbin2.Insert(json::Number(num_bins_per_axis[k1]));
    }
    arr_numbin1.Insert(arr_numbin2);
    json_meta["NumBinsPerAxis"] = arr_numbin1;

    // "ValuesPerAxis": [[[data1],[data2],...]]
    json::Array        arr_values1, arr_values2;
    for(size_t k1 = 0; k1 < values_per_axis.size(); k1++)
    {
        json::Array        arr_values3;
        for(size_t k2 = 0; k2 < values_per_axis[k1].size(); k2++)
        {
            arr_values3.Insert(json::Number(values_per_axis[k1][k2]));
        }
        arr_values2.Insert(arr_values3);
    }
    arr_values1.Insert(arr_values2);
    json_meta["ValuesPerAxis"]  = arr_values1;

    // "MeaningPerAxis": [[[data]]]
    json::Array        arr_explan1, arr_explan2, arr_explan3;
    for(size_t k1 = 0; k1 < _age_bin_friendly_names.size(); k1++)
    {
        arr_explan3.Insert(json::String(_age_bin_friendly_names[k1]));
    }
    arr_explan2.Insert(arr_explan3);
    arr_explan1.Insert(arr_explan2);
    json_meta["MeaningPerAxis"] = arr_explan1;

    json_header["Subchannel_Metadata"] = obj_meta;
    json_header["Channels"]            = json::Number((int)channelDataMap.GetNumChannels());

    json_doc["Header"] = obj_header;

    // "Channels": {}
    LOG_DEBUG("Iterating over channelDataMap\n");
    json::Object       obj_channels;
    json::QuickBuilder json_channels(obj_channels);

    // Iterate over channel entry
    for( auto& name : channelDataMap.GetChannelNames() )
    {
        const ChannelDataMap::channel_data_t& channel_data = channelDataMap.GetChannel( name );

        json::Object       obj_entry;
        json::QuickBuilder json_entry(obj_entry);

        // "Data": [[data1],[data2],...]
        json::Array arr_data;
        formatChannelDataBins(arr_data, channel_data.data(), num_bins_per_axis, 0, num_total_bins);

        json_entry["Units"] = json::String(units_map[name].c_str());
        json_entry["Data"]  = arr_data;

        json_channels[name] = obj_entry;
    }

    json_doc["Channels"] = obj_channels;

    // Write output to file
    LOG_DEBUG("Writing JSON output file\n");
    ofstream binned_report_json;
    FileSystem::OpenFileForWriting( binned_report_json, FileSystem::Concat( EnvPtr->OutputPath, report_name ).c_str() );
    json::Writer::Write( json_doc, binned_report_json );
    binned_report_json.close();
}

void BinnedReport::formatChannelDataBins(json::Array& root, const float data[], std::vector<int>& dims, int start_axis, int num_remaining_bins)
{
    if(start_axis < dims.size())
    {
        int num_bins = num_remaining_bins / dims[start_axis];
        for(int k1=0; k1<dims[start_axis]; k1++)
        {
            formatChannelDataBins(root, data + (k1 * num_bins), dims, start_axis+1, num_bins);
        }
    }
    else
    {
        json::Array arr_subarr;
        for(int k1=0; k1<num_timesteps; k1++)
        {
            ChannelDataMap::channel_data_t::value_type val = data[k1 * num_total_bins];
            if (std::isnan(val)) val = 0;   // Since NaN isn't part of the json standard, force all NaN values to zero
            arr_subarr.Insert(json::Number(val));
        }
        root.Insert(arr_subarr);
    }
}

void BinnedReport::populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map )
{
}

void BinnedReport::postProcessAccumulatedData()
{
}

}
