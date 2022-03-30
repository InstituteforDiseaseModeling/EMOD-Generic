/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "StrainIdentity.h"
#include "Infection.h"
#include "SimulationConfig.h"
#include "Log.h"

SETUP_LOGGING( "StrainIdentity" )

namespace Kernel {

    StrainIdentity::StrainIdentity(void)
        : cladeID(0)
        , geneticID(0)
    {
    }

    StrainIdentity::StrainIdentity(int initial_clade, int initial_genome, RANDOMBASE * pRng )
        : cladeID( initial_clade )
        , geneticID( initial_genome )
    {
        if( initial_clade >= int(InfectionConfig::number_clades) )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "initial_clade", initial_clade, InfectionConfig::number_clades );
        }

        if ( initial_genome < 0 )
        {
            int max_genome = InfectionConfig::number_genomes;
            unsigned int BARCODE_BITS = 0;
            while(max_genome >>= 1) ++BARCODE_BITS;
            geneticID = pRng->ul() & ((1 << BARCODE_BITS)-1);
            LOG_DEBUG_F("random genome generation... clade: %d\t genome: %d\n", cladeID, geneticID);
        }
        else if( initial_genome >= int(InfectionConfig::number_genomes) )
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
