/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IArchive.h"
#include "IStrainIdentity.h"
#include "RANDOM.h"

namespace Kernel
{
    class StrainIdentity : public IStrainIdentity
    {
    public:
        StrainIdentity(void);
        StrainIdentity(uint32_t initial_clade, uint64_t initial_genome);
        StrainIdentity(const IStrainIdentity *copy);
        virtual ~StrainIdentity(void);

        // IStrainIdentity methods
        std::pair<uint32_t, uint64_t> GetStrainName(void) const override;
        virtual uint32_t GetCladeID(void) const override;
        virtual uint64_t GetGeneticID(void) const override;
        virtual void SetCladeID(uint32_t in_cladeID) override;
        virtual void SetGeneticID(uint64_t in_geneticID) override;
        virtual void ResolveInfectingStrain(IStrainIdentity* strainId) const;

        static IArchive& serialize(IArchive&, StrainIdentity*&);
        static IArchive& serialize(IArchive&, StrainIdentity&);

    protected:
        uint32_t cladeID;
        uint64_t geneticID;
    };
}
