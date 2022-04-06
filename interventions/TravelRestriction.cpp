/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "TravelRestriction.h"
#include "NodeEventContext.h"
#include "INodeContext.h"

SETUP_LOGGING("TravelRestriction")

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY( TravelRestriction )
        HANDLE_INTERFACE( IConfigurable )
        HANDLE_INTERFACE( INodeDistributableIntervention )
        HANDLE_INTERFACE( IBaseIntervention )
        HANDLE_ISUPPORTS_VIA( INodeDistributableIntervention )
    END_QUERY_INTERFACE_BODY( TravelRestriction )

    IMPLEMENT_FACTORY_REGISTERED( TravelRestriction )

    TravelRestriction::TravelRestriction()
        : BaseNodeIntervention()
        , connections_multiplier_inbound(0.0f)
        , connections_multiplier_outbound(0.0f)
        , age(0.0f)
        , duration(0.0f)
    { }

    TravelRestriction::TravelRestriction(const TravelRestriction& original)
        : BaseNodeIntervention(original)
        , connections_multiplier_inbound(original.connections_multiplier_inbound)
        , connections_multiplier_outbound(original.connections_multiplier_outbound)
        , age(original.age)
        , duration(original.duration)
    { }

    TravelRestriction::~TravelRestriction()
    { }

    bool TravelRestriction::Configure(const Configuration* inputJson)
    {
        initConfigTypeMap( "Multiplier_Inbound",    &connections_multiplier_inbound,   TRIV_Multiplier_Inbound_DESC_TEXT,   0.0f,    1.0f,    1.0f );
        initConfigTypeMap( "Multiplier_Outbound",   &connections_multiplier_outbound,  TRIV_Multiplier_Outbound_DESC_TEXT,  0.0f, FLT_MAX,    1.0f );
        initConfigTypeMap( "Duration",              &duration,                         TRIV_Duration_DESC_TEXT,             0.0f, FLT_MAX, FLT_MAX );

        return BaseNodeIntervention::Configure( inputJson );
    }

    bool TravelRestriction::Distribute(INodeEventContext* context, IEventCoordinator2* pEC)
    {
        bool distributed = BaseNodeIntervention::Distribute(context, pEC);

        return distributed;
    }

    void TravelRestriction::Update(float dt)
    {
        age += dt;

        if(age > duration)
        {
            expired = true;
        }
        else
        {
            parent->UpdateConnectionModifiers(connections_multiplier_inbound, connections_multiplier_outbound);
        }
    }

    void TravelRestriction::serialize(IArchive& ar, TravelRestriction* obj)
    {
        BaseNodeIntervention::serialize(ar, obj);

        TravelRestriction& restrictions_obj = *obj;

        ar.labelElement("connections_multiplier_inbound")     & restrictions_obj.connections_multiplier_inbound;
        ar.labelElement("connections_multiplier_outbound")    & restrictions_obj.connections_multiplier_outbound;
        ar.labelElement("age")                                & restrictions_obj.age;
        ar.labelElement("duration")                           & restrictions_obj.duration;
    }
}
