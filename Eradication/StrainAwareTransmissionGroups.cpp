/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "StrainAwareTransmissionGroups.h"
#include "Exceptions.h"

// These includes are required to bring in randgen
#include "Environment.h"
//#include "Contexts.h"
#include "RANDOM.h"

#include "Log.h"
#include "Debug.h"
#include "StrainIdentity.h"
#include "SimulationConfig.h"
#include <numeric>

SETUP_LOGGING( "StrainAwareTransmissionGroups" )

namespace Kernel
{
    StrainAwareTransmissionGroups::StrainAwareTransmissionGroups( RANDOMBASE* prng )
        : pRNG( prng )
        , propertyToValuesMap()
        , scalingMatrix()
        , contagionDecayRate( 1.0f )
        , populationSize( 0.0f )
        , populationSizeByGroup()
        , cladeCount(0)
        , genomeCount(0)
        , normalizeByTotalPopulation(true)
        , cladeWasShed()
        , genomeWasShed()
        , newlyDepositedContagionByCladeAndGroup()
        , currentContagionByCladeAndSourceGroup()
        , currentContagionByCladeAndDestinationGroup()
        , forceOfInfectionByCladeAndGroup()
        , newContagionByCladeGroupAndGenome()
        , currentContagionByCladeSourceGroupAndGenome()
        , currentContagionByCladeDestinationGroupAndGenome()
        , forceOfInfectionByCladeGroupAndGenome()
        , tag("contact")
    {
    }

    void StrainAwareTransmissionGroups::AddProperty( const string& property, const PropertyValueList_t& values, const ScalingMatrix_t& scalingMatrix )
    {
        LOG_DEBUG_F( "Adding property %s\n", property.c_str() );
        checkForDuplicatePropertyName(property);
        checkForValidValueListSize(values);
        checkForValidScalingMatrixSize(scalingMatrix, values);

        addPropertyValueListToPropertyToValueMap(property, values);
        addScalingMatrixToPropertyToMatrixMap(property, scalingMatrix);
    }

    void StrainAwareTransmissionGroups::addPropertyValueListToPropertyToValueMap( const string& property, const PropertyValueList_t& values )
    {
        propertyToValuesMap[property] = values;
    }

    void StrainAwareTransmissionGroups::buildScalingMatrix( void )
    {
        LOG_DEBUG_F( "%s\n", __FUNCTION__ );

        ScalingMatrix_t cumulativeMatrix;
        InitializeCumulativeMatrix(cumulativeMatrix);

        // For each property, aggregate the propertyMatrix with the cumulative scaling matrix
        for (const auto& pair : propertyToValuesMap)
        {
            const string& propertyName = pair.first;
            const PropertyValueList_t& valueList = pair.second;
            addPropertyValuesToValueToIndexMap(propertyName, valueList, cumulativeMatrix.size());

            AggregatePropertyMatrixWithCumulativeMatrix(propertyNameToMatrixMap[propertyName], cumulativeMatrix);
        }

        scalingMatrix = cumulativeMatrix;
    }

    // EVERYTHING UP TO HERE WAS SETUP. AFTER HERE IS RUNTIME STUFF.

