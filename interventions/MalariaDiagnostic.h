/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Diagnostics.h"
#include "InterventionEnums.h"

namespace Kernel
{
    class MalariaDiagnostic : public SimpleDiagnostic 
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        DECLARE_FACTORY_REGISTERED(InterventionFactory, MalariaDiagnostic, IDistributableIntervention)

    public: 
        MalariaDiagnostic();
        MalariaDiagnostic( const MalariaDiagnostic& );

        virtual bool Configure( const Configuration* pConfig ) override;
        virtual ~MalariaDiagnostic();

    protected:
        virtual bool positiveTestResult() override;

        MalariaDiagnosticType::Enum malaria_diagnostic_type;
        float detection_threshold;

        DECLARE_SERIALIZABLE(MalariaDiagnostic);
    };
}


