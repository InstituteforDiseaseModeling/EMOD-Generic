/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <ctime>
#include "ReportPluginAgeAtInfection.h"
#include "Environment.h"
#include "IIndividualHuman.h"
#include "FileSystem.h"
#include "INodeContext.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"
#include "FactorySupport.h" // for DTK_DLLEXPORT

//******************************************************************************

using namespace std;
using namespace json;

//******************************************************************************

SETUP_LOGGING( "ReportPluginAgeAtInfection" )

static const char* _sim_types[] = {"GENERIC_SIM", "VECTOR_SIM", "MALARIA_SIM", "POLIO_SIM", "TBHIV_SIM", NULL};

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
    return new ReportPluginAgeAtInfection();
}

#ifdef __cplusplus
}
#endif

//******************************************************************************

/////////////////////////
// Initialization methods
/////////////////////////


ReportPluginAgeAtInfection::ReportPluginAgeAtInfection()
: BaseChannelReport( "AgeAtInfectionReport.json" )
, sampling_ratio(1.0f)
{
    LOG_INFO( "ReportPluginAgeAtInfection ctor\n" );
}

/////////////////////////
// steady-state methods
/////////////////////////
void
ReportPluginAgeAtInfection::EndTimestep(float currentTime, float dt)
{
    time_age_map.emplace((unsigned int)time_age_map.size()+1u, ages);
    ages.clear();
}

void
ReportPluginAgeAtInfection::LogNodeData(
    Kernel::INodeContext * const pNode
)
{
}

void 
ReportPluginAgeAtInfection::LogIndividualData(
    Kernel::IIndividualHuman* individual
)
{
    LOG_DEBUG( "LogIndividualData\n" );

    if (individual->GetNewInfectionState() == NewInfectionState::NewAndDetected || individual->GetNewInfectionState() == NewInfectionState::NewInfection)
    {
        LOG_DEBUG_F("Individual's New infection state is %d\n", (int)individual->GetNewInfectionState());
        if( sampling_ratio == 1.0 || ( individual->GetParent()->GetRng()->e() < sampling_ratio) )
        {
            ages.push_back(individual->GetAge());
        }
    }
}


// Just calling the base class but for demo purposes leaving in because I can imagine wanting to do this custom.
void
ReportPluginAgeAtInfection::Finalize()
{
    LOG_INFO( "WriteData\n" );
    postProcessAccumulatedData();

    std::map<std::string, std::string> units_map;
    populateSummaryDataUnitsMap(units_map);

    time_t now = time(0);
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

    Element elementRoot = String();
    QuickBuilder qb(elementRoot);
    qb["Header"]["DateTime"] = String(now3.substr(0,now3.length()-1)); // have to remove trailing '\n'

    ProgDllVersion pv;
    ostringstream dtk_ver;
    dtk_ver << pv.getRevisionNumber() << " " << pv.getSccsBranch() << " " << pv.getBuildDate();
    qb["Header"]["DTK_Version"] = String(dtk_ver.str());
    qb["Header"]["Report_Version"] = String("3");
    qb["Header"]["Timesteps"] = String("3");
    qb["Channels"]["Ages"]["Units"] = String("Days");
    int timestep_ind = 0;
    for (auto& timestep_entry : time_age_map)
    {
        const float curr_timestep = timestep_entry.first;
        const vector<float> curr_ages = timestep_entry.second;

        int age_ind = 0;
        if (curr_ages.size() == 0)
        {
            qb["Channels"]["Ages"]["Data"][timestep_ind][0] = Number(-1);
        }
        else
        {
            for (auto& age_entry : curr_ages)
            {
                qb["Channels"]["Ages"]["Data"][timestep_ind][age_ind] = Number(age_entry);
                age_ind++;
            } 
        }
        timestep_ind++;

    }
    qb["Header"]["Timesteps"] = Number(timestep_ind);

    // write to an internal buffer first... if we write directly to the network share, performance is slow
    // (presumably because it's doing a bunch of really small writes of all the JSON elements instead of one
    // big write)
    ostringstream oss;
    Writer::Write(elementRoot, oss);

    ofstream inset_chart_json;
    FileSystem::OpenFileForWriting( inset_chart_json, FileSystem::Concat( EnvPtr->OutputPath, report_name ).c_str() );

    inset_chart_json << oss.str();
    inset_chart_json.close();
}

void 
ReportPluginAgeAtInfection::populateSummaryDataUnitsMap(
    std::map<std::string, std::string> &units_map
)
{
    LOG_INFO( "populateSummaryDataUnitsMap\n" );
    units_map["AgeAtInfection"]                        = "Age at Infection";
    units_map["TimeOfInfection"]                       = "Time of Infection";
    units_map["MCWeight"]                                  = "MC Weight";
}

// not sure whether to leave this in custom demo subclass
void
ReportPluginAgeAtInfection::postProcessAccumulatedData()
{
    LOG_DEBUG( "getSummaryDataCustomProcessed\n" );
}

void
ReportPluginAgeAtInfection::Reduce()
{
    LOG_INFO( "Reduce\n" );
    
}