    /* 
     * The goal here is to create and return a map of route indices to group indices. So each route,
     * e.g., environmental and contact, represented by an id, say 1 and 0, will map to property "ids",
     * aka group ids, like 0-10.
     */
    void
    StrainAwareTransmissionGroups::GetGroupMembershipForProperties( const tProperties& properties, TransmissionGroupMembership_t& membershipOut ) const
    {
        // Start at 0. Will, potentially, modify below based on property values and their index offsets.
        membershipOut.group = 0;

        for( const auto& entry : properties )
        {
            const string& propertyName = entry.first;
            if( propertyNameToMatrixMap.find( propertyName ) != propertyNameToMatrixMap.end() )
            {
                if( propertyValueToIndexMap.find( propertyName ) != propertyValueToIndexMap.end() &&
                    propertyValueToIndexMap.at( propertyName ).find( entry.second ) != propertyValueToIndexMap.at( propertyName ).end() )
                {
                    const string& propertyValue = entry.second;
                    size_t offset = propertyValueToIndexMap.at( propertyName ).at( propertyValue );
                    LOG_VALID_F( "Increasing/setting tx group membership for (route) index 0 by %d.\n", offset );
                    LOG_DEBUG_F( "Increasing tx group membership for (route) index 0 (property name=%s, value=%s) by %d.\n", propertyName.c_str(), propertyValue.c_str(), offset );
                    membershipOut.group += offset;
                }
                else
                {
                    // TBD: Actually there are 2 possible map failures and we should add code to be specific so we get right one.
                    throw BadMapKeyException( __FILE__, __LINE__, __FUNCTION__, "propertyValueToIndexMap", propertyName.c_str() );
                }
            }
            else
            {
                LOG_DEBUG_F( "Property %s not used in HINT configuration.\n", propertyName.c_str() );
            }
        }
        //LOG_DEBUG_F( "membership returning %p\n", membershipOut );
    }

    void StrainAwareTransmissionGroups::UpdatePopulationSize(const TransmissionGroupMembership_t& membership, float size_changes, float mc_weight)
    {
        float delta = size_changes * mc_weight;
        populationSize += delta;
        populationSizeByGroup[membership.group] += delta;
    }

    void StrainAwareTransmissionGroups::Build(float contagionDecayRate, int numberOfClades, int numberOfGenomes)
    {
        buildScalingMatrix();
        this->contagionDecayRate = contagionDecayRate;
        allocateAccumulators(numberOfClades, numberOfGenomes);

        LOG_DEBUG_F("Built %d groups with %d clades and %d genomes.\n", getGroupCount(), numberOfClades, numberOfGenomes);
    }

    void StrainAwareTransmissionGroups::allocateAccumulators( NaturalNumber numberOfClades, NaturalNumber numberOfGenomes )
    {
        LOG_VALID( "AllocateAccumulators called.\n" );
        cladeCount  = numberOfClades;
        genomeCount = numberOfGenomes;

        cladeWasShed.resize(cladeCount);
        genomeWasShed.resize(cladeCount);

        newlyDepositedContagionByCladeAndGroup.resize(cladeCount);
        currentContagionByCladeAndSourceGroup.resize(cladeCount);
        currentContagionByCladeAndDestinationGroup.resize(cladeCount);
        forceOfInfectionByCladeAndGroup.resize(cladeCount);
        newContagionByCladeGroupAndGenome.resize(cladeCount);
        currentContagionByCladeSourceGroupAndGenome.resize(cladeCount);
        currentContagionByCladeDestinationGroupAndGenome.resize(cladeCount);
        forceOfInfectionByCladeGroupAndGenome.resize(cladeCount);
        for (size_t iClade = 0; iClade < cladeCount; ++iClade)
        {
            size_t groupCount = getGroupCount();
            newlyDepositedContagionByCladeAndGroup[iClade].resize(groupCount);
            currentContagionByCladeAndSourceGroup[iClade].resize(groupCount);
            currentContagionByCladeAndDestinationGroup[iClade].resize(groupCount);
            forceOfInfectionByCladeAndGroup[iClade].resize(groupCount);
            newContagionByCladeGroupAndGenome[iClade].resize(groupCount);
            currentContagionByCladeSourceGroupAndGenome[iClade].resize(groupCount);
            currentContagionByCladeDestinationGroupAndGenome[iClade].resize(groupCount);
            forceOfInfectionByCladeGroupAndGenome[iClade].resize(groupCount);
        }

        populationSizeByGroup.resize(getGroupCount());
    }

