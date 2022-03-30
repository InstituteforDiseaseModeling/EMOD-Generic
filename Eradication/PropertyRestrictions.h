/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <map>
#include <vector>

#include "BoostLibWrapper.h"
#include "Configure.h"
#include "Properties.h"
#include "PropertiesString.h"
#include "NodeProperties.h"
#include "JsonConfigurableCollection.h"

namespace Kernel
{
    struct IIndividualHumanEventContext;

    class PropertyRestriction : public JsonConfigurable
    {
        GET_SCHEMA_STATIC_WRAPPER(PropertyRestriction)

    public:
        PropertyRestriction() {}
        virtual bool Configure(const Configuration* config);
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
            DECLARE_QUERY_INTERFACE()
        
        std::vector<std::string> const & GetRestrictions() const;

    private:
        std::vector<std::string> m_restrictions;
        bool m_configured;
    };

    template<class Key, class KeyValue, class Container>
    class IDMAPI PropertyRestrictions : public JsonConfigurableCollection<PropertyRestriction>
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) { return e_NOINTERFACE; }
    public:
        PropertyRestrictions();
        virtual void ConfigureFromJsonAndKey( const Configuration *, const std::string &key ) override;
        virtual bool  HasValidDefault() const override { return true; }

        int Size() const;
        void Add( std::map< std::string, std::string >& rMap );
        bool Qualifies( const Container& rPropertiesContainer );
        bool Qualifies( const tProperties* pPropsMap );
        std::string GetAsString() const;

    private:
#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::list< Container > _restrictions;
#pragma warning( pop )

    protected:
        virtual PropertyRestriction* CreateObject() override;
    };
}
