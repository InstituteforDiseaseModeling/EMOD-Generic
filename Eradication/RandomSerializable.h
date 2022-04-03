#pragma once

#include "RANDOM.h"

#include "ISerializable.h"

namespace Kernel
{
    class RANDOMBASE_SER
    {
    public:
        static void serialize( IArchive&, RANDOMBASE* );
    };
    
    class LINEAR_CONGRUENTIAL_SER : public LINEAR_CONGRUENTIAL, public ISerializable
    {
    public:
        LINEAR_CONGRUENTIAL_SER( uint32_t iSequence = 0x31415926, size_t nCache = 0 );
       ~LINEAR_CONGRUENTIAL_SER();

        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        DECLARE_SERIALIZABLE( LINEAR_CONGRUENTIAL_SER );
        static void serialize( IArchive&, RANDOMBASE* );
    };

    class PSEUDO_DES_SER : public PSEUDO_DES, public ISerializable
    {
    public:
        PSEUDO_DES_SER( uint64_t iSequence = 0, size_t nCache = 0 );
        ~PSEUDO_DES_SER();

        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        DECLARE_SERIALIZABLE( PSEUDO_DES_SER );
        static void serialize( IArchive&, RANDOMBASE* );
    };

    class AES_COUNTER_SER : public AES_COUNTER, public ISerializable
    {
    public:
        AES_COUNTER_SER( uint64_t iSequence = 0, size_t nCache = 0 );
        ~AES_COUNTER_SER();

        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
        DECLARE_SERIALIZABLE( AES_COUNTER_SER );
        static void serialize( IArchive&, RANDOMBASE* );
    };
}
