/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <functional>
#include <map>

#include "Report.h"
#include "INodeContext.h"
#include "IIndividualHuman.h"
#include "SimulationEnums.h"
#include "ConfigParams.h"
#include "Climate.h"

using namespace std;
using namespace json;

SETUP_LOGGING( "Report" )

const string Report::_stat_pop_label       ( "Statistical Population" );
const string Report::_susceptible_pop_label( "Susceptible Population" );
const string Report::_exposed_pop_label    ( "Exposed Population" );
const string Report::_infectious_pop_label ( "Infectious Population" );
const string Report::_recovered_pop_label  ( "Recovered Population" );
const string Report::_waning_pop_label     ( "Waning Population" );
const string Report::_immunized_pop_label  ( "Immunized Population" );

const string Report::_infected_fraction_label   ( "Infected Fraction" );
const string Report::_new_infections_label      ( "New Infections" );
const string Report::_hum_infectious_res_label  ( "Human Infectious Reservoir" );
const string Report::_infection_rate_label      ( "Daily (Human) Infection Rate" );


/////////////////////////
// Initialization methods
/////////////////////////
Kernel::IReport* Report::CreateReport()
{
    return new Report();
}


Report::Report()
    : Report( "InsetChart.json" )
{
}


Report::Report( const std::string& rReportName )
    : BaseChannelReport( rReportName )
    , env_rep(false)
    , contact_infections_counter(0.0f)
    , countOfSusceptibles(0.0f)
    , countOfExposed(0.0f)
    , countOfInfectious(0.0f)
    , countOfRecovered(0.0f)
    , countOfImmunized(0.0f)
    , countOfWaning(0.0f)
    , disease_deaths(0.0f)
    , enviro_infections_counter(0.0f)
    , new_infections(0.0f)
    , new_reported_infections(0.0f)
{
}


/////////////////////////
// steady-state methods
/////////////////////////
void Report::BeginTimestep()
{
    BaseChannelReport::BeginTimestep();

    new_infections          = 0.0f;
    new_reported_infections = 0.0f;

    enviro_infections_counter  = 0.0f;
    contact_infections_counter = 0.0f;

    countOfSusceptibles = 0.0f;
    countOfExposed      = 0.0f;
    countOfInfectious   = 0.0f;
    countOfRecovered    = 0.0f;
    countOfWaning       = 0.0f;
}


void Report::LogIndividualData( Kernel::IIndividualHuman* individual )
{
    auto mcw = individual->GetMonteCarloWeight();
    auto nis = individual->GetNewInfectionState();

    if(nis == NewInfectionState::NewAndDetected || nis == NewInfectionState::NewInfection)
    {
        new_infections += mcw;

        // Check is necessarry; new infection may have already cleared
        if(individual->GetInfections().size() > 0)
        {
            auto inf = individual->GetInfections().back();
            if( inf->GetSourceRoute() == Kernel::TransmissionRoute::ENVIRONMENTAL )
            {
                enviro_infections_counter += mcw;
            }
            else if( inf->GetSourceRoute() == Kernel::TransmissionRoute::CONTACT )
            {
                contact_infections_counter += mcw;
            }
        }
    }

    if(nis == NewInfectionState::NewAndDetected || nis == NewInfectionState::NewlyDetected)
    {
        new_reported_infections += mcw;
    }

    if(individual->GetStateChange() == HumanStateChange::KilledByInfection)
    {
        disease_deaths += mcw;
    }

    UpdateSEIRW(individual, mcw);
}


void Report::LogNodeData( Kernel::INodeContext * pNC )
{
    LOG_DEBUG( "LogNodeData\n" );

    Accumulate(_stat_pop_label,          pNC->GetStatPop());
    Accumulate("Births",                 pNC->GetBirths());
    Accumulate("Infected",               pNC->GetInfected());
    Accumulate("Symptomatic Population", pNC->GetSymptomatic());

    Accumulate("Newly Symptomatic",      pNC->GetNewlySymptomatic() + new_reported_infections ); //either GetNewlySymptomatic() or new_reported_infections is used (other channel is 0)
    new_reported_infections = 0.0f;

    if (pNC->GetLocalWeather())
    {
        Accumulate("Air Temperature",   pNC->GetLocalWeather()->airtemperature());
        Accumulate("Land Temperature",  pNC->GetLocalWeather()->landtemperature());
        Accumulate("Rainfall",          pNC->GetLocalWeather()->accumulated_rainfall());
        Accumulate("Relative Humidity", pNC->GetLocalWeather()->humidity());
    }

    if(pNC->GetParams()->enable_environmental_route)
    {
        env_rep = true;
        auto contagionPop = pNC->GetContagionByRoute();

        Accumulate("Contact Contagion Population",       contagionPop[Kernel::TransmissionRoute::CONTACT]  );
        Accumulate("Environmental Contagion Population", contagionPop[Kernel::TransmissionRoute::ENVIRONMENTAL] );
    }

    Accumulate(_new_infections_label,            new_infections);
    new_infections = 0.0f;

    Accumulate("Campaign Cost",                  pNC->GetCampaignCost());
    Accumulate("Human Infectious Reservoir",     pNC->GetInfectivity());
    Accumulate(_infection_rate_label,            pNC->GetInfectionRate());

    AccumulateSEIRW();
}


