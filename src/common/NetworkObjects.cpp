#include <common/NetworkObjects.h>
#include <algorithm>

namespace NetObj
{
	bool IsValidRPC(uint8_t rpc)
	{
		return rpc > RPC_START && rpc < RPC_END;
	}

	const std::string& TypeToString(uint8_t type)
	{
        static std::string INT{"INT"};
		static std::string SHORTREAL{"SHORT_REAL"};
		static std::string FLOAT{"FLOAT"};
		static std::string STRING{"STRING"};
		static std::string UNKNOWN{"UNKNOWN"};

		switch (type) {
		case TYPE_INT: return INT;
      	case TYPE_SHORTREAL: return SHORTREAL;
        case TYPE_FLOAT: return FLOAT;
        case TYPE_STRING: return STRING;
		}

		return UNKNOWN;
	}

	void Int::ConstructString()
	{
		_str.clear();

		if (sign) _str.push_back('-');
		_str += std::to_string(number);
	}

	void ShortReal::ConstructString()
	{
		_str = fmt::format("{:03d}", number);
		_str.insert(_str.length() - 2, ".");
	}

	void Float::ConstructString()
	{
		_str.clear();

		if (sign) _str.push_back('-');
		
		if (exponent == 0) {
			_str += std::to_string(number);
		}
		else {
			_str += fmt::format("{:0{}}", number, exponent + 1);
			_str.insert(_str.length() - exponent, ".");
		}
	}
}


static size_t Impl_Read(BitStream& bs, std::string& str, size_t max_size)
{
	size_t bytes_read;

	str.resize(max_size);
	bytes_read = bs.Read(&str[0], max_size);

	size_t i = 0;
	for (; i < bytes_read; ++i) {
		if (str[i] == '\0') {
			break;
		}
	}

	str.resize(i);
	return bytes_read;
}

static size_t Impl_Write(BitStream& bs, const std::string& data, size_t max_size)
{
	return bs.Write(data.c_str(), std::min(data.size() + 1, max_size));
}

template <>
size_t BitStream::Read(NetObj::Topic& data)
{
	return ::Impl_Read(*this, data._str, data.GetMaxSize());
}

template <>
size_t BitStream::Write(const NetObj::Topic& data)
{
	return ::Impl_Write(*this, data._str, data.GetMaxSize());
}

template <>
size_t BitStream::Read(NetObj::Message& data)
{
	return ::Impl_Read(*this, data._str, data.GetMaxSize());
}

template <>
size_t BitStream::Write(const NetObj::Message& data)
{
	return ::Impl_Write(*this, data._str, data.GetMaxSize());
}

template <>
size_t BitStream::Read(NetObj::Int& data)
{
	size_t bytes_read = 0; 

	bytes_read += Read(data.sign);
	bytes_read += Read(data.number);

	data.ConstructString();
	return bytes_read;
}

template <>
size_t BitStream::Write(const NetObj::Int& data)
{
	size_t bytes_written = 0; 

	bytes_written += Write(data.sign);
	bytes_written += Write(data.number);

	return bytes_written;
}

template <>
size_t BitStream::Read(NetObj::ShortReal& data)
{
	size_t bytes_read = 0; 

	bytes_read += Read(data.number);
	/**
	 * In tema nu este mentionat ca SHORT_REAL este in network order,
	 * desi toate celelalte tipuri care pot fi byteswapped mentionaza
	 * explicit acest lucru.
	 * 
	 * data.number = BitStream::ntohT(data.number);
	 */

	data.ConstructString();
	return bytes_read;
}

template <>
size_t BitStream::Write(const NetObj::ShortReal& data)
{
	size_t bytes_written = 0; 

	bytes_written += Write(/*BitStream::ntohT*/(data.number));

	return bytes_written;
}

template <>
size_t BitStream::Read(NetObj::Float& data)
{
	size_t bytes_read = 0; 

	bytes_read += Read(data.sign);
	bytes_read += Read(data.number);
	bytes_read += Read(data.exponent);

	data.ConstructString();
	return bytes_read;
}

template <>
size_t BitStream::Write(const NetObj::Float& data)
{
	size_t bytes_written = 0; 

	bytes_written += Write(data.sign);
	bytes_written += Write(data.number);
	bytes_written += Write(data.exponent);

	return bytes_written;
}

template <>
size_t BitStream::Write(const std::string& in)
{
    size_t written_bytes = 0;

    written_bytes += Write<uint16_t>(in.length());
    written_bytes += Write(in.c_str(), in.length());
    return written_bytes;
}

template <>
size_t BitStream::Read(std::string& out)
{
    size_t read_bytes = 0;
	size_t ret;
    uint16_t size;

    ret = Read(size);
    read_bytes += ret;
    
    if (ret != sizeof (uint16_t)) {
        return read_bytes;
    }

    out.resize(size);

    ret = Read(&out[0], size);
    read_bytes += ret;

    out.resize(ret); // poate am citit mai putin din stream decat era anuntat initial
    return read_bytes;
}
