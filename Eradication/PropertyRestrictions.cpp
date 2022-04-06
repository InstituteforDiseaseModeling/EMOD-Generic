/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "PropertyRestrictions.h"

SETUP_LOGGING( "PropertyRestrictions" )

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(PropertyRestriction)
    END_QUERY_INTERFACE_BODY(PropertyRestriction)

    bool PropertyRestriction::Configure(const Configuration* config)
    {
        initConfigTypeMap("Restrictions", &m_restrictions, "");

        bool result = JsonConfigurable::Configure(config);

        if (!JsonConfigurable::_dryrun && result)
        {
            m_configured = true;
        }

        return result;
    }
    
        std::vector<std::string> const & PropertyRestriction::GetRestrictions() const
    {
        release_assert(m_configured);
        return m_restrictions;
    }

    template<class Key, class KeyValue, class Container>
    PropertyRestriction* PropertyRestrictions<Key, KeyValue, Container>::CreateObject()
    {
        return new PropertyRestriction();
    }

    template<class Key, class KeyValue, class Container>
    PropertyRestrictions<Key,KeyValue,Container>::PropertyRestrictions()
    : JsonConfigurableCollection("PropertyRestrictions")
    , _restrictions()
    {
    }

    template<class Key, class KeyValue, class Container>
    int PropertyRestrictions<Key, KeyValue, Container>::Size() const
    {
        return _restrictions.size();
    }

    template<class Key, class KeyValue, class Container>
    json::QuickBuilder PropertyRestrictions<Key, KeyValue, Container>::GetSchema()
    {
        json::QuickBuilder obj_schema = JsonConfigurableCollection::GetSchema();
        obj_schema["default"] = json::Array();
        return obj_schema;
    }

    template<class Key, class KeyValue, class Container>
    void PropertyRestrictions<Key, KeyValue, Container>::ConfigureFromJsonAndKey( const Configuration * inputJson, const std::string& key )
    {
        JsonConfigurableCollection::ConfigureFromJsonAndKey(inputJson, key);

        // Make this optional.
        if( inputJson->Exist( key ) == false )
        {
            return;
        }

        // We have a list of json objects. The format/logic is as follows. For input:
        // [
        //  {"Restrictions": ["Character:Good", "Income:High"]},
        //  {"Restrictions": ["Character:Bad",  "Income:Low" ]}
        // ]
        // We give the intervention if the individuals has
        // Good Character AND High Income OR Bad Character AND Low Income.
        // So we AND together the elements of each json object and OR together these 
        // calculated truth values of the elements of the json array.
        for (auto& outer_restriction : m_Collection)
        {
            const std::vector<std::string> & inner_restrictions = outer_restriction->GetRestrictions();
            Container container;
            
            for (auto& restriction : inner_restrictions)
            {
                KeyValue kv(restriction);
                container.Add(kv);
            }
            _restrictions.push_back(container);
        }
    }

    template<class Key, class KeyValue, class Container>
    void PropertyRestrictions<Key, KeyValue, Container>::Add( std::map< std::string, std::string >& rMap )
    {
        Container container;
        for( auto& entry : rMap )
        {
            KeyValue kv( entry.first, entry.second );
            container.Add( kv );
        }
        _restrictions.push_back( container );
    }

    template<class Key, class KeyValue, class Container>
    bool PropertyRestrictions<Key, KeyValue, Container>::Qualifies( const Container& rPropertiesContainer )
    {
        bool qualifies = true;

        // individual has to have one of these properties
        for( Container& container : _restrictions)
        {
            qualifies = false;
            bool meets_property_restriction_criteria = true;
            for( auto kv : container )
            {
                if( rPropertiesContainer.Contains( kv ) )
                {
                    LOG_DEBUG_F( "Partial property restriction: constraint is %s.\n", kv.ToString().c_str() );
                    continue; // we're good
                }
                else
                {
                    meets_property_restriction_criteria = false;
#ifdef WIN32
                    LOG_DEBUG_F( "Person does not get the intervention because the allowed property is %s and the person is %s.\n", 
                                 kv.ToString().c_str(), rPropertiesContainer.Get( kv.GetKey<Key>() ).ToString().c_str() );
#endif
                    break;
                }
            }
            // If verified, we're done since these are OR-ed together
            if( meets_property_restriction_criteria )
            {
                qualifies = true;
                LOG_DEBUG_F( "Individual meets at least 1 of the OR-ed together property restriction conditions. Not checking the rest.\n" );
                break;
            }
        }

        return qualifies;
    }

    template<class Key, class KeyValue, class Container>
    bool PropertyRestrictions<Key, KeyValue, Container>::Qualifies( const tProperties* pProp )
    {
        release_assert( pProp );

        bool qualifies = true;

        // individual has to have one of these properties
        for( Container& container : _restrictions )
        {
            qualifies = false;
            bool meets_property_restriction_criteria = true;
            for( KeyValue kv : container )
            {
                const std::string& szKey = kv.GetKeyAsString();
                const std::string& szVal = kv.GetValueAsString();

                LOG_DEBUG_F( "Applying property restrictions in event coordinator: %s/%s.\n", szKey.c_str(), szVal.c_str() );
                // Every individual has to have a property value for each property key
                if( pProp->at( szKey ) == szVal )
                {
                    LOG_DEBUG_F( "Person satisfies (partial) property restriction: constraint is %s/%s and the person is %s.\n", szKey.c_str(), szVal.c_str(), pProp->at( szKey ).c_str() );
                    continue; // we're good
                }
                else
                {
                    meets_property_restriction_criteria = false;
                    LOG_DEBUG_F( "Person does not get the intervention because the allowed property is %s/%s and the person is %s.\n", szKey.c_str(), szVal.c_str(), pProp->at( szKey ).c_str() );
                    break;
                }
            }
            // If verified, we're done since these are OR-ed together
            if( meets_property_restriction_criteria )
            {
                qualifies = true;
                LOG_DEBUG_F( "Individual meets at least 1 of the OR-ed together property restriction conditions. Not checking the rest.\n" );
                break;
            }
        }

        return qualifies;
    }

    template<class Key, class KeyValue, class Container>
    std::string PropertyRestrictions<Key, KeyValue, Container>::GetAsString() const
    {
        std::string restriction_str ;
        if( _restrictions.size() > 0 )
        {
            for( Container container : _restrictions )
            {
                restriction_str += "[ ";
                for( KeyValue kv : container )
                {
                    std::string prop = kv.ToString();
                    restriction_str += "'"+ prop +"', " ;
                }
                restriction_str = restriction_str.substr( 0, restriction_str.length()-2 );
                restriction_str += " ], ";
            }
            restriction_str = restriction_str.substr( 0, restriction_str.length()-2 );
        }
        return restriction_str;
    }

    // -------------------------------------------------------------------------------
    // --- This defines the implementations for these templetes with these parameters.
    // --- If you comment these out, you will get unresolved externals when linking.
    // -------------------------------------------------------------------------------
    template class PropertyRestrictions<IPKey, IPKeyValue, IPKeyValueContainer>;
    template class PropertyRestrictions<NPKey, NPKeyValue, NPKeyValueContainer>;

}
