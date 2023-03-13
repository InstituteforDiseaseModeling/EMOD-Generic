/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "VectorHabitatReport.h"

#include <map>
#include <string>

#include "Debug.h"
#include "Environment.h"
#include "Exceptions.h"
#include "VectorContexts.h"
#include "VectorPopulation.h"
#include "Sugar.h"
#include "INodeContext.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"
#include "FactorySupport.h" // for DTK_DLLEXPORT

//******************************************************************************

#pragma warning(disable: 4996) // for suppressing strcpy caused security warnings

using namespace Kernel;

//******************************************************************************

SETUP_LOGGING( "VectorHabitatReport" )

static const char*       _sim_types[] = {"VECTOR_SIM", "MALARIA_SIM", nullptr};
static const std::string _report_name = "VectorHabitatReport.json";

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
    return new Kernel::VectorHabitatReport();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- VectorHabitatReport Methods
// ----------------------------------------

namespace Kernel {

VectorHabitatReport::VectorHabitatReport()
{
}

VectorHabitatReport::~VectorHabitatReport()
{
}

void VectorHabitatReport::Initialize( unsigned int nrmSize )
{
    LOG_DEBUG("Initialize\n");

    _nrmSize = nrmSize;
    release_assert( _nrmSize );

    report_name = _report_name;

    num_total_bins = 0;
}

void VectorHabitatReport::initChannelBins()
{
    // This is postponed until the first time through LogNodeData
    // at which point this reporter knows what species and habitat types
    // are being used in the simulation.

    LOG_DEBUG_F( "There are %d bins: \n", num_total_bins );

    axis_labels.push_back("Species:Habitat");
    num_axes = axis_labels.size();
    num_bins_per_axis.push_back(num_total_bins);
    values_per_axis.push_back(std::vector<float>(num_total_bins, 0.0f));
       
    std::ostringstream oss;
    std::vector<std::string> axis_names;
    for ( int i=0; i<num_total_bins; i++ )
    {
        for ( auto &habitat : species_habitat_idx_map )
        {
            if (habitat.second == i)
            {
                axis_names.push_back(habitat.first);
                oss << habitat.first << " ";
                break;
            }
        }
    }
    // friendly_names_per_axis.push_back(axis_names);
    _age_bin_friendly_names= std::vector<std::string>( axis_names.begin(), axis_names.end() );

    oss << endl;
    LOG_DEBUG(oss.str().c_str());
}

void VectorHabitatReport::clearChannelsBins()
{
    // NOTE: don't really need to do this if everything is being overwritten every timestep
    // If there is a possibility that any of the values would only be filled conditionally,
    // then we need to zero out the other values (one way or another).

    //for(int i = 0; i < num_total_bins; i++)
    //{
    //    current_habitat_capacity[i] = 0;
    //    total_larva[i] = 0;
    //    egg_crowding_factor[i] = 0;
    //    local_larval_mortality[i] = 0;
    //    artificial_larval_mortality[i] = 0;
    //    local_larval_growth_mod[i] = 0;
    //}
}

void VectorHabitatReport::Accumulate( const std::string& channel_name, const ChannelDataMap::channel_data_t& binned_data)
{
    for(int i = 0; i < binned_data.size(); i++)
    {
        // NOTE: We have to call Accumulate() here even if there's nothing to accumulate, because 
        // we need to make sure the channels are added.  Could potentially check outside the for-loop
        // on timestep 0, and add a channel that isn't there...?
        // if(bin_data[i] > 0)
        BinnedReport::Accumulate(channel_name, binned_data.at(i), i);
    }
}

void VectorHabitatReport::EndTimestep( float currentTime, float dt )
{
    Accumulate("Current Habitat Capacity", current_habitat_capacity);
    Accumulate("Total Larva", total_larva);
    Accumulate("Egg Crowding Factor", egg_crowding_factor);
    Accumulate("Local Larval Mortality", local_larval_mortality);
    Accumulate("Artificial Larval Mortality", artificial_larval_mortality);
    Accumulate("Rainfall Larval Mortality", rainfall_larval_mortality);
    Accumulate("Local Larval Growth Modifier", local_larval_growth_mod);

    std::vector<std::string> channel_names = channelDataMap.GetChannelNames();
    for( auto name : channel_names )
    {
        const ChannelDataMap::channel_data_t& r_channel_data = channelDataMap.GetChannel( name );
        LOG_DEBUG_F( "channelDataMap[%s].size() = %d\n", name.c_str(), r_channel_data.size() );
    }

    clearChannelsBins();
}

void VectorHabitatReport::LogIndividualData( IIndividualHuman* individual )
{
}

int VectorHabitatReport::calcBinIndex(const IIndividualHuman* individual)
{
    return 0;
}

void VectorHabitatReport::LogNodeData( INodeContext * pNC )
{
    LOG_DEBUG( "LogNodeData\n" );
    bool empty_idx_map = species_habitat_idx_map.empty();

    INodeVector* p_node_vector = pNC->GetNodeVector();
    release_assert(p_node_vector);

    const VectorPopulationReportingList_t& vectorPopulations = p_node_vector->GetVectorPopulationReporting();

    for ( auto vp : vectorPopulations )
    {
        std::string species_name = vp->get_SpeciesID();
        for ( auto hab : vp->GetHabitats() )
        {
            std::string habitat_name = p_node_vector->GetHabitatName( hab->GetVectorHabitatType() );
            std::string species_habitat = species_name + ':' + habitat_name;

            if (empty_idx_map)
            {
                species_habitat_idx_map[species_habitat] = num_total_bins++;

                LOG_DEBUG_F( "species_habitat = %s  num_total_bins = %d  capacity = %0.2f\n", species_habitat.c_str(), num_total_bins, float(hab->GetCurrentLarvalCapacity()) );

                current_habitat_capacity.push_back(    hab->GetCurrentLarvalCapacity()        );
                total_larva.push_back(                 hab->GetTotalLarvaCount(TimeStepIndex::CURRENT_TIME_STEP));
                egg_crowding_factor.push_back(         hab->GetEggCrowdingCorrection()        );
                local_larval_mortality.push_back(      hab->GetLocalLarvalMortality(1.0, 0.5) ); // NOTE: mortality relative to species baseline (1.0) and intermediate larval age (0.5)
                artificial_larval_mortality.push_back( hab->GetArtificialLarvalMortality()    );
                rainfall_larval_mortality.push_back(   hab->GetRainfallMortality()            );
                local_larval_growth_mod.push_back(     hab->GetLocalLarvalGrowthModifier()    );
            }
            else
            {
                LOG_DEBUG_F( "species_habitat = %s  capacity = %0.2f\n", species_habitat.c_str(), float(hab->GetCurrentLarvalCapacity()) );
                int idx = species_habitat_idx_map.at(species_habitat);

                current_habitat_capacity[idx]    = hab->GetCurrentLarvalCapacity();
                total_larva[idx]                 = hab->GetTotalLarvaCount(TimeStepIndex::CURRENT_TIME_STEP);
                egg_crowding_factor[idx]         = hab->GetEggCrowdingCorrection();
                local_larval_mortality[idx]      = hab->GetLocalLarvalMortality(1.0, 0.5); // NOTE: mortality relative to species baseline (1.0) and intermediate larval age (0.5)
                artificial_larval_mortality[idx] = hab->GetArtificialLarvalMortality();
                rainfall_larval_mortality[idx]   = hab->GetRainfallMortality();
                local_larval_growth_mod[idx]     = hab->GetLocalLarvalGrowthModifier();
            }
        }
    }

    if (empty_idx_map)
    {
        initChannelBins();
    }
}

}
