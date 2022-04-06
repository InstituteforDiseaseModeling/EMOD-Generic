/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <tuple>
#include <vector>

typedef std::tuple<uint32_t,uint64_t,int32_t> sparse_contagion_id;

struct sparse_contagion_repr
{
    public:
        sparse_contagion_repr()
            : inf_clade()
            , inf_genome()
            , inf_group()
            , inf_vals()
        { }

        sparse_contagion_repr(const sparse_contagion_repr& rhs_arg)
            : inf_clade()
            , inf_genome()
            , inf_group()
            , inf_vals()
        { 
            inf_clade   = rhs_arg.inf_clade;
            inf_genome  = rhs_arg.inf_genome;
            inf_group   = rhs_arg.inf_group;
            inf_vals    = rhs_arg.inf_vals;
        }

        sparse_contagion_repr& operator=(const sparse_contagion_repr& rhs_arg)
        {
            inf_clade   = rhs_arg.inf_clade;
            inf_genome  = rhs_arg.inf_genome;
            inf_group   = rhs_arg.inf_group;
            inf_vals    = rhs_arg.inf_vals;

            return *this;
        }

        void clear()
        {
            inf_clade.clear();
            inf_genome.clear();
            inf_group.clear();
            inf_vals.clear();
        }

        size_t size() const
        {
            return inf_clade.size();
        }

        void add(uint32_t clade, uint64_t genome, int32_t group, float val)
        {
            inf_clade.push_back(clade);
            inf_genome.push_back(genome);
            inf_group.push_back(group);
            inf_vals.push_back(val);
        }

        sparse_contagion_id GetId(size_t idx) const
        {
            return std::make_tuple(inf_clade[idx], inf_genome[idx], inf_group[idx]);
        }

        float GetInf(size_t idx) const
        {
            return inf_vals[idx];
        }

        std::vector<uint32_t>      inf_clade;
        std::vector<uint64_t>      inf_genome;
        std::vector< int32_t>      inf_group;
        std::vector<float>         inf_vals;
};