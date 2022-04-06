/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "UnitTest++.h"
#include "../utils/Common.h"
#include "IdmMpi.h"
#include "RANDOM.h"
#include "TransmissionGroupsFactory.h"
#include "StrainIdentity.h"
#include "TransmissionGroupMembership.h"
#include "Infection.h"

using namespace std;
using namespace Kernel;

#define CONTACT_DECAY       (1.0f)
#define ENVIRONMENTAL_DECAY (0.5f)

SUITE(TransmissionGroupsTest)
{
    struct TransmissionGroupsTestFixture
    {
        TransmissionGroupsTestFixture()
            : risk("RISK")
            , high("HIGH")
            , medium("MEDIUM")
            , low("LOW")
            , risk_values{ high, medium, low }
            , location("LOCATION")
            , bothell("BOTHELL")
            , bellevue("BELLEVUE")
            , redmond("REDMOND")
            , renton("RENTON")
            , location_values{ bothell, bellevue, redmond, renton }
            , risk_matrix{
                MatrixRow_t{ 1, 0, 0 },
                MatrixRow_t{ 0, 1, 0 },
                MatrixRow_t{ 0, 0, 1 } }
            , location_matrix{
                MatrixRow_t{ 0.5f,   0.25f,  0.25f,  0.0f },
                MatrixRow_t{ 0.125f, 0.625f, 0.125f, 0.125f },
                MatrixRow_t{ 0.0f,   0.5f,   0.5f,   0.0f },
                MatrixRow_t{ 0.0f,   0.125f, 0.125f, 0.75f } }
        {
            /* Set up environment: */
            JsonConfigurable::ClearMissingParameters();
            Environment::Finalize();
            Environment::setLogger(new SimpleLogger(Logger::tLevel::WARNING));

            IPFactory::DeleteFactory();
            IPFactory::CreateFactory();

            std::map<std::string, float> risk_ip_values{ {high, 0.5f}, {medium, 0.25f}, {low, 0.25f} };
            IPFactory::GetInstance()->AddIP(1, risk, risk_ip_values);

            std::map<std::string, float> location_ip_values{ {bothell, 0.25f}, {bellevue, 0.25f }, {redmond, 0.25f}, {renton, 0.25f} };
            IPFactory::GetInstance()->AddIP(1, location, location_ip_values);
        }

        ~TransmissionGroupsTestFixture()
        {
            IPFactory::DeleteFactory();
            Environment::Finalize();
        }

        std::string risk, high, medium, low;
        PropertyValueList_t risk_values;

        std::string location, bothell, bellevue, redmond, renton;
        PropertyValueList_t location_values;

        ScalingMatrix_t risk_matrix;
        ScalingMatrix_t location_matrix;

        PSEUDO_DES rng;
    };

    TEST_FIXTURE(TransmissionGroupsTestFixture, TestStrainAwareTxGroupsSingleProperty)
    {
        // Instantiate StrainAwareTransmissionGroups with single property
        unique_ptr<ITransmissionGroups> txGroups( TransmissionGroupsFactory::CreateNodeGroups(TransmissionGroupType::StrainAwareGroups, &rng) );
        txGroups->AddProperty(risk, risk_values, risk_matrix);
        txGroups->Build(CONTACT_DECAY, 2, 2);

        // Deposit contagion
        StrainIdentity strain00 = StrainIdentity(0, 0);
        StrainIdentity strain01 = StrainIdentity(0, 1);
        StrainIdentity strain10 = StrainIdentity(1, 0);
        StrainIdentity strain11 = StrainIdentity(1, 1);
        TransmissionGroupMembership_t membership;

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, high } }, membership);
        txGroups->UpdatePopulationSize(membership, 2, 1.0f);
        txGroups->DepositContagion(strain00, 0.25, membership);
        txGroups->DepositContagion(strain01, 0.25, membership);
        txGroups->DepositContagion(strain10, 0.25, membership);
        txGroups->DepositContagion(strain11, 0.25, membership);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, medium } }, membership);
        txGroups->UpdatePopulationSize(membership, 1, 2.0f);
        txGroups->DepositContagion(strain00, 0.125, membership);
        txGroups->DepositContagion(strain01, 0.125, membership);
        txGroups->DepositContagion(strain10, 0.125, membership);
        txGroups->DepositContagion(strain11, 0.125, membership);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, low } }, membership);
        txGroups->UpdatePopulationSize(membership, 4, 1.0f);
        txGroups->DepositContagion(strain00, 0.0625, membership);
        txGroups->DepositContagion(strain01, 0.0625, membership);
        txGroups->DepositContagion(strain10, 0.0625, membership);
        txGroups->DepositContagion(strain11, 0.0625, membership);

        txGroups->EndUpdate();

        float contagion = txGroups->GetTotalContagion();
        CHECK_EQUAL( ((0.25f * 4) + (0.125f * 4) + (0.0625f) * 4) / (2 + 2 + 4), contagion );

        contagion = txGroups->GetContagionByProperty(IPKeyValue(risk, high));
        CHECK_EQUAL( (0.25f * 4)/8, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(risk, medium));
        CHECK_EQUAL( (0.125f * 4)/8, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(risk, low));
        CHECK_EQUAL( (0.0625f * 4)/8, contagion);

        // Groups and properties have a 1:1 relationship in this setup.
        // Don't bother checking GetTotalContagionForGroup().
    }

    TEST_FIXTURE(TransmissionGroupsTestFixture, TestStrainAwareTxGroupsMultipleProperties)
    {
        // Instantiate StrainAwareTransmissionGroups with two properties
        unique_ptr<ITransmissionGroups> txGroups( TransmissionGroupsFactory::CreateNodeGroups(TransmissionGroupType::StrainAwareGroups, &rng) );
        txGroups->AddProperty(risk, risk_values, risk_matrix);
        txGroups->AddProperty(location, location_values, location_matrix);
        txGroups->Build(CONTACT_DECAY, 2, 1);

        // Deposit contagion
        StrainIdentity strain0 = StrainIdentity(0, 0);
        StrainIdentity strain1 = StrainIdentity(1, 0);
        TransmissionGroupMembership_t membership;

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, medium },{ location, bothell } }, membership);
        txGroups->UpdatePopulationSize(membership, 8, 1.0f);
        txGroups->DepositContagion(strain0, 0.25f, membership);
        txGroups->DepositContagion(strain1, 0.75f, membership);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, low },{ location, bellevue } }, membership);
        txGroups->UpdatePopulationSize(membership, 1, 4.0f);
        txGroups->DepositContagion(strain0, 2.25f, membership);
        txGroups->DepositContagion(strain1, 0.75f, membership);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, low },{ location, redmond } }, membership);
        txGroups->UpdatePopulationSize(membership, 2, 1.0f);
        txGroups->DepositContagion(strain0, 1.25f, membership);
        txGroups->DepositContagion(strain1, 3.75f, membership);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, high },{ location, renton } }, membership);
        txGroups->UpdatePopulationSize(membership, 1, 2.0f);
        txGroups->DepositContagion(strain0, 5.25f, membership);
        txGroups->DepositContagion(strain1, 1.75f, membership);

        txGroups->EndUpdate();

        float contagion = txGroups->GetContagionByProperty(IPKeyValue(risk, high));
        CHECK_EQUAL(7.0f/16, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(risk, medium));
        CHECK_EQUAL(1.0f/16, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(risk, low));
        CHECK_EQUAL(8.0f/16, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(location, bothell));
        CHECK_EQUAL(0.875f/16, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(location, bellevue));
        CHECK_EQUAL(5.5f/16, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(location, redmond));
        CHECK_EQUAL(4.0f/16, contagion);

        contagion = txGroups->GetContagionByProperty(IPKeyValue(location, renton));
        CHECK_EQUAL(5.625f/16, contagion);

        // Check contagion by group
        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, high },{ location, bothell } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.000f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, medium },{ location, bothell } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.500f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, low },{ location, bothell } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.375f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, high },{ location, bellevue } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.875f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, medium },{ location, bellevue } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.250f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, low },{ location, bellevue } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(4.375f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, high },{ location, redmond } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.875f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, medium },{ location, redmond } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.250f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, low },{ location, redmond } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(2.875f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, high },{ location, renton } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(5.250f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, medium },{ location, renton } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.000f/16, contagion);

        txGroups->GetGroupMembershipForProperties(tProperties{ { risk, low },{ location, renton } }, membership);
        contagion = txGroups->GetTotalContagionForGroup(membership);
        CHECK_EQUAL(0.375f/16, contagion);
    }

    TEST_FIXTURE(TransmissionGroupsTestFixture, TestStrainAwareTxGroupsBadParams)
    {
        // Instantiate StrainAwareTransmissionGroups with single property
        unique_ptr<ITransmissionGroups> txGroups( TransmissionGroupsFactory::CreateNodeGroups( TransmissionGroupType::StrainAwareGroups, &rng ) );
        txGroups->AddProperty(risk, risk_values, risk_matrix);
        txGroups->Build(CONTACT_DECAY, 2, 2);
        txGroups->EndUpdate();

        // location is not a property used in this transmission setting.
        CHECK_EQUAL( 0.0f, txGroups->GetContagionByProperty( IPKeyValue(location, bothell) ) );
    }
}
