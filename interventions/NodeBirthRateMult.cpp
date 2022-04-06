/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "NodeBirthRateMult.h"
#include "NodeEventContext.h"
#include "INodeContext.h"

SETUP_LOGGING("NodeBirthRateMult")

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY( NodeBirthRateMult )
        HANDLE_INTERFACE( IConfigurable )
        HANDLE_INTERFACE( INodeDistributableIntervention )
        HANDLE_INTERFACE( IBaseIntervention )
        HANDLE_ISUPPORTS_VIA( INodeDistributableIntervention )
    END_QUERY_INTERFACE_BODY( NodeBirthRateMult )

    IMPLEMENT_FACTORY_REGISTERED( NodeBirthRateMult )

    NodeBirthRateMult::NodeBirthRateMult()
        : BaseNodeIntervention()
        , duration(0.0f)
        , mult_by_duration()
    { }

    NodeBirthRateMult::NodeBirthRateMult(const NodeBirthRateMult& original)
        : BaseNodeIntervention(original)
        , duration(original.duration)
        , mult_by_duration(original.mult_by_duration)
    { }

    NodeBirthRateMult::~NodeBirthRateMult()
    { }

    bool NodeBirthRateMult::Configure(const Configuration* inputJson)
    {
        initConfigTypeMap("Multiplier_By_Duration",  &mult_by_duration,  NBRMIV_Multiplier_By_Duration_DESC_TEXT);

        return BaseNodeIntervention::Configure( inputJson );
    }

    bool NodeBirthRateMult::Distribute(INodeEventContext* context, IEventCoordinator2* pEC)
    {
        duration = 0.0f;

        bool distributed = BaseNodeIntervention::Distribute(context, pEC);

        return distributed;
    }

    void NodeBirthRateMult::Update(float dt)
    {
        duration += dt;

        if(mult_by_duration.isAtEnd(duration))
        {
            expired = true;
        }
        else
        {
            parent->UpdateBirthRateMultiplier(mult_by_duration.getValueLinearInterpolation(duration, 1.0f));
        }
    }

    void NodeBirthRateMult::serialize(IArchive& ar, NodeBirthRateMult* obj)
    {
        BaseNodeIntervention::serialize(ar, obj);

        NodeBirthRateMult& infect_mult_obj = *obj;

        ar.labelElement("duration")                        & infect_mult_obj.duration;
        ar.labelElement("mult_by_duration")                & infect_mult_obj.mult_by_duration;
    }
}
