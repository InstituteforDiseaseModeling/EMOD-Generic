/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "TransmissionGroupsBase.h"
#include "IContagionPopulation.h"
#include "Types.h"

using namespace std;

namespace Kernel
{
    class RANDOMBASE;
    typedef map<uint32_t, float>         GenomeMap_t;
    typedef vector<GenomeMap_t>          GroupGenomeMap_t;
    typedef vector<GroupGenomeMap_t>     CladeGroupGenomeMap_t;

    class StrainAwareTransmissionGroups : protected TransmissionGroupsBase
    {
    public:
        StrainAwareTransmissionGroups( RANDOMBASE* prng );
        virtual ~StrainAwareTransmissionGroups() {}

    protected:

        RANDOMBASE* pRNG;
        PropertyToValuesMap_t propertyToValuesMap;
        ScalingMatrix_t scalingMatrix;
        float contagionDecayRate;
        float populationSize;
        vector<float> populationSizeByGroup;

        // Function names are lower case to differentiate from externally visible methods.
        void addPropertyValueListToPropertyToValueMap( const string& property, const PropertyValueList_t& values );
        void buildScalingMatrix( void );
        void allocateAccumulators( NaturalNumber numberOfClades, NaturalNumber numberOfGenomes );

        inline int getGroupCount() { return scalingMatrix.size(); }

        int cladeCount;
        int genomeCount;
        bool normalizeByTotalPopulation;
        vector<bool> cladeWasShed;                // Contagion of this cladeId was shed this cycle.
        vector<set<unsigned int>> genomeWasShed;  // Contagion of this cladeId/genomeId was shed this cycle.

        vector<ContagionAccumulator_t> newlyDepositedContagionByCladeAndGroup;       // All clade (genome summed) shed this timestep
        vector<ContagionAccumulator_t> currentContagionByCladeAndSourceGroup;        // All clade (genome summed) current contagion (by contagion source)
        vector<ContagionAccumulator_t> currentContagionByCladeAndDestinationGroup;   // All clade (genome summed) current contagion (by contagion destination)
        vector<ContagionAccumulator_t> forceOfInfectionByCladeAndGroup;              // All clade (genome summed) force of infection (current contagion normalized)
        CladeGroupGenomeMap_t newContagionByCladeGroupAndGenome;
        CladeGroupGenomeMap_t currentContagionByCladeSourceGroupAndGenome;
        CladeGroupGenomeMap_t currentContagionByCladeDestinationGroupAndGenome;
        CladeGroupGenomeMap_t forceOfInfectionByCladeGroupAndGenome;

        std::string tag;

        class GenomePopulationImpl : IContagionPopulation
        {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        public: 
            GenomePopulationImpl(RANDOMBASE* prng, int _cladeId, float _quantity, const GenomeMap_t& _genomeDistribution);
                
        private:
            // IContagionPopulation implementation
            RANDOMBASE* pRNG;
            virtual std::string GetName() const override;
            virtual int GetCladeID() const override;
            virtual int GetGeneticID() const override;
            virtual void SetCladeID(int cladeID) override;
            virtual void SetGeneticID(int geneticID) override;
            virtual float GetTotalContagion() const override;
            virtual void ResolveInfectingStrain( IStrainIdentity* strainId ) const override;

            int   cladeId;
            float contagionQuantity;
            const GenomeMap_t genomeDistribution;
        };

    private:

        // ITransmissionGroups
        virtual void AddProperty(const string& property, const PropertyValueList_t& values, const ScalingMatrix_t& scalingMatrix) override;
        virtual void Build(float contagionDecayRate, int numberOfClades = 1, int numberOfGenomes = 1) override;
        virtual void GetGroupMembershipForProperties(const tProperties& properties, TransmissionGroupMembership_t& membershipOut ) const override;
        virtual void UpdatePopulationSize(const TransmissionGroupMembership_t& transmissionGroupMembership, float size_changes, float mc_weight) override;
        virtual void DepositContagion(const IStrainIdentity& strain, float amount, TransmissionGroupMembership_t transmissionGroupMembership) override;
        virtual void ExposeToContagion(IInfectable* candidate, TransmissionGroupMembership_t transmissionGroupMembership, float deltaTee, TransmissionRoute::Enum tx_route) const override;
        virtual void CorrectInfectivityByGroup(float infectivityCorrection, TransmissionGroupMembership_t transmissionGroupMembership) override;
        virtual void EndUpdate(float infectivityMultiplier = 1.0f, float InfectivityAddition = 0.0f ) override;
        virtual float GetContagionByProperty( const IPKeyValue& property_value ) override;

        virtual void UseTotalPopulationForNormalization() override { normalizeByTotalPopulation = true; }
        virtual void UseGroupPopulationForNormalization() override { normalizeByTotalPopulation = false; }

        virtual void SetTag( const std::string& tag ) override   { this->tag = tag; }
        virtual const std::string& GetTag( void ) const override { return tag; }

        virtual float GetTotalContagion( void ) override;                                           // Return total contagion.
        virtual float GetTotalContagionForGroup( TransmissionGroupMembership_t group ) override;    // Return total contagion for given membership.
// NOTYET        virtual float GetTotalContagionForProperties( const IPKeyValueContainer& property_value ) override;             // Return total contagion on for given properties (maps to membership).
    };
}
