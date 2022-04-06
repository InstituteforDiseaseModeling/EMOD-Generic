/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "StrainIdentity.h"
#include "Infection.h"
#include "Log.h"

SETUP_LOGGING( "StrainIdentity" )

namespace Kernel {

    StrainIdentity::StrainIdentity(void)
        : cladeID(0)
        , geneticID(0)
    {
    }

    StrainIdentity::StrainIdentity(int initial_clade, int initial_genome)
        : cladeID(initial_clade)
        , geneticID(initial_genome)
    {
        if( initial_clade < 0 )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "initial_clade", initial_clade, 0 );
        }
        else if( initial_clade >= static_cast<int>(InfectionConfig::number_clades) )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "initial_clade", initial_clade, InfectionConfig::number_clades-1 );
        }

        if( initial_genome < 0 )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "initial_genome", initial_genome, 0 );
        }
        else if( initial_genome >= static_cast<int>(InfectionConfig::number_genomes) )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "initial_genome", initial_genome, InfectionConfig::number_genomes-1 );
        }
    }

    StrainIdentity::StrainIdentity( const IStrainIdentity *copy )
        : cladeID( copy->GetCladeID() )
        , geneticID( copy->GetGeneticID() )
    {
        LOG_DEBUG_F( "New infection with clade id %d and genetic id %d\n", cladeID, geneticID );
    }

    StrainIdentity::~StrainIdentity(void)
    {
    }

    std::string StrainIdentity::GetName(void) const
    {
        // Strain name determined by clade and genome
        return (std::to_string(cladeID) + "_" + std::to_string(geneticID));
    }

    int StrainIdentity::GetCladeID(void) const
    {
        return cladeID;
    }

    int StrainIdentity::GetGeneticID(void) const
    {
        return geneticID;
    }

    void StrainIdentity::SetCladeID(int in_cladeID)
    {
        cladeID = in_cladeID;
    }

    void StrainIdentity::SetGeneticID(int in_geneticID)
    {
        geneticID = in_geneticID;
    }

    void StrainIdentity::ResolveInfectingStrain( IStrainIdentity* strainId ) const
    {
        strainId->SetCladeID(cladeID);
        strainId->SetGeneticID(geneticID);
    }

    IArchive& StrainIdentity::serialize(IArchive& ar, StrainIdentity*& ptr)
    {
        if (!ar.IsWriter())
        {
            ptr = new StrainIdentity();
        }

        StrainIdentity& strain = *ptr;

        serialize( ar, strain );

        return ar;
    }

    IArchive& StrainIdentity::serialize(IArchive& ar, StrainIdentity& strain)
    {
        ar.startObject();
            ar.labelElement("cladeID") & strain.cladeID;
            ar.labelElement("geneticID") & strain.geneticID;
        ar.endObject();

        return ar;
    }
}
