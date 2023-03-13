/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>
#include "IAssortivity.h"
#include "IRelationship.h"
#include "Properties.h"

namespace Kernel 
{
    class RANDOMBASE;

    class IDMAPI Assortivity : public IAssortivity
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE();
    public:
        Assortivity( RelationshipType::Enum relType=RelationshipType::TRANSITORY, RANDOMBASE *prng=nullptr );
        virtual ~Assortivity();

        const IPKey& GetPropertyKey() const { return m_PropertyKey ; }

        // -------------------------
        // --- IAssortivity Methods
        // -------------------------
        virtual bool Configure(const Configuration *config) override;

        // Update assortivity parameters/controls.
        virtual void Update( const IdmDateTime& rCurrentTime, float dt ) override;

        // Using the attributes of pPartnerA and the attributes of the potential partners, 
        // select a partner from the potentialPartnerList.
        // Return nullptr if a suitable partner was not found.
        // nullptr can be returned even if the list is not empty.
        virtual IIndividualHumanSTI* SelectPartner( IIndividualHumanSTI* pPartnerA,
                                                    const list<IIndividualHumanSTI*>& potentialPartnerList ) override;

        virtual void SetParameters( RANDOMBASE* prng ) override;
    protected:
        typedef std::function<int( const Assortivity*, const IIndividualHumanSTI* )> tGetIndexFunc;
        typedef std::function<std::string( const Assortivity*, const IIndividualHumanSTI* )> tGetStringValueFunc;

        IIndividualHumanSTI* FindPartner( IIndividualHumanSTI* pPartnerA,
                                          const list<IIndividualHumanSTI*>& potentialPartnerList,
                                          tGetIndexFunc func);

        IIndividualHumanSTI* FindPartnerIP( IIndividualHumanSTI* pPartnerA,
                                            const list<IIndividualHumanSTI*>& potentialPartnerList,
                                            tGetStringValueFunc func);

        // Derived classes will add more options to the switch statement
        virtual IIndividualHumanSTI* SelectPartnerForExtendedGroups( AssortivityGroup::Enum group,
                                                                     IIndividualHumanSTI* pPartnerA,
                                                                     const list<IIndividualHumanSTI*>& potentialPartnerList );

        // This routine is called inside Configure() but before the data is completely read.
        virtual void AddConfigurationParameters( AssortivityGroup::Enum group, const Configuration *config ) {};

        // This routine is called inside Configure() but after the data has been read.
        // It allows the derived class to make sure values are correct for the given
        // Group type.
        virtual void CheckDerivedValues() {} ;

        // Return the value of the group read in
        AssortivityGroup::Enum GetGroup() const { return m_Group; };

        // Return the value to use based on calls to Update()
        virtual AssortivityGroup::Enum GetGroupToUse() const ;

        RelationshipType::Enum GetRelationshipType() const { return m_RelType ; }
        std::vector<std::string>& GetAxes() { return m_Axes; }

        int GetIndex( const std::string& rStringValue );
        void CheckAxesForTrueFalse();
        void CheckAxesForProperty();
        void CheckMatrix();
        void SortMatrixFalseTrue();
        bool UsesStartYear() const ;

        static std::string Assortivity::ValuesToString( const std::vector<std::string>& rList ) ;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        RelationshipType::Enum          m_RelType ;
        RANDOMBASE*                     m_pRNG ;
        AssortivityGroup::Enum          m_Group ;
        IPKey                           m_PropertyKey ;
        std::vector<std::string>        m_Axes ;
        std::vector<std::vector<float>> m_WeightingMatrix ;
        float                           m_StartYear;  // if current year is < start year, default to NO_GROUP
        bool                            m_StartUsing; // value is based on start year versus current year

        DECLARE_SERIALIZABLE(Assortivity);
#pragma warning( pop )
    };
}