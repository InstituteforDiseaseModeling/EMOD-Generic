/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

namespace Kernel
{
    struct IStrainIdentity
    {
        virtual std::pair<uint32_t, uint64_t> GetStrainName(void) const = 0;
        virtual uint32_t GetCladeID(void) const = 0;
        virtual uint64_t GetGeneticID(void) const = 0;
        virtual void SetCladeID(uint32_t in_cladeID) = 0;
        virtual void SetGeneticID(uint64_t in_geneticID) = 0;
        virtual void ResolveInfectingStrain( IStrainIdentity* strainId ) const = 0;

        // Order first by cladeID, then by geneticID
        inline virtual bool operator<(const IStrainIdentity& id) const 
        {
            return ( this->GetCladeID() <  id.GetCladeID() ) || 
                   ( this->GetCladeID() == id.GetCladeID() && this->GetGeneticID() < id.GetGeneticID() );
        }

        inline virtual bool operator>(const IStrainIdentity& id) const 
        {
            return ( this->GetCladeID() >  id.GetCladeID() ) ||
                   ( this->GetCladeID() == id.GetCladeID() && this->GetGeneticID() > id.GetGeneticID() );
        }
    };
}