    void StrainAwareTransmissionGroups::DepositContagion(const IStrainIdentity& strain, float amount, TransmissionGroupMembership_t membership)
    {
        if ( amount > 0 )
        {
            int iClade    = strain.GetCladeID();
            int genomeId  = strain.GetGeneticID();

            if ( iClade >= cladeCount )
            {
                ostringstream msg;
                msg << "Strain clade ID (" << iClade << ") >= configured number of clades (" << cladeCount << ").\n";
                throw new OutOfRangeException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str(), float(iClade), float(cladeCount) );
            }

            cladeWasShed[iClade]                = true;
            genomeWasShed[iClade].insert(genomeId);

            GroupIndex iGroup = membership.group;
            newlyDepositedContagionByCladeAndGroup[iClade][iGroup] += amount;
            newContagionByCladeGroupAndGenome[iClade][iGroup][genomeId] += amount;

            LOG_VALID_F( "(%s) DepositContagion (clade = %d, route = 0, group = %d [genome = %d]) increased by %f\n",
                            tag.c_str(),
                            iClade,
                            iGroup,
                            genomeId,
                            amount );
        }
    }

    void StrainAwareTransmissionGroups::ExposeToContagion(IInfectable* candidate, TransmissionGroupMembership_t membership, float deltaTee, TransmissionRoute::Enum txRoute) const
    {
        //LOG_DEBUG_F( "ExposeToContagion\n" );
        for (int iClade = 0; iClade < cladeCount; iClade++)
        {
            float forceOfInfection = 0.0f;
            const ContagionAccumulator_t& forceOfInfectionByGroup = forceOfInfectionByCladeAndGroup[iClade];

            size_t iGroup = membership.group;
            forceOfInfection = forceOfInfectionByGroup[iGroup];

            if ((forceOfInfection > 0) && (candidate != nullptr))
            {
                LOG_DEBUG_F("ExposureToContagion: [Clade:%d] Route:0, Group:%d, exposure qty = %f\n", iClade, iGroup, forceOfInfection );
                GenomePopulationImpl contagionPopulation(pRNG, iClade, forceOfInfection, forceOfInfectionByCladeGroupAndGenome[iClade][iGroup]);
                candidate->Expose((IContagionPopulation*)&contagionPopulation, deltaTee, txRoute );
            }
        }
    }

    float StrainAwareTransmissionGroups::GetContagionByProperty(const IPKeyValue& property_value)
    {
        std::vector<size_t> indices;
        getGroupIndicesForProperty( property_value, propertyToValuesMap, indices );
        float total = 0.0f;
        for (size_t iClade = 0; iClade < cladeCount; ++iClade)
        {
            const auto& contagion = forceOfInfectionByCladeAndGroup[iClade];
            total += std::accumulate(indices.begin(), indices.end(), 0.0f, [&](float init, size_t index) { return init + contagion[index]; });
        }

        return total;
    }

    void StrainAwareTransmissionGroups::CorrectInfectivityByGroup(float infectivityMultiplier, TransmissionGroupMembership_t membership)
    {
        // By clade (genomes aggregated)
        for (int iClade = 0; iClade < cladeCount; iClade++)
        {
            int iGroup = membership.group;
            LOG_DEBUG_F("CorrectInfectivityByGroup: [Clade:%d] Route:0, Group:%d, ContagionBefore = %f, infectivityMultiplier = %f\n", iClade, iGroup, newlyDepositedContagionByCladeAndGroup[iClade][iGroup], infectivityMultiplier);
            newlyDepositedContagionByCladeAndGroup[iClade][iGroup] *= infectivityMultiplier;
            LOG_DEBUG_F("CorrectInfectivityByGroup: [Clade:%d] Route:0, Group:%d, ContagionAfter = %f\n", iClade, iGroup, newlyDepositedContagionByCladeAndGroup[iClade][iGroup]);
        }

        // By individual genome
        for (int iClade = 0; iClade < cladeCount; iClade++)
        {
            GroupGenomeMap_t& shedClade = newContagionByCladeGroupAndGenome[iClade];
            int iGroup = membership.group;
            for (auto& entry : shedClade[iGroup])
            {
                uint32_t genomeId = entry.first;
                LOG_DEBUG_F("CorrectInfectivityByGroup: [Clade:%d][Route:0][Group:%d][Genome:%d], ContagionBefore = %f, infectivityMultiplier = %f\n", iClade, iGroup, genomeId, shedClade[iGroup][genomeId], infectivityMultiplier);
                entry.second *= infectivityMultiplier;
                LOG_DEBUG_F("CorrectInfectivityByGroup: [Clade:%d][Route:0][Group:%d][Genome:%d], ContagionAfter  = %f\n", iClade, iGroup, genomeId, shedClade[iGroup][genomeId]);
            }
        }
    }

    void StrainAwareTransmissionGroups::EndUpdate( float infectivityMultiplier, float infectivityAddition )
    {
        LOG_VALID_F( "(%s) Enter (%s)\n", tag.c_str(), __FUNCTION__ );

        float additionalContagion = infectivityAddition;

        if ( (infectivityAddition != 0.0f) &&
             ((getGroupCount() > 1) || (cladeCount > 1) || (genomeCount > 1)) )
        {
            LOG_WARN_F( "StrainAwareTransmissionGroups::EndUpdate() infectivityAddition != 0 (%f actual), but one or more of # HINT groups (%d), clade count (%d), or genome count (%d) is > 1. Using 0 for additional contagion.\n",
                        infectivityAddition, getGroupCount(), cladeCount, genomeCount );
            additionalContagion = 0.0f;
        }

        for (size_t iClade = 0; iClade < cladeCount; ++iClade)
        {
            size_t groupCount = getGroupCount();
            for (size_t iGroup = 0; iGroup < groupCount; ++iGroup)
            {
                LOG_VALID_F( "(%s) New contagion (clade = %d, route = 0, group = %d) = %f\n", tag.c_str(), iClade, iGroup, newlyDepositedContagionByCladeAndGroup[iClade][iGroup] );
                for (const auto& entry : newContagionByCladeGroupAndGenome[iClade][iGroup])
                {
                    LOG_VALID_F( "(%s) New contagion (clade = %d, route = 0, group = %d, genome = %d) = %f\n", tag.c_str(), iClade, iGroup, entry.first, entry.second );
                }
                LOG_VALID_F( "(%s) Current contagion [source] (clade = %d, route = 0, group = %d) = %f\n", tag.c_str(), iClade, iGroup, currentContagionByCladeAndSourceGroup[iClade][iGroup] );
                for (const auto& entry : currentContagionByCladeSourceGroupAndGenome[iClade][iGroup])
                {
                    LOG_VALID_F( "(%s) Current contagion [source] (clade = %d, route = 0, group = %d, genome = %d) = %f\n", tag.c_str(), iClade, iGroup, entry.first, entry.second );
                }
            }
        }

        // For each clade...
        for (size_t iClade = 0; iClade < cladeCount; ++iClade)
        {
            auto& refCurrentContagionForCladeBySourceGroup                  = currentContagionByCladeAndSourceGroup[iClade];
            auto& refCurrentContagionForCladeByDestinationGroup             = currentContagionByCladeAndDestinationGroup[iClade];
            auto& refCurrentContagionForCladeBySourceGroupAndGenome         = currentContagionByCladeSourceGroupAndGenome[iClade];
            auto& refCurrentContagionForCladeByDestinationGroupAndGenome    = currentContagionByCladeDestinationGroupAndGenome[iClade];
            auto& refNewlyDepositedContagionForCladeByGroup                 = newlyDepositedContagionByCladeAndGroup[iClade];
            auto& refForceOfInfectionForCladeByGroup                        = forceOfInfectionByCladeAndGroup[iClade];
            auto& refNewContagionForCladeByGroupAndGenome                   = newContagionByCladeGroupAndGenome[iClade];
            auto& refForceOfInfectionForCladeByGroupAndGenome               = forceOfInfectionByCladeGroupAndGenome[iClade];

            // Decay previously accumulated contagion and add in newly deposited contagion for each source group:
            float decayFactor = 1.0f - contagionDecayRate;
            LOG_VALID_F ( "(%s) Decay rate for route 0 = %f => decay factor = %f\n", tag.c_str(), contagionDecayRate, decayFactor );
            vectorScalarMultiply( refCurrentContagionForCladeBySourceGroup, decayFactor );
            vectorElementAdd( refCurrentContagionForCladeBySourceGroup, refNewlyDepositedContagionForCladeByGroup );
            vectorScalarAdd( refCurrentContagionForCladeBySourceGroup, additionalContagion );

            // We just added this to the current contagion accumulator. Clear it out.
            size_t groupCount = getGroupCount();
            memset( refNewlyDepositedContagionForCladeByGroup.data(), 0, groupCount * sizeof(float) );

            // Current contagion (by group which deposited it) is up to date and new contagion by group is reset.

            // Update accumulated contagion by destination group:
            for (size_t iGroup = 0; iGroup < groupCount; ++iGroup)
            {
                const MatrixRow_t& betaVector = scalingMatrix[iGroup];
                float accumulatedContagion = vectorDotProduct( refCurrentContagionForCladeBySourceGroup, betaVector );
                LOG_VALID_F("(%s) Adding %f to %f contagion [clade:%d,route:0,group:%d]\n", tag.c_str(), accumulatedContagion, 0.0f, iClade, iGroup);
                refCurrentContagionForCladeByDestinationGroup[iGroup] = accumulatedContagion;
            }

            // Current contagion (by receiving group) is up to date (based on current contagion from source groups).

            for (size_t iGroup = 0; iGroup < groupCount; ++iGroup)
            {
                // Update effective contagion (force of infection) by destination group:
                float population = (normalizeByTotalPopulation ? populationSize : populationSizeByGroup[iGroup]);
                float normalization = ((population == 0.0f) ? 0.0f : 1.0f/population);
                LOG_VALID_F( "(%s) Normalization (%s) for group %d is %f based on population %f\n", tag.c_str(), (normalizeByTotalPopulation ? "total population" : "group population"), iGroup, normalization, population);
                LOG_VALID_F( "(%s) Contagion for [clade:%d,route:0] population scaled by %f\n", tag.c_str(), iClade, population);

                refForceOfInfectionForCladeByGroup[iGroup] = refCurrentContagionForCladeByDestinationGroup[iGroup]
                                                                * infectivityMultiplier
                                                                * normalization;
                LOG_VALID_F("(%s) Normalized contagion for group %d is %f and seasonally scaled by %f\n", tag.c_str(), iGroup, float(refCurrentContagionForCladeByDestinationGroup[iGroup]) * normalization, infectivityMultiplier);
            }

            // Force of infection for each group (current contagion * correction * normalization) is up to date.

            for (size_t iGroup = 0; iGroup < groupCount; ++iGroup)
            {
                // Decay previously accumulated contagion and add in newly deposited contagion by genome
                auto& refCurrentContagionForCladeAndSourceGroupByGenome    = refCurrentContagionForCladeBySourceGroupAndGenome[iGroup];
                auto& refNewContagionForCladeAndGroupByGenome              = refNewContagionForCladeByGroupAndGenome[iGroup];

                // Decay previously accumulated contagion:
                if ( decayFactor > 0.0f )
                {
                    for (auto& entry : refCurrentContagionForCladeAndSourceGroupByGenome)
                    {
                        // entry = pair<genomeId, contagion>
                        entry.second *= decayFactor;
                    }
                }
                else
                {
                    refCurrentContagionForCladeAndSourceGroupByGenome.clear();
                }

                // Current contagion (by source group) has been decayed.

                // Add in newly deposited contagion:
                for (auto& entry : refNewContagionForCladeAndGroupByGenome)
                {
                    auto genomeId    = entry.first;
                    auto contagion   = entry.second;
                    refCurrentContagionForCladeAndSourceGroupByGenome[genomeId] += (contagion + additionalContagion);
                }

                // Current contagion, per genome, (by source group) has been updated from new contagion, per genome, (by source group)

                // We just added this to the current genome contagion accumulator, clear it out.
                refNewContagionForCladeAndGroupByGenome.clear();
            }

            for (size_t iGroup = 0; iGroup < groupCount; ++iGroup)
            {
                auto& refCurrentContagionForCladeAndDestinationGroupByGenome = refCurrentContagionForCladeByDestinationGroupAndGenome[iGroup];

                const MatrixRow_t& betaVector = scalingMatrix[iGroup];

                // We are going to transfer accumulated contagion to this structure, clear it out.
                refCurrentContagionForCladeAndDestinationGroupByGenome.clear();

                // Update accumulated contagion, by genome, indexed by destination group:
                for (size_t srcGroup = 0; srcGroup < groupCount; ++srcGroup)
                {
                    float beta = betaVector[srcGroup];
                    for (const auto& entry : refCurrentContagionForCladeBySourceGroupAndGenome[srcGroup])
                    {
                        auto genome    = entry.first;
                        auto contagion = entry.second;
                        refCurrentContagionForCladeAndDestinationGroupByGenome[genome] += contagion * beta;
                    }
                }

                // Current contagion, per genome, (by receiving group), has been updated (based on current contagion from source groups).

                // Update effective contagion (force of infection):
                auto& refForceOfInfectionForCladeAndGroupByGenome = refForceOfInfectionForCladeByGroupAndGenome[iGroup];
                // We are going to set current force of infection from current accumulated contagion, clear it out.
                refForceOfInfectionForCladeAndGroupByGenome.clear();

                float population = (normalizeByTotalPopulation ? populationSize : populationSizeByGroup[iGroup]);
                float normalization = ((population == 0.0f) ? 0.0f : 1.0f/population);

                for (const auto& entry : refCurrentContagionForCladeAndDestinationGroupByGenome)
                {
                    auto genome    = entry.first;
                    auto contagion = entry.second;
                    refForceOfInfectionForCladeAndGroupByGenome[genome] = contagion * infectivityMultiplier * normalization;
                }

                // Force of infection, per genome, for each group (current contagion * correction * normalization) is up to date.
            }
        }

        for (size_t iClade = 0; iClade < cladeCount; ++iClade)
        {
            size_t groupCount = getGroupCount();
            for (size_t iGroup = 0; iGroup < groupCount; ++iGroup)
            {
                LOG_VALID_F( "(%s) Current contagion [dest] (clade = %d, route = 0, group = %d) = %f\n", tag.c_str(), iClade, iGroup, currentContagionByCladeAndDestinationGroup[iClade][iGroup] );
                for (const auto& entry : currentContagionByCladeDestinationGroupAndGenome[iClade][iGroup])
                {
                    LOG_VALID_F( "(%s) Current contagion [dest] (clade = %d, route = 0, group = %d, genome = %d) = %f\n", tag.c_str(), iClade, iGroup, entry.first, entry.second );
                }
                LOG_VALID_F( "(%s) Force of infection (clade = %d, route = 0, group = %d) = %f\n", tag.c_str(), iClade, iGroup, forceOfInfectionByCladeAndGroup[iClade][iGroup] );
                for (const auto& entry : forceOfInfectionByCladeGroupAndGenome[iClade][iGroup])
                {
                    LOG_VALID_F( "(%s) Force of infection (clade = %d, route = 0, group = %d, genome = %d) = %f\n", tag.c_str(), iClade, iGroup, entry.first, entry.second );
                }
            }
        }

        LOG_VALID_F( "(%s) Exit %s\n", tag.c_str(), __FUNCTION__ );
    }

    float StrainAwareTransmissionGroups::GetTotalContagion( void )
    {
        float contagion = 0.0f;

        for (auto& forceOfInfectionForCladeByGroup : forceOfInfectionByCladeAndGroup)
        {
            for (float forceOfInfectionForCladeAndGroup : forceOfInfectionForCladeByGroup)
            {
                contagion += forceOfInfectionForCladeAndGroup;
            }
        }

        return contagion;
    }

    float StrainAwareTransmissionGroups::GetTotalContagionForGroup( TransmissionGroupMembership_t membership )
    {
        float contagion = 0.0f;

        for (auto& forceOfInfectionForCladeByGroup : forceOfInfectionByCladeAndGroup)
        {
            float forceOfInfection = forceOfInfectionForCladeByGroup[ membership.group ];
            contagion += forceOfInfection;
        }

        return contagion;
    }

