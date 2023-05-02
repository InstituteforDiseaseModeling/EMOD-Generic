/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "MathFunctions.h"
#include "DistributionFactory.h"
#include "Distributions.h"
#include "DistributionsConfigurable.h"
#include "IDistribution.h"

SETUP_LOGGING( "DistributionFactory" )

namespace Kernel
{

    IDistribution* DistributionFactory::CreateDistribution( DistributionFunction::Enum dist_val )
    {
        switch( dist_val )
        {
        case DistributionFunction::CONSTANT_DISTRIBUTION:
            return new DistributionConstant();
        case DistributionFunction::GAUSSIAN_DISTRIBUTION:
            return new DistributionGaussian();
        case DistributionFunction::POISSON_DISTRIBUTION:
            return new DistributionPoisson();
        case DistributionFunction::EXPONENTIAL_DISTRIBUTION:
            return new DistributionExponential();
        case DistributionFunction::GAMMA_DISTRIBUTION:
            return new DistributionGamma();
        case DistributionFunction::LOG_NORMAL_DISTRIBUTION:
            return new DistributionLogNormal();
        case DistributionFunction::DUAL_EXPONENTIAL_DISTRIBUTION:
            return new DistributionDualExponential();
        case DistributionFunction::WEIBULL_DISTRIBUTION:
            return new DistributionWeibull();
        case DistributionFunction::DUAL_CONSTANT_DISTRIBUTION:
            return new DistributionDualConstant();
        case DistributionFunction::UNIFORM_DISTRIBUTION:
            return new DistributionUniform();
        default:
            throw Kernel::IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "DistributionFunction does not exist." );
        }
    }


    // Factory that configures distribution assuming all distributions are valid options
    IDistribution* DistributionFactory::CreateDistribution( JsonConfigurable* pParent, DistributionFunction::Enum dist_val, std::string base_name, const Configuration* config )
    {
        if( JsonConfigurable::_dryrun )
        {
            for(auto dist_int : DistributionFunction::pairs::get_values())
            {
                AddToSchema( pParent, config, base_name, DistributionFunction::Enum(dist_int) );
            }
            return nullptr;
        }

        return MakeDistPtr( pParent, config, base_name, dist_val );
    }


    // Factory that configures distribution using strings and look-up for valid options
    IDistribution* DistributionFactory::CreateDistribution( JsonConfigurable* pParent, const Configuration* config, std::string base_name, std::string dist_name, std::vector<std::string> dist_opts )
    {
        if( JsonConfigurable::_dryrun )
        {
            for(auto dist_str : dist_opts)
            {
                int dist_int = DistributionFunction::pairs::lookup_value(dist_str);
                AddToSchema( pParent, config, base_name, DistributionFunction::Enum(dist_int) );
            }
            return nullptr;
        }

        int dist_int = DistributionFunction::pairs::lookup_value(dist_name);
        return MakeDistPtr( pParent, config, base_name, DistributionFunction::Enum(dist_int) );
    }


    void DistributionFactory::AddToSchema( JsonConfigurable* pParent, const Configuration* config, std::string base_name, DistributionFunction::Enum dist_val )
    {
        switch( dist_val )
        {
            case DistributionFunction::NOT_INITIALIZED:
            {
                break;
            }
            case DistributionFunction::CONSTANT_DISTRIBUTION:
            {
                DistributionConstantConfigurable distribution_constant;
                distribution_constant.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::GAUSSIAN_DISTRIBUTION:
            {
                DistributionGaussianConfigurable distribution_gaussian;
                distribution_gaussian.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::POISSON_DISTRIBUTION:
            {
                DistributionPoissonConfigurable distribution_poisson;
                distribution_poisson.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::EXPONENTIAL_DISTRIBUTION:
            {
                DistributionExponentialConfigurable distribution_exponential;
                distribution_exponential.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::GAMMA_DISTRIBUTION:
            {
                DistributionGammaConfigurable distribution_gamma;
                distribution_gamma.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::LOG_NORMAL_DISTRIBUTION:
            {
                DistributionLogNormalConfigurable distribution_log_normal;
                distribution_log_normal.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::DUAL_EXPONENTIAL_DISTRIBUTION:
            {
                DistributionDualExponentialConfigurable distribution_dual_exponential;
                distribution_dual_exponential.Configure(pParent, base_name, config);
                break;
            }
            case DistributionFunction::WEIBULL_DISTRIBUTION:
            {
                DistributionWeibullConfigurable distribution_weibull;
                distribution_weibull.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::DUAL_CONSTANT_DISTRIBUTION:
            {
                DistributionDualConstantConfigurable distribution_dual_constant;
                distribution_dual_constant.Configure( pParent, base_name, config );
                break;
            }
            case DistributionFunction::UNIFORM_DISTRIBUTION:
            {
                DistributionUniformConfigurable distribution_uniform;
                distribution_uniform.Configure( pParent, base_name, config );
                break;
            }
            default:
            {
                release_assert(false);
            }
        }

        return;
    }


    IDistribution* DistributionFactory::MakeDistPtr( JsonConfigurable* pParent, const Configuration* config, std::string base_name, DistributionFunction::Enum dist_val )
    {
        IDistribution* idist_ptr = nullptr;

        switch( dist_val )
        {
            case DistributionFunction::NOT_INITIALIZED:
            {
                break;
            }
            case DistributionFunction::CONSTANT_DISTRIBUTION:
            {
                DistributionConstantConfigurable* dist_ptr = new DistributionConstantConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::GAUSSIAN_DISTRIBUTION:
            {
                DistributionGaussianConfigurable* dist_ptr = new DistributionGaussianConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::POISSON_DISTRIBUTION:
            {
                DistributionPoissonConfigurable* dist_ptr = new DistributionPoissonConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::EXPONENTIAL_DISTRIBUTION:
            {
                DistributionExponentialConfigurable* dist_ptr = new DistributionExponentialConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::GAMMA_DISTRIBUTION:
            {
                DistributionGammaConfigurable* dist_ptr = new DistributionGammaConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::LOG_NORMAL_DISTRIBUTION:
            {
                DistributionLogNormalConfigurable* dist_ptr = new DistributionLogNormalConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::DUAL_EXPONENTIAL_DISTRIBUTION:
            {
                DistributionDualExponentialConfigurable* dist_ptr = new DistributionDualExponentialConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::WEIBULL_DISTRIBUTION:
            {
                DistributionWeibullConfigurable* dist_ptr = new DistributionWeibullConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::DUAL_CONSTANT_DISTRIBUTION:
            {
                DistributionDualConstantConfigurable* dist_ptr = new DistributionDualConstantConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            case DistributionFunction::UNIFORM_DISTRIBUTION:
            {
                DistributionUniformConfigurable* dist_ptr = new DistributionUniformConfigurable();
                dist_ptr->Configure( pParent, base_name, config );
                idist_ptr = static_cast<IDistribution*>(dist_ptr);
                break;
            }
            default:
            {
                release_assert(false);
            }
        }

        return idist_ptr;
    }

}
