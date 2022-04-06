/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <string>


#include "SusceptibilityPy.h"
#include "IIndividualHumanContext.h"

SETUP_LOGGING( "SusceptibilityPy" )

#ifdef ENABLE_PYTHON

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Py.Susceptibility,SusceptibilityPyConfig)
    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPyConfig)
    END_QUERY_INTERFACE_BODY(SusceptibilityPyConfig)

    bool
    SusceptibilityPyConfig::Configure(
        const Configuration* config
    )
    {
        LOG_DEBUG( "Configure\n" );
        return JsonConfigurable::Configure( config );
    }

    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPy)
        HANDLE_INTERFACE(ISusceptibilityPy)
        HANDLE_INTERFACE(ISusceptibilityPyReportable)
    END_QUERY_INTERFACE_BODY(SusceptibilityPy)

    SusceptibilityPy::SusceptibilityPy(IIndividualHumanContext *context)
        : Susceptibility(context) 
    {
        // Everything initialized to 0 in Initialize
    }

    void SusceptibilityPy::Initialize(float _immmod, float _riskmod)
    {
        LOG_DEBUG_F( "Initializing Py immunity object for new individual: id=%lu, immunity modifier=%f, risk modifier=%f\n", parent->GetSuid().data, _immmod, _riskmod );
        Susceptibility::Initialize(_immmod, _riskmod);

        // throws exception on error, no return type. 
    }

    SusceptibilityPy::~SusceptibilityPy(void)
    {
    }

    SusceptibilityPy *SusceptibilityPy::CreateSusceptibility(IIndividualHumanContext *context, float immmod, float riskmod)
    {
        //LOG_DEBUG_F( "Creating Py immunity object for new individual: immunity modifier=%f, risk modifier=%f\n", immmod, riskmod );
        SusceptibilityPy *newsusceptibility = _new_ SusceptibilityPy(context);
        newsusceptibility->Initialize(immmod, riskmod);

        return newsusceptibility;
    }

    void SusceptibilityPy::Update(float dt)
    { }
}

#endif // ENABLE_PYTHON