// NOTYET    float StrainAwareTransmissionGroups::GetTotalContagionForProperties( const IPKeyValueContainer& property_value )
// NOTYET    {
// NOTYET        TransmissionGroupMembership_t txGroups;
// NOTYET        tProperties properties;
// NOTYET        // Convert key:values in IPKeyValueContainer to tProperties
// NOTYET        GetGroupMembershipForProperties( routeIndexToNameMap, properties, &txGroups );
// NOTYET
// NOTYET        return GetTotalContagionForGroup( txGroups );
// NOTYET    }

    BEGIN_QUERY_INTERFACE_BODY(StrainAwareTransmissionGroups::GenomePopulationImpl)
    END_QUERY_INTERFACE_BODY(StrainAwareTransmissionGroups::GenomePopulationImpl)

    StrainAwareTransmissionGroups::GenomePopulationImpl::GenomePopulationImpl(
        RANDOMBASE* prng,
    int   _cladeId,
    float _quantity,
    const GenomeMap_t& _genomeDistribution
    )
        : pRNG( prng )
        , cladeId(_cladeId)
        , contagionQuantity(_quantity)
        , genomeDistribution(_genomeDistribution)
    {
    }

    std::string StrainAwareTransmissionGroups::GenomePopulationImpl::GetName( void ) const
    {
        // Never valid code path, have to implement this method due to interface.
        throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "Not valid for GenomePopulationImpl" );
    }

    int StrainAwareTransmissionGroups::GenomePopulationImpl::GetCladeID( void ) const
    {
        return cladeId;
    }

    int StrainAwareTransmissionGroups::GenomePopulationImpl::GetGeneticID( void ) const
    {
        // Never valid code path, have to implement this method due to interface.
        throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "Not valid for GenomePopulationImpl" );
    }

    void StrainAwareTransmissionGroups::GenomePopulationImpl::SetCladeID( int cladeID )
    {
        // Never valid code path, have to implement this method due to interface.
        throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "Not valid for GenomePopulationImpl" );
    }

    void StrainAwareTransmissionGroups::GenomePopulationImpl::SetGeneticID( int geneticID )
    {
        // Never valid code path, have to implement this method due to interface.
        throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "Not valid for GenomePopulationImpl" );
    }

    float StrainAwareTransmissionGroups::GenomePopulationImpl::GetTotalContagion( void ) const
    {
        return contagionQuantity;
    }

    void StrainAwareTransmissionGroups::GenomePopulationImpl::ResolveInfectingStrain( IStrainIdentity* strainId ) const
    {
        LOG_VALID_F( "%s\n", __FUNCTION__ );
        float totalRawContagion = 0.0f;
        for (auto& entry : genomeDistribution)
        {
            totalRawContagion += entry.second;
        }

        if (totalRawContagion == 0.0f) {
            LOG_WARN_F( "Found no raw contagion for clade=%d (%f total contagion)\n", cladeId, contagionQuantity);
        }

        float rand = pRNG->e();
        float target = totalRawContagion * rand;
        float contagionSeen = 0.0f;
        int   genomeId = 0;

        strainId->SetCladeID(cladeId);

        for (auto& entry : genomeDistribution)
        {
            float contagion = entry.second;
            if (contagion > 0.0f)
            {
                genomeId = entry.first;
                contagionSeen += contagion;
                if (contagionSeen >= target)
                {
                    LOG_DEBUG_F( "Selected strain genetic id %d\n", genomeId );
                    strainId->SetGeneticID(genomeId); // ????
                    return;
                }
            }
        }

        LOG_WARN_F( "Ran off the end of the distribution (rounding error?). Using last valid genome we saw: %d\n", genomeId );
        strainId->SetGeneticID(genomeId);
    }
}
