/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "NodeInfo.h"
#include "JsonObject.h"
#include "Serializer.h"
#include "INodeContext.h"
#include "NodeEventContext.h"

namespace Kernel
{
    NodeInfo::NodeInfo()
        : m_Suid( suids::nil_suid() )
        , m_ExternalId(0)
        , m_Rank(0)
        , m_Population(0.0f)
        , m_LongitudeDegrees(0.0f)
        , m_LatitudeDegrees(0.0f)
        , m_net_mult_in(0.0f)
        , m_net_inf_frac(0.0f)
        , m_net_inf_rep()
    { }

    NodeInfo::NodeInfo( int rank, INodeContext* pNC )
        : m_Suid( pNC->GetSuid() )
        , m_ExternalId( pNC->GetExternalID() )
        , m_Rank( rank )
        , m_Population( pNC->GetStatPop() )
        , m_LongitudeDegrees( pNC->GetLongitudeDegrees() )
        , m_LatitudeDegrees( pNC->GetLatitudeDegrees() )
        , m_net_mult_in( pNC->GetEventContext()->GetInboundConnectionModifier() )
        , m_net_inf_frac( pNC->GetNetInfectFrac() )
        , m_net_inf_rep( pNC->GetNetInfRep() )
    { }

    NodeInfo::~NodeInfo()
    { }

    void NodeInfo::Update( INodeContext* pNC )
    {
        m_Population    = pNC->GetStatPop();
        m_net_mult_in   = pNC->GetEventContext()->GetInboundConnectionModifier();
        m_net_inf_rep   = pNC->GetNetInfRep();
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!! In a scenario like 25_Madagascar that has 30000+ nodes,
    // !!! we really have to minimize the data that is being shared
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    void NodeInfo::serialize( IArchive& ar, bool firstTime )
    {
        if( firstTime )
        {
            ar.labelElement("m_Suid"             ) & m_Suid;
            ar.labelElement("m_ExternalId"       ) & m_ExternalId;
            ar.labelElement("m_Rank"             ) & m_Rank;
            ar.labelElement("m_LongitudeDegrees" ) & m_LongitudeDegrees;
            ar.labelElement("m_LatitudeDegrees"  ) & m_LatitudeDegrees;
        }

        ar.labelElement("m_Population")    & m_Population;
        ar.labelElement("m_net_mult_in")   & m_net_mult_in;
        ar.labelElement("m_net_inf_frac" ) & m_net_inf_frac;

        ar.labelElement("inf_clade" )        & m_net_inf_rep.inf_clade;
        ar.labelElement("inf_genome" )       & m_net_inf_rep.inf_genome;
        ar.labelElement("inf_group" )        & m_net_inf_rep.inf_group;
        ar.labelElement("inf_vals" )         & m_net_inf_rep.inf_vals;
    }
}
