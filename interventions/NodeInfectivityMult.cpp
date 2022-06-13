/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "NodeInfectivityMult.h"
#include "NodeEventContext.h"
#include "INodeContext.h"

SETUP_LOGGING("NodeInfectivityMult")

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY( NodeInfectivityMult )
        HANDLE_INTERFACE( IConfigurable )
        HANDLE_INTERFACE( INodeDistributableIntervention )
        HANDLE_INTERFACE( IBaseIntervention )
        HANDLE_ISUPPORTS_VIA( INodeDistributableIntervention )
    END_QUERY_INTERFACE_BODY( NodeInfectivityMult )

    IMPLEMENT_FACTORY_REGISTERED( NodeInfectivityMult )

    NodeInfectivityMult::NodeInfectivityMult()
        : BaseNodeIntervention()
        , duration(0.0f)
        , tx_route(TransmissionRoute::CONTACT)
        , mult_by_duration()
    { }

    NodeInfectivityMult::NodeInfectivityMult(const NodeInfectivityMult& original)
        : BaseNodeIntervention(original)
        , duration(original.duration)
        , tx_route(original.tx_route)
        , mult_by_duration(original.mult_by_duration)
    { }

    NodeInfectivityMult::~NodeInfectivityMult()
    { }

    bool NodeInfectivityMult::Configure(const Configuration* inputJson)
    {
        initConfig("Transmission_Route",  tx_route,  inputJson, MetadataDescriptor::Enum("Transmission_Route", NIMIV_Transmission_Route_DESC_TEXT, MDD_ENUM_ARGS(TransmissionRoute)));

        initConfigTypeMap("Multiplier_By_Duration",  &mult_by_duration,  NIMIV_Multiplier_By_Duration_DESC_TEXT);

        return BaseNodeIntervention::Configure( inputJson );
    }

    bool NodeInfectivityMult::Distribute(INodeEventContext* context, IEventCoordinator2* pEC)
    {
        // Verify that transmission route is supported for current simulation
        RouteList_t tx_routes = context->GetNodeContext()->GetTransmissionRoutes();
        if(!std::count(tx_routes.begin(), tx_routes.end(), tx_route))
        {
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Transmission route not supported for current simulation type." );
        }

        bool distributed = BaseNodeIntervention::Distribute(context, pEC);

        return distributed;
    }

    void NodeInfectivityMult::Update(float dt)
    {
        duration += dt;

        if(mult_by_duration.isAtEnd(duration))
        {
            expired = true;
        }
        else
        {
            parent->UpdateInfectivityMultiplier(mult_by_duration.getValueLinearInterpolation(duration, 1.0f), tx_route);
        }
    }

    void NodeInfectivityMult::serialize(IArchive& ar, NodeInfectivityMult* obj)
    {
        BaseNodeIntervention::serialize(ar, obj);

        NodeInfectivityMult& infect_mult_obj = *obj;

        ar.labelElement("duration")           &            infect_mult_obj.duration;
        ar.labelElement("tx_route")           & (uint32_t&)infect_mult_obj.tx_route;
        ar.labelElement("mult_by_duration")   &            infect_mult_obj.mult_by_duration;
    }
}
