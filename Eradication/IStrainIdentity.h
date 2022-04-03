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
        virtual std::string GetName(void) const = 0;
        virtual int  GetCladeID(void) const = 0;
        virtual int  GetGeneticID(void) const = 0;
        virtual void SetCladeID(int in_cladeID) = 0;
        virtual void SetGeneticID(int in_geneticID) = 0;
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
