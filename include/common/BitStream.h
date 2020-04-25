#pragma once

#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstdint>

class BitStream
{
public:
    BitStream();
    BitStream(const void *data, size_t size);

    size_t Write(BitStream& bs, size_t size = SIZE_MAX);
    size_t Write(const void *data, size_t size);
    size_t Read(void *data, size_t size);

    bool SetReadPointer(size_t idx);
    size_t GetReadPointer() const;
    void ResetReadPointer();

    const std::vector<char> GetUnderlyingBuffer() const;
    size_t GetBytes() const;
    size_t GetRemainingBytesToRead() const;

    size_t Remove(size_t start, size_t size);


    template <class T>
    size_t Write(const T& in)
    {
        static_assert(std::is_integral<T>::value, "No specialization found for type T");

        T net = htonT(in);
        return Write(&net, sizeof(T));
    }    

    template <class T>
    size_t Read(T& out)
    {
        static_assert(std::is_integral<T>::value, "No specialization found for type T");

        size_t ret = Read(&out, sizeof(T));
        out = ntohT(out);
        return ret;
    }

    template <typename T>
    static constexpr T htonT (T value) noexcept
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        char* ptr = reinterpret_cast<char*>(&value);
        std::reverse(ptr, ptr + sizeof(T));
#endif
        return value;
    }

    template <typename T>
    static constexpr T ntohT (T value) noexcept
    {
        return htonT(value);
    }

private:
    std::vector<char> _buffer;
    size_t _writePtr;
    size_t _readPtr;
};
