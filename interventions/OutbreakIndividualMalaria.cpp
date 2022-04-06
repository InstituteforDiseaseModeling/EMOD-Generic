/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "OutbreakIndividualMalaria.h"
#include "Exceptions.h"
#include "SimulationConfig.h"
#include "IGenomeMarkers.h"
#include "MalariaParameters.h"

SETUP_LOGGING( "OutbreakIndividualMalaria" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED( OutbreakIndividualMalaria, OutbreakIndividual )
    END_QUERY_INTERFACE_DERIVED( OutbreakIndividualMalaria, OutbreakIndividual )

    IMPLEMENT_FACTORY_REGISTERED( OutbreakIndividualMalaria )

    OutbreakIndividualMalaria::OutbreakIndividualMalaria()
        : OutbreakIndividual()
        , m_GenomeMarkerNames()
    {
        initSimTypes( 1, "MALARIA_SIM" );
    }

    OutbreakIndividualMalaria::~OutbreakIndividualMalaria()
    {
    }

    bool OutbreakIndividualMalaria::Configure( const Configuration * inputJson )
    {
        const std::set<std::string>* p_known_markers = nullptr;
        if( !JsonConfigurable::_dryrun )
        {
            p_known_markers = &(GET_CONFIGURABLE( SimulationConfig )->malaria_params->pGenomeMarkers->GetNameSet());
        }

        initConfigTypeMap( "Clade", &clade, Clade_DESC_TEXT, 0, 9, 0 );
        initConfigTypeMap( "Genome_Markers", &m_GenomeMarkerNames, OIM_Genome_Markers_DESC_TEXT, "<configuration>.Genome_Markers", *p_known_markers );
        initConfigTypeMap( "Ignore_Immunity", &ignoreImmunity, OB_Ignore_Immunity_DESC_TEXT, true );
        initConfigTypeMap( "Incubation_Period_Override", &incubation_period_override, Incubation_Period_Override_DESC_TEXT, -1.0f, FLT_MAX, -1.0f);

        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret && !JsonConfigurable::_dryrun )
        {
            genome = GET_CONFIGURABLE( SimulationConfig )->malaria_params->pGenomeMarkers->CreateBits( m_GenomeMarkerNames );
        }

        return ret;
    }
}
