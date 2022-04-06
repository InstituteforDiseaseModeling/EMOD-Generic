/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "NodeSet.h"
#include "ObjectFactoryTemplates.h"


SETUP_LOGGING( "NodeSetFactory" )

namespace Kernel
{
    NodeSetFactory* NodeSetFactory::_instance = nullptr;

    template NodeSetFactory* ObjectFactory<INodeSet, NodeSetFactory>::getInstance();
}
