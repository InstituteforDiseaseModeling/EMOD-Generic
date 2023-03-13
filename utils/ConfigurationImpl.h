/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>
#include <functional>
#include "ISupports.h"
#include "EnumSupport.h"

namespace Kernel
{
    namespace MetadataDescriptor 
    {
        struct Enum; // forward decl
    }

    namespace MetadataDescriptor
    {
        struct Base
        {
        public:
            std::string name;
            std::string description;
            virtual Element GetSchemaElement() = 0;

        protected: // Don't want outsiders constructing this
            Base(std::string _name, std::string _desc) : name(_name), description(_desc) {}
        };

        struct Enum : public Base
        {
            // convenience macro to save typing of these args
#define MDD_ENUM_ARGS(enum_name) enum_name::pairs::count(), enum_name::pairs::get_keys(), enum_name::pairs::get_values()

            Enum(std::string _name, std::string _desc, int count, const std::vector<std::string> strings, const std::vector<int> values) : Base(_name, _desc)
            {
                for (int k = 0; k < count; k++)
                {
                    enum_value_specs.push_back(pair<std::string,int>(strings[k], values[k]));
                }
            }

            typedef pair<std::string,int> enum_value_spec_t;
            std::vector<enum_value_spec_t> enum_value_specs;

            virtual const char *GetTypeString()
            {
                return "enum";
            }

            virtual Element GetSchemaElement()
            {
                Element member = Object();
                QuickBuilder qb(member);
                qb["type"] = json::String(GetTypeString());

                for (int k = 0; k < enum_value_specs.size(); k++)
                {
                    qb["enum"][k] = json::String(enum_value_specs[k].first);
                }

                qb["description"] = json::String(description);
                qb["default"] = json::String(enum_value_specs[0].first);
                return member;
            }
        };

        struct VectorOfEnum : public Enum
        {
            VectorOfEnum(std::string _name, std::string _desc, int count, const std::vector<std::string> strings, const std::vector<int> values) :
                Enum(_name, _desc, count, strings, values)
            {
            }

            virtual const char *GetTypeString()
            {
                return "Vector Enum";
            }

            virtual Element GetSchemaElement()
            {
                Element member = Enum::GetSchemaElement();
                QuickBuilder qb(member);
                auto default_array = json::Array();
                qb["default"] = default_array;
                return member;
            }
        };
    }
}
