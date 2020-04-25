#include <common/BitStream.h>
#include <cstring>

BitStream::BitStream() : _writePtr(0), _readPtr(0)
{ 

}

BitStream::BitStream(const void *data, size_t size) : _writePtr(0), _readPtr(0)
{
    Write(data, size);
}

size_t BitStream::Write(BitStream& bs, size_t size)
{
    if (bs._readPtr >= bs._buffer.size()) {
        return 0;
    }

    if (bs._readPtr + size > bs._buffer.size()) {
        size = bs._buffer.size() - bs._readPtr;
    }

    Write(&bs._buffer[bs._readPtr], size);
    bs._readPtr += size;
    return size;
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

bool BitStream::SetReadPointer(size_t idx)
{
    if (idx > _buffer.size()) {
        return false;
    }

    _readPtr = idx;
    return true;
}

size_t BitStream::GetReadPointer() const
{
    return _readPtr;
}

void BitStream::ResetReadPointer()
{
    _readPtr = 0;
}

const std::vector<char> BitStream::GetUnderlyingBuffer() const
{
    return _buffer;
}

size_t BitStream::GetBytes() const
{
     return _buffer.size();
}

size_t BitStream::GetRemainingBytesToRead() const
{
    return _buffer.size() - _readPtr;
}

size_t BitStream::Remove(size_t start, size_t size)
{
    if (start >= _buffer.size()) {
        return 0;
    }

    if (start + size > _buffer.size()) {
        size = _buffer.size() - start;
    }

    auto begin = _buffer.begin() + start;
    auto end = begin + size;

    _buffer.erase(begin, end);

    _readPtr = std::min(_readPtr, _buffer.size());
    _writePtr = std::min(_writePtr, _buffer.size());

    return size;
}