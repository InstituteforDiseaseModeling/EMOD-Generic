/***************************************************************************************************

Copyright (c) 2018 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <algorithm>


// Substitution into declaration syntax
#define ENUM_VALUE_SPEC(name, value)    name=value,

// Declaration of enum and supporting functions for enum; goes in header
#define ENUM_DECLARE(enum_name, elements)                                                          \
    namespace enum_name                                                                            \
    {                                                                                              \
        enum Enum : int32_t                                                                        \
        {                                                                                          \
            elements                                                                               \
        };                                                                                         \
                                                                                                   \
        class pairs                                                                                \
        {                                                                                          \
            private:                                                                               \
                static bool                            enum_init;                                  \
                static std::string                     arg_str;                                    \
                                                                                                   \
                static std::vector<std::string>        key_vec;                                    \
                static std::vector<int>                val_vec;                                    \
                                                                                                   \
                static void                            pre_proc_enum();                            \
                                                                                                   \
            public:                                                                                \
                static       int                       count();                                    \
                static const std::vector<std::string>  get_keys();                                 \
                static const std::vector<int>          get_values();                               \
                static       std::string               lookup_key(int);                            \
                static       int                       lookup_value(std::string);                  \
        };                                                                                         \
    };

// Required 1 level of pass-through for correct pre-processor substitution; goes in object code
#define ENUM_INITIALIZE(enum_name, elements)  ENUM_INITIALIZE_INTERNAL(enum_name, elements)

// Implementation of supporting functions for enum
#define ENUM_INITIALIZE_INTERNAL(enum_name, ...)                                                   \
    namespace enum_name                                                                            \
    {                                                                                              \
        bool         pairs::enum_init = false;                                                     \
        std::string  pairs::arg_str(#__VA_ARGS__);                                                 \
                                                                                                   \
        std::vector<std::string>  pairs::key_vec;                                                  \
        std::vector<int>          pairs::val_vec;                                                  \
                                                                                                   \
        void pairs::pre_proc_enum()                                                                \
        {                                                                                          \
            size_t pos      = 0;                                                                   \
            auto   lamb_fun = [](unsigned char const c) { return std::isspace(c); };               \
                                                                                                   \
            while((pos = arg_str.find("=")) != std::string::npos)                                  \
            {                                                                                      \
                std::string kstr = arg_str.substr(0, pos);                                         \
                kstr.erase(std::remove_if(kstr.begin(), kstr.end(), lamb_fun), kstr.end());        \
                arg_str.erase(0, pos+1);                                                           \
                key_vec.push_back(kstr);                                                           \
                                                                                                   \
                pos = arg_str.find(",");                                                           \
                std::string val_str = arg_str.substr(0, pos);                                      \
                arg_str.erase(0, pos+1);                                                           \
                val_vec.push_back(std::stoi(val_str));                                             \
            }                                                                                      \
            enum_init = true;                                                                      \
            return;                                                                                \
        }                                                                                          \
                                                                                                   \
        int pairs::count()                                                                         \
        {                                                                                          \
            if(!enum_init)                                                                         \
            {                                                                                      \
                pre_proc_enum();                                                                   \
            }                                                                                      \
            return static_cast<int>(key_vec.size());                                               \
        }                                                                                          \
                                                                                                   \
        const std::vector<std::string> pairs::get_keys()                                           \
        {                                                                                          \
            if(!enum_init)                                                                         \
            {                                                                                      \
                pre_proc_enum();                                                                   \
            }                                                                                      \
            return key_vec;                                                                        \
        }                                                                                          \
                                                                                                   \
        const std::vector<int> pairs::get_values()                                                 \
        {                                                                                          \
            if(!enum_init)                                                                         \
            {                                                                                      \
                pre_proc_enum();                                                                   \
            }                                                                                      \
            return val_vec;                                                                        \
        }                                                                                          \
                                                                                                   \
        std::string pairs::lookup_key(int enum_val)                                                \
        {                                                                                          \
            if(!enum_init)                                                                         \
            {                                                                                      \
                pre_proc_enum();                                                                   \
            }                                                                                      \
            for(size_t k1 = 0; k1 < key_vec.size(); k1++)                                          \
            {                                                                                      \
                if (val_vec[k1] == enum_val)                                                       \
                {                                                                                  \
                    return key_vec[k1];                                                            \
                }                                                                                  \
            }                                                                                      \
            return "";                                                                             \
        }                                                                                          \
                                                                                                   \
        int pairs::lookup_value(std::string enum_key)                                              \
        {                                                                                          \
            if(!enum_init)                                                                         \
            {                                                                                      \
                pre_proc_enum();                                                                   \
            }                                                                                      \
            for(size_t k1 = 0; k1 < key_vec.size(); k1++)                                          \
            {                                                                                      \
                if (key_vec[k1] == enum_key)                                                       \
                {                                                                                  \
                    return val_vec[k1];                                                            \
                }                                                                                  \
            }                                                                                      \
            return -1;                                                                             \
        }                                                                                          \
    };