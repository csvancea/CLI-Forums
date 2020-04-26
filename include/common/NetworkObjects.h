#pragma once

#include <string>

#include <common/BitStream.h>
#include <fmt/ostream.h>

#define TOPIC_SIZE 50

#define MESSAGE_INT_SIZE 5
#define MESSAGE_SHORTREAL_SIZE 2
#define MESSAGE_FLOAT_SIZE 6
#define MESSAGE_STRING_SIZE 1500

namespace NetObj
{
    enum {
        RPC_INVALID,
        RPC_START = RPC_INVALID,
        
        RPC_CLIENT_ANNOUNCE,
        RPC_SUBSCRIBE,
        RPC_UNSUBSCRIBE,

        RPC_END
    };
    bool IsValidRPC(uint8_t rpc);

    enum {
        TYPE_INT,
        TYPE_SHORTREAL,
        TYPE_FLOAT,
        TYPE_STRING
    };
    const std::string& TypeToString(uint8_t type);

    class Int
    {
    public:
        const std::string& ToString() const { return _str; }
        friend std::ostream& operator<<(std::ostream& os, const Int& other) {
            return os << other._str;
        }

    private:
        void ConstructString();

        std::string _str;
        uint8_t sign;
        uint32_t number;

        friend class ::BitStream;
    };

    class ShortReal
    {
    public:
        const std::string& ToString() const { return _str; }
        friend std::ostream& operator<<(std::ostream& os, const ShortReal& other) {
            return os << other._str;
        }    

    private:
        void ConstructString();

        std::string _str;
        uint16_t number;

        friend class ::BitStream;
    };

    class Float
    {
    public:
        const std::string& ToString() const { return _str; }
        friend std::ostream& operator<<(std::ostream& os, const Float& other) {
            return os << other._str;
        }   

    private:
        void ConstructString();

        std::string _str;
        uint8_t sign;
        uint32_t number;
        uint8_t exponent;

        friend class ::BitStream;
    };

    template <size_t N>
    class String
    {        
    public:
        const std::string& ToString() const { return _str; }
        constexpr size_t GetMaxSize() const { return N; }
        friend std::ostream& operator<<(std::ostream& os, const String& other) {
            return os << other._str;
        }        

    private:
        std::string _str;
        friend class ::BitStream;
    };
    using Topic = String<TOPIC_SIZE>;
    using Message = String<MESSAGE_STRING_SIZE>; 
}

template <>
size_t BitStream::Read(NetObj::Topic& data);

template <>
size_t BitStream::Write(const NetObj::Topic& data);

template <>
size_t BitStream::Read(NetObj::Message& data);

template <>
size_t BitStream::Write(const NetObj::Message& data);

template <>
size_t BitStream::Read(NetObj::Int& data);

template <>
size_t BitStream::Write(const NetObj::Int& data);

template <>
size_t BitStream::Read(NetObj::ShortReal& data);

template <>
size_t BitStream::Write(const NetObj::ShortReal& data);

template <>
size_t BitStream::Read(NetObj::Float& data);

template <>
size_t BitStream::Write(const NetObj::Float& data);

template <>
size_t BitStream::Write(const std::string& in);

template <>
size_t BitStream::Read(std::string& out);
