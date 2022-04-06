/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "WaningEffect.h"
#include "CajunIncludes.h"

SETUP_LOGGING( "WaningEffectNull" )

namespace Kernel
{
    IMPLEMENT_FACTORY_REGISTERED(WaningEffectNull)
    IMPL_QUERY_INTERFACE2(WaningEffectNull, IWaningEffect, IConfigurable)

    WaningEffectNull::WaningEffectNull()
    : IWaningEffect()
    , JsonConfigurable()
    { }

    WaningEffectNull::WaningEffectNull(const WaningEffectNull& rOrig)
    : IWaningEffect()
    , JsonConfigurable()
    { }

    IWaningEffect* WaningEffectNull::Clone()
    {
        return new WaningEffectNull(*this);
    }

    REGISTER_SERIALIZABLE(WaningEffectNull);

    void WaningEffectNull::serialize(IArchive& ar, WaningEffectNull* obj)
    {
        WaningEffectNull& effect = *obj;
    }
}
