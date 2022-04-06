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
    { }

    StrainIdentity::StrainIdentity(uint32_t initial_clade, uint64_t initial_genome)
        : cladeID(initial_clade)
        , geneticID(initial_genome)
    { }

    StrainIdentity::StrainIdentity( const IStrainIdentity *copy )
        : cladeID( copy->GetCladeID() )
        , geneticID( copy->GetGeneticID() )
    {
        LOG_DEBUG_F( "New infection with clade id %d and genetic id %d\n", cladeID, geneticID );
    }

    StrainIdentity::~StrainIdentity(void)
    { }

    std::pair<uint32_t, uint64_t> StrainIdentity::GetStrainName(void) const
    {
        // Provides a unique and hashable indentifier
        return std::make_pair(cladeID,geneticID);
    }

    uint32_t StrainIdentity::GetCladeID(void) const
    {
        return cladeID;
    }

    uint64_t StrainIdentity::GetGeneticID(void) const
    {
        return geneticID;
    }

    void StrainIdentity::SetCladeID(uint32_t in_cladeID)
    {
        cladeID = in_cladeID;
    }

    void StrainIdentity::SetGeneticID(uint64_t in_geneticID)
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
