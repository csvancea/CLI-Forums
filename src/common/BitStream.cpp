#include <common/BitStream.h>
#include <cstring>

BitStream::BitStream() : _writePtr(0), _readPtr(0)
{ 

}

BitStream::BitStream(const void *data, size_t size) : _writePtr(0), _readPtr(0)
{
    Write(data, size);
}

size_t BitStream::Write(const void *data, size_t size)
{
    const char *bytes = (const char *)data;

    _buffer.insert(_buffer.begin() + _writePtr, bytes, bytes + size);
    _writePtr += size;

    return size;
}

size_t BitStream::Read(void *data, size_t size)
{
    if (_readPtr + size > _buffer.size()) {
        size = _buffer.size() - _readPtr;
    }

    memcpy(data, &_buffer[_readPtr], size);
    _readPtr += size;

    return size;
}

void BitStream::ResetReadPointer()
{
    _readPtr = 0;
}

const std::vector<char> BitStream::GetUnderlyingBuffer() const
{
    return _buffer;
}