void Report::EndTimestep( float currentTime, float dt )
{
    Accumulate("Disease Deaths", disease_deaths);

    if(env_rep)
    {
        Accumulate( "New Infections By Route (ENVIRONMENT)",  enviro_infections_counter );
        Accumulate( "New Infections By Route (CONTACT)",      contact_infections_counter );
    }

    BaseChannelReport::EndTimestep( currentTime, dt );
}


void Report::populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map )
{
    units_map[_stat_pop_label]                  = "Population";
    units_map["Births"]                         = "Births";
    units_map["Infected"]                       = _infected_fraction_label;
    units_map["Rainfall"]                       = "mm/day";
    units_map["Temperature"]                    = "degrees C";
    units_map[_new_infections_label]            = "";
    units_map["Reported New Infections"]        = "";
    units_map["Disease Deaths"]                 = "";
    units_map["Campaign Cost"]                  = "USD";
    units_map[_hum_infectious_res_label]        = "Total Infectivity";
    units_map[_infection_rate_label]            = "Infection Rate";

    AddSEIRWUnits(units_map);
}


// normalize by timestep and create derived channels
void Report::postProcessAccumulatedData()
{
    LOG_DEBUG( "postProcessAccumulatedData\n" );

    normalizeChannel("Infected", _stat_pop_label);
    if( channelDataMap.HasChannel( "Air Temperature" ) )
    {
        normalizeChannel("Air Temperature", float(_nrmSize));
        normalizeChannel("Land Temperature", float(_nrmSize));
        normalizeChannel("Relative Humidity", float(_nrmSize));
        normalizeChannel("Rainfall", float(_nrmSize) * (1 / 1000.0f)); // multiply by 1000 to get result in mm/day
    }

    normalizeChannel( _infection_rate_label, float(_nrmSize) );

    // add derived channels
    NormalizeSEIRWChannels();
}


void Report::UpdateSEIRW( const Kernel::IIndividualHuman* individual, float monte_carlo_weight )
{
    if (!individual->IsInfected())  // Susceptible, Recovered (Immune), or Waning
    {
        float acquisitionModifier = individual->GetImmunityReducedAcquire() * individual->GetInterventionReducedAcquire();
        if (acquisitionModifier >= 1.0f)
        {
            countOfSusceptibles += monte_carlo_weight;
        }
        else if (acquisitionModifier > 0.0f)
        {
            countOfWaning += monte_carlo_weight;
        }
        else
        {
            countOfRecovered += monte_carlo_weight;
        }
    }
    else // Exposed or Infectious 
    {
        if (individual->GetInfectiousness() > 0.0f)
        {
            countOfInfectious += monte_carlo_weight;
        }
        else
        {
            countOfExposed += monte_carlo_weight;
        }
    }
}


void Report::AccumulateSEIRW()
{
    Accumulate( _susceptible_pop_label, countOfSusceptibles );
    Accumulate( _exposed_pop_label,     countOfExposed );
    Accumulate( _infectious_pop_label,  countOfInfectious );
    Accumulate( _recovered_pop_label,   countOfRecovered );
    Accumulate( _waning_pop_label,      countOfWaning );

    countOfSusceptibles = 0.0f;
    countOfExposed      = 0.0f;
    countOfInfectious   = 0.0f;
    countOfRecovered    = 0.0f;
    countOfWaning       = 0.0f;
}


void Report::AddSEIRWUnits( std::map<std::string, std::string> &units_map )
{
    units_map[_susceptible_pop_label] = "Susceptible Fraction";
    units_map[_exposed_pop_label]     = "Exposed Fraction";
    units_map[_infectious_pop_label]  = "Infectious Fraction";
    units_map[_recovered_pop_label]   = "Recovered (Immune) Fraction";
    units_map[_waning_pop_label]      = "Waning Immunity Fraction";
}

void Report::NormalizeSEIRWChannels()
{
    normalizeChannel(_susceptible_pop_label, _stat_pop_label);
    normalizeChannel(_exposed_pop_label,     _stat_pop_label);
    normalizeChannel(_infectious_pop_label,  _stat_pop_label);
    normalizeChannel(_recovered_pop_label,   _stat_pop_label);
    normalizeChannel(_waning_pop_label,      _stat_pop_label);
}