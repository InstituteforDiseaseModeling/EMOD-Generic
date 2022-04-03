/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include <vector>
#include "ISupports.h"
#include "IRelationshipManager.h"
#include "ISociety.h"
#include "TransmissionGroupMembership.h"

using namespace std;

namespace Kernel
{
    class INodeSTI : public ISupports
    {
    public:
        virtual /*const?*/ IRelationshipManager* GetRelationshipManager() /*const?*/ = 0;
        virtual void GetGroupMembershipForIndividual_STI( const std::map<std::string, uint32_t>& properties, std::map< int, TransmissionGroupMembership_t>& membershipOut ) = 0;
        virtual ISociety* GetSociety() = 0;
    };
}
