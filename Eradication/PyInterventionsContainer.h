/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#ifdef ENABLE_PYTHON
#include <string>
#include <list>
#include <vector>

#include "Interventions.h"
#include "InterventionsContainer.h"

namespace Kernel
{
    class IPyVaccine;

    class PyInterventionsContainer : public InterventionsContainer
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        PyInterventionsContainer();
        virtual ~PyInterventionsContainer();

        virtual QueryResult QueryInterface(iid_t iid, void** pinstance) override;

    private:
    };
}
#endif
