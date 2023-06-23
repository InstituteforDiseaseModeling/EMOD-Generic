/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#ifdef ENABLE_PYTHON

#include "Sugar.h"
#include "Debug.h"
#include "Environment.h"
#include "InterventionFactory.h"
#include "PyInterventionsContainer.h"

SETUP_LOGGING( "PyInterventionsContainer" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(PyInterventionsContainer, InterventionsContainer)
    END_QUERY_INTERFACE_DERIVED(PyInterventionsContainer, InterventionsContainer)

    PyInterventionsContainer::PyInterventionsContainer()
    {
    }

    PyInterventionsContainer::~PyInterventionsContainer()
    {
    }
}

#endif // ENABLE_PYTHON
