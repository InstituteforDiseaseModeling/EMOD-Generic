/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <map>

#include "ISerializable.h"
#include "Configure.h"

namespace Kernel
{
    struct IIndividualHumanContext;
    struct INodeEventContext;

    struct IWaningEffect : ISerializable
    {
        virtual IWaningEffect* Clone()                               = 0;

        virtual bool  Configure(const Configuration*)                = 0;
        virtual void  Update(float)                                  = 0;
        virtual float Current()                               const  = 0;
        virtual bool  Expired()                               const  = 0;
        virtual void  SetContextTo(IIndividualHumanContext*)         = 0;
        virtual void  SetContextTo(INodeEventContext*)               = 0;
        virtual float GetInitial()                            const  = 0;
        virtual void  SetInitial(float)                              = 0;

        virtual JsonConfigurable* GetConfigurable()                  = 0;
    };

    class WaningEffectFactory
    {
    public:
        static IWaningEffect* CreateInstance();
    };
}
