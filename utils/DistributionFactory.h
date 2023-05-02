/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Configure.h"
#include "IDistribution.h"


namespace Kernel
{
    class DistributionFactory
    {
    public:
        static IDistribution* CreateDistribution( DistributionFunction::Enum dist_val );
        static IDistribution* CreateDistribution( JsonConfigurable* pParent, DistributionFunction::Enum dist_val, std::string base_name, const Configuration* config );
        static IDistribution* CreateDistribution( JsonConfigurable* pParent, const Configuration* config, std::string base_name, std::string dist_name, std::vector<std::string> dist_opts );

    private:
        static void           AddToSchema( JsonConfigurable* pParent, const Configuration* config, std::string base_name, DistributionFunction::Enum dist_val );
        static IDistribution* MakeDistPtr( JsonConfigurable* pParent, const Configuration* config, std::string base_name, DistributionFunction::Enum dist_val );
    };
}