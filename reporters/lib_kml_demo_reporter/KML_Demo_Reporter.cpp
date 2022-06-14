/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <stdio.h> // for sscanf
#include <cmath>

#include <fstream>
#include <iostream>

// Note: tarball.cpp/h files are moved into reporters directory
// and future reporter emodules(dlls) specific files should reside in there
#include "tarball.h"

#include "KML_Demo_Reporter.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"
#include "Sugar.h"
#include "Environment.h"
#include "INodeContext.h"
#include "IIndividualHuman.h"
#include "suids.hpp"
#include "IdmMpi.h"
#include "NoCrtWarnings.h"
#include "FileSystem.h"

//******************************************************************************

#define KML 1

#define MASTER_FILE "//bayesianfil01/IDM/home/dbridenbecker/regression_test_files/tajik_master.kml"

using namespace std;
using namespace json;

//******************************************************************************

SETUP_LOGGING( "KML_Demo_Reporter" )

static const char* _sim_types[]    = { "*", nullptr };
static const char* _stat_pop_label = "Statistical Population";
static const char* _infected_label = "Infected";

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
    return new Kernel::KML_Demo_Reporter();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

// ----------------------------------------
// --- KML_Demo_Reporter Methods
// ----------------------------------------

namespace Kernel
{

void
KML_Demo_Reporter::Initialize( unsigned int nrmSize )
{
    LOG_INFO( "KML_Demo_Reporter Initialize\n" );
}


KML_Demo_Reporter::KML_Demo_Reporter()
{
    LOG_INFO( "KML_Demo_Reporter ctor\n" );
}

void
KML_Demo_Reporter::BeginTimestep()
{
    LOG_DEBUG( "BeginTimestep\n" );
}

void
KML_Demo_Reporter::EndTimestep( float currentTime, float dt )
{
    LOG_DEBUG( "EndTimestep\n" );
}

/////////////////////////
// steady-state methods
/////////////////////////
void
KML_Demo_Reporter::LogNodeData(
    Kernel::INodeContext * pNC
)
{
    uint32_t nodeId = pNC->GetExternalID();

    // The following three channels are not currently being used:
    nodeChannelMapVec[ _stat_pop_label ][ nodeId ].push_back( pNC->GetStatPop() );
    nodeChannelMapVec[ _infected_label ][ nodeId ].push_back( pNC->GetInfected() );
}

void 
KML_Demo_Reporter::LogIndividualData(
    Kernel::IIndividualHuman* individual
)
{
}

/////////////////////////
// Finalization methods
/////////////////////////
void
KML_Demo_Reporter::Reduce()
{
    LOG_INFO( "ReduceSpatial\n" );

    tNode2DataMap receive_buffer;

    std::string node_data_pair_send_buffer = "";
    std::string node_data_pair_receive_buffer;
    for( tChannel2Node2DataMapVec::const_iterator cit = nodeChannelMapVec.begin(); cit != nodeChannelMapVec.end(); ++cit )
    {
        const std::string channelName = cit->first;

        // We're going to serialize the node_id,channel_node_value pairs for this channel in order to mpi_send them.
        // We need to populate a buffer and have a receive buffer. Custom packing/parsing.
        std::ostringstream oss;
        for( tNode2DataMapVec::iterator it = nodeChannelMapVec[ channelName ].begin(); it != nodeChannelMapVec[ channelName ].end(); ++it )
        {
            oss << it->first << ":";    // Node Id, then values
            for( tNode2DataMapVecType::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2 )
            {
                if( it2 != it->second.begin() )
                    oss << ",";         // Don't do the comma the first time
                oss << *it2;
            }
            oss << ";";
        }
        node_data_pair_send_buffer = oss.str();

//LOG_INFO_F( "REDUCESPATIAL:send_buffer: %s\n", node_data_pair_send_buffer.c_str() );

        EnvPtr->MPI.p_idm_mpi->GatherToRoot( node_data_pair_send_buffer, node_data_pair_receive_buffer );

//LOG_INFO_F( "REDUCESPATIAL:receive_buffer: %s\n", node_data_pair_receive_buffer.c_str());

        // Got data as string, parse out
        Kernel::suids::suid_data_t nodeId = 0;
        float value = 0.0f;
        if( node_data_pair_receive_buffer.size() > 0 )
        {
            std::istringstream i_node_data_pair_receive_buffer(node_data_pair_receive_buffer);
            std::string string_token = "";
            std::string token = "";
            int cnt;
            while (getline(i_node_data_pair_receive_buffer, string_token, ';'))
            {
                cnt = 0;
                std::istringstream i_timeseries( string_token );
                getline(i_timeseries, token, ':');
                sscanf_s(token.c_str(), "%u",&nodeId);

                if (nodeChannelMapVec[ channelName ].find( nodeId ) != nodeChannelMapVec[ channelName ].end()) //data already exists
                {
                    continue;
                }
                else
                {
                    while (getline(i_timeseries, token, ','))   // Missing last value?
                    {
                        sscanf_s(token.c_str(), "%f",&value);
                        nodeChannelMapVec[ channelName ][ nodeId ].push_back (value);
                    }
                }
            }
        }
    }
}

void KML_Demo_Reporter::Finalize()
{
//    LOG_INFO_F("I'm starting to log my data on rank %d.", EnvPtr->MPI.Rank);
//    ReduceSpatial();
    if( EnvPtr->MPI.Rank > 0 )
    {
        return;
    }

    std::stringstream fake_out("fake_file");        
    // Iterate over nodeChannelMapVec, channel by channel
    for( tChannel2Node2DataMapVec::iterator it = nodeChannelMapVec.begin();
         it != nodeChannelMapVec.end();
         ++it )
    {
        const std::string &channelName = it->first;
        std::string fileName = "ReportByNode_"+channelName + ".csv";

        //space to underscore
        for(int i = 0; i < fileName.length(); i++)
        {
            if(fileName[i] == ' ')
                fileName[i] = '_';
        }

        const tNode2DataMapVec& node2DataMapVec = it->second;

        // Write nodeID in column 1, and data in subsequent columns
        std::ofstream channelDataFile;
        FileSystem::OpenFileForWriting( channelDataFile, FileSystem::Concat( EnvPtr->OutputPath, fileName ).c_str() );

        std::stringstream channelStringStream;
        for( tNode2DataMapVec::const_iterator it2 = node2DataMapVec.begin();
             it2 != node2DataMapVec.end();
             ++it2 )
        {
            const Kernel::suids::suid_data_t &node_id = it2->first;
            channelDataFile << node_id << ",";
            channelStringStream << node_id << ",";

            const tNode2DataMapVecType &dataVec = it2->second;
            for( tNode2DataMapVecType::const_iterator it3 = dataVec.begin();
                it3 != dataVec.end();
                ++it3 )
            {
                const float &value = *it3;
                channelDataFile << value << ",";
                channelStringStream << value << ",";
            }
            channelDataFile << std::endl;
            channelStringStream << std::endl;
        }

        channelDataFile.close();
    }
#ifdef KML
    WriteKmlData();
#endif
}

// This method writes out each channel as an animated KML file so it can be viewed as a 'video' in something
// like Google Earth or NASA WorldWinds.
void KML_Demo_Reporter::WriteKmlData()
{
    LOG_INFO( __FUNCTION__ );

    // Iterate over nodeChannelMapVec, channel by channel
    for( tChannel2Node2DataMapVec::iterator it = nodeChannelMapVec.begin();
         it != nodeChannelMapVec.end();
         ++it )
    {
        // We keep a map of 'last values' to enable sparse KML file output
        map< Kernel::suids::suid_data_t, float > chanValueCache;

        // Construct our output filename.
        const std::string &channelName = it->first;
        std::string kml_file = channelName + ".kml";

        // We start by copying the template for this geography, basically whatever this filename is
        // Note that even though this example has a KML suffix, it's not a valid KML file because
        // it is missing proper closing tags (to save us from having to find the right insertion point).
        // This way we can just append and provide the necessary closure tags ourself.
        const char * master_filename = MASTER_FILE;
        if( !FileSystem::FileExists( master_filename ) )
        {
            throw Kernel::FileNotFoundException( __FILE__, __LINE__, __FUNCTION__, master_filename );
        }

        // Copy the file
        FILE * fp_master=nullptr;
        FILE * fp_clone=nullptr;
        fopen_s( &fp_master, master_filename, "r" );
        fopen_s( &fp_clone, FileSystem::Concat( EnvPtr->OutputPath, kml_file ).c_str(), "w" );

        unsigned char buf[ 1024 ];
        size_t read_size;
        while (read_size = fread(buf, 1, 1024, fp_master))
        {
            fwrite(buf, 1, read_size, fp_clone);
        }

        fclose(fp_clone);
        fclose(fp_master);

        // Write the one-time animation xml
        std::ofstream channelXmlFile;
        FileSystem::OpenFileForWriting( channelXmlFile, FileSystem::Concat( EnvPtr->OutputPath, kml_file ).c_str(), false, true );
        channelXmlFile << "    <gx:Tour>" << std::endl
                       << "      <name>Play me!</name>" << std::endl
                       << "      <gx:Playlist>" << std::endl
                       << "        <gx:Wait>" << std::endl
                       << "          <gx:duration>2.0</gx:duration>" << std::endl
                       << "        </gx:Wait>" << std::endl;

        const tNode2DataMapVec& node2DataMapVec = it->second;

        // Iterate over timesteps
        for( unsigned int timestep = 0;
             timestep < node2DataMapVec.begin()->second.size();
             ++timestep
             )
        {
            // All this kml just to change the colour...
            LOG_DEBUG_F( "Writing kml for timestep %d\n", timestep );
            channelXmlFile << "        <gx:AnimatedUpdate>" << std::endl;
            channelXmlFile << "          <gx:duration>0.0</gx:duration>" << std::endl;
            channelXmlFile << "            <Update>" << std::endl;
            channelXmlFile << "              <targetHref></targetHref>" << std::endl;
            channelXmlFile << "                <Change>" << std::endl;

            // iterate over nodes
            for( tNode2DataMapVec::const_iterator it2 = node2DataMapVec.begin();
                 it2 != node2DataMapVec.end();
                 ++it2 )
            {
                const Kernel::suids::suid_data_t &node_id = it2->first;
                const tNode2DataMapVecType &dataVec = it2->second;
                const float value = dataVec[ timestep ];
                // To get sparser KML files, don't blindly write out updates for each node,
                // for each timestep. Here we use a 10% delta threshold. Use whatever you want.
                if( fabs( value - chanValueCache[ node_id ] ) < 0.1*value )
                {
                    continue;
                }
                chanValueCache[ node_id ] = value;

                // OK, super-hacky normalization here. Do what works for your data.
                int normValue = (int)(255*value/100000);

                // OK, here's the actual colour update kml. For this example we just use grayscale at 50% opacity
                // Use the node id as the taretId.
                channelXmlFile << "              <PolyStyle targetId='ps" << node_id << "'>" << std::endl;
                // convert color to hex RGB somehow
                char colorString[7];
                sprintf_s( &colorString[0], 7, "%02x%02x%02x", normValue, normValue, normValue);
                channelXmlFile << "                <color>7d" << colorString << "</color>" << std::endl;
                channelXmlFile << "              </PolyStyle>" << std::endl;
            }

            // OK, close off this animation/timestep update.
            channelXmlFile << "            </Change>" << std::endl;
            channelXmlFile << "          </Update>" << std::endl;
            channelXmlFile << "        </gx:AnimatedUpdate>" << std::endl;
            channelXmlFile << "        <gx:Wait>" << std::endl;
            channelXmlFile << "          <gx:duration>0.2</gx:duration>" << std::endl;
            channelXmlFile << "        </gx:Wait>" << std::endl;
        }
        // Finally close the KML file
        channelXmlFile << "          </gx:Playlist>" << std::endl
                       << "       </gx:Tour>" << std::endl
                       << "   </Document>" << std::endl
                       << "</kml>" << std::endl;

        channelXmlFile.close();
    }
}

std::string KML_Demo_Reporter::GetReportName() const
{
    return std::string("KML_Demo_Reporter.csv");
}

}
