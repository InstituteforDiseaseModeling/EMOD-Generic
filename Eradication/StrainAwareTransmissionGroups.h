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
    typedef map<uint64_t, float>         GenomeMap_t;
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
        void allocateAccumulators( uint32_t numberOfClades, uint64_t numberOfGenomes );

        inline int getGroupCount() { return scalingMatrix.size(); }

        uint32_t cladeCount;
        uint64_t genomeCount;
        bool normalizeByTotalPopulation;
        vector<bool> cladeWasShed;                // Contagion of this cladeId was shed this cycle.
        vector<set<uint64_t>> genomeWasShed;  // Contagion of this cladeId/genomeId was shed this cycle.

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
            GenomePopulationImpl(RANDOMBASE* prng, uint32_t _cladeId, float _quantity, const GenomeMap_t& _genomeDistribution);
                
        private:
            // IContagionPopulation implementation
            RANDOMBASE* pRNG;
            virtual std::pair<uint32_t, uint64_t> GetStrainName(void) const override;
            virtual uint32_t GetCladeID() const override;
            virtual uint64_t GetGeneticID() const override;
            virtual void SetCladeID(uint32_t cladeID) override;
            virtual void SetGeneticID(uint64_t geneticID) override;
            virtual float GetTotalContagion() const override;
            virtual void ResolveInfectingStrain( IStrainIdentity* strainId ) const override;

            uint32_t   cladeId;
            float contagionQuantity;
            const GenomeMap_t genomeDistribution;
        };

    private:

        // ITransmissionGroups
        virtual void AddProperty(const string& property, const PropertyValueList_t& values, const ScalingMatrix_t& scalingMatrix) override;
        virtual void Build(float contagionDecayRate, uint32_t numberOfClades = 1, uint64_t numberOfGenomes = 1) override;
        virtual void ChangeMatrix(const string& propertyName, const ScalingMatrix_t& newScalingMatrix) override;
        virtual void GetGroupMembershipForProperties(const tProperties& properties, TransmissionGroupMembership_t& membershipOut ) const override;
        virtual void UpdatePopulationSize(const TransmissionGroupMembership_t& transmissionGroupMembership, float size_changes, float mc_weight) override;
        virtual void DepositContagion(const IStrainIdentity& strain, float amount, TransmissionGroupMembership_t transmissionGroupMembership) override;
        virtual void ExposeToContagion(IInfectable* candidate, TransmissionGroupMembership_t transmissionGroupMembership, float deltaTee, TransmissionRoute::Enum tx_route) const override;
        virtual void CorrectInfectivityByGroup(float infectivityCorrection, TransmissionGroupMembership_t transmissionGroupMembership) override;
        virtual void EndUpdate(float infectivityMultiplier = 1.0f, float InfectivityAddition = 0.0f, float infectivityOverdispersion = 0.0f) override;
        virtual float GetContagionByProperty( const IPKeyValue& property_value ) override;
        virtual void LoadSparseRepVecs(sparse_contagion_repr& inf_rep) override;

        virtual void UseTotalPopulationForNormalization() override { normalizeByTotalPopulation = true; }
        virtual void UseGroupPopulationForNormalization() override { normalizeByTotalPopulation = false; }

        virtual void SetTag( const std::string& tag ) override   { this->tag = tag; }
        virtual const std::string& GetTag( void ) const override { return tag; }

        virtual float GetTotalContagion( void ) override;                                           // Return total contagion.
        virtual float GetTotalContagionForGroup( TransmissionGroupMembership_t group ) override;    // Return total contagion for given membership.
// NOTYET        virtual float GetTotalContagionForProperties( const IPKeyValueContainer& property_value ) override;             // Return total contagion on for given properties (maps to membership).
    };
}
