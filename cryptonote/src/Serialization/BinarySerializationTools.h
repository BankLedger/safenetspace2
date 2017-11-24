
// Copyright (c) 2014-2016 XDN developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <CryptoNote.h>
#include "BinaryInputStreamSerializer.h"
#include "BinaryOutputStreamSerializer.h"
#include "Common/MemoryInputStream.h"
#include "Common/StdInputStream.h"
#include "Common/StdOutputStream.h"
#include "Common/VectorOutputStream.h"

#include <fstream>
#if SQ_ADD_ALIAS
#include <cassert>
#include <iostream>
#include <iterator>
#include <boost/type_traits/make_unsigned.hpp>

#include "Common/Varint.h"
//#include "warnings.h"
#include "CryptoNoteCore/Difficulty.h"

//PUSH_WARNINGS
//DISABLE_VS_WARNINGS(4244)
//DISABLE_VS_WARNINGS(4100)

//TODO: fix size_t warning in x32 platform

template <class Stream, bool IsSaving>
struct binary_archive_base
{
	typedef Stream stream_type;
	typedef binary_archive_base<Stream, IsSaving> base_type;
	typedef boost::mpl::bool_<IsSaving> is_saving;

	typedef uint8_t variant_tag_type;

	explicit binary_archive_base(stream_type &s) : stream_(s) { }

	void tag(const char *) { }
	void begin_object() { }
	void end_object() { }
	void begin_variant() { }
	void end_variant() { }
	stream_type &stream() { return stream_; }
protected:
	stream_type &stream_;
};

template <bool W>
struct binary_archive;

template <>
struct binary_archive<false> : public binary_archive_base<std::istream, false>
{
	explicit binary_archive(stream_type &s) : base_type(s) {
		stream_type::streampos pos = stream_.tellg();
		stream_.seekg(0, std::ios_base::end);
		eof_pos_ = stream_.tellg();
		stream_.seekg(pos);
	}

	template <class T>
	void serialize_int(T &v)
	{
		serialize_uint(*(typename boost::make_unsigned<T>::type *)&v);
	}

	template <class T>
	void serialize_uint(T &v, size_t width = sizeof(T))
	{
		T ret = 0;
		unsigned shift = 0;
		for (size_t i = 0; i < width; i++) {
			//std::cerr << "tell: " << stream_.tellg() << " value: " << ret << std::endl;
			char c;
			stream_.get(c);
			T b = (unsigned char)c;
			ret += (b << shift);
			shift += 8;
		}
		v = ret;
	}
	void serialize_blob(void *buf, size_t len, const char *delimiter = "") { stream_.read((char *)buf, len); }

	template <class T>
	void serialize_varint(T &v)
	{
		serialize_uvarint(*(typename boost::make_unsigned<T>::type *)(&v));
	}

	template <class T>
	void serialize_uvarint(T &v)
	{
		typedef std::istreambuf_iterator<char> it;
		Tools::read_varint(it(stream_), it(), v); // XXX handle failure
	}

#if SQ_BINARY_DEBUG
	template <class T>
	void serialize_uint128_t(T &v)
	{
		uint64_t nUpper = 0, nLower = 0;

		serialize_varint(nUpper);

		serialize_varint(nLower);

		uint128_t nTemp(nUpper, nLower);
		v = CryptoNote::uint128_n2b(nTemp);
	}
#endif

	void begin_array(size_t &s)
	{
		serialize_varint(s);
	}
	void begin_array() { }

	void delimit_array() { }
	void end_array() { }

	void begin_string(const char *delimiter = "\"") { }
	void end_string(const char *delimiter = "\"") { }

	void read_variant_tag(variant_tag_type &t) {
		serialize_int(t);
	}

	size_t remaining_bytes() {
		if (!stream_.good())
			return 0;
		//std::cerr << "tell: " << stream_.tellg() << std::endl;
		assert(stream_.tellg() <= eof_pos_);
		return eof_pos_ - stream_.tellg();
	}
protected:
	std::streamoff eof_pos_;
};

template <>
struct binary_archive<true> : public binary_archive_base<std::ostream, true>
{
	explicit binary_archive(stream_type &s) : base_type(s) { }

	template <class T>
	void serialize_int(T v)
	{
		serialize_uint(static_cast<typename boost::make_unsigned<T>::type>(v));
	}
	template <class T>
	void serialize_uint(T v)
	{
		for (size_t i = 0; i < sizeof(T); i++) {
			stream_.put((char)(v & 0xff));
			if (1 < sizeof(T)) {
				v >>= 8;
			}
		}
	}
	void serialize_blob(void *buf, size_t len, const char *delimiter = "") { stream_.write((char *)buf, len); }

	template <class T>
	void serialize_varint(T &v)
	{
		serialize_uvarint(*(typename boost::make_unsigned<T>::type *)(&v));
	}

	template <class T>
	void serialize_uvarint(T &v)
	{
		typedef std::ostreambuf_iterator<char> it;
		Tools::write_varint(it(stream_), v);
	}

#if SQ_BINARY_DEBUG
	template <class T>
	void serialize_uint128_t(T &v)
	{
		uint64_t lowpart, highpart;
		uint128_t value = CryptoNote::uint128_b2n(v);

		highpart = value.upper();
		lowpart = value.lower();

		serialize_varint(highpart);

		serialize_varint(lowpart);
	}
#endif

	void begin_array(size_t s)
	{
		serialize_varint(s);
	}
	void begin_array() { }
	void delimit_array() { }
	void end_array() { }

	void begin_string(const char *delimiter = "\"") { }
	void end_string(const char *delimiter = "\"") { }

	void write_variant_tag(variant_tag_type t) {
		serialize_int(t);
	}
};

//POP_WARNINGS
#endif
namespace CryptoNote
{
	template <typename T>
	BinaryArray storeToBinary(const T& obj)
	{
		BinaryArray result;
		Common::VectorOutputStream stream(result);
		BinaryOutputStreamSerializer ba(stream);
		serialize(const_cast<T&>(obj), ba);
		return result;
	}
	
	template <typename T>
	void loadFromBinary(T& obj, const BinaryArray& blob)
	{
		Common::MemoryInputStream stream(blob.data(), blob.size());
		BinaryInputStreamSerializer ba(stream);
		serialize(obj, ba);
	}

	template <typename T>
	bool storeToBinaryFile(const T& obj, const std::string& filename)
	{
		try
		{
			std::ofstream dataFile;
			dataFile.open(filename, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
			if (dataFile.fail())
			{
				return false;
			}
			
			Common::StdOutputStream stream(dataFile);
			BinaryOutputStreamSerializer out(stream);
			CryptoNote::serialize(const_cast<T&>(obj), out);
      
			if (dataFile.fail())
			{
				return false;
			}
			
			dataFile.flush();
		}
		catch (std::exception&)
		{
			return false;
		}
		
		return true;
	}
	
	template<class T>
	bool loadFromBinaryFile(T& obj, const std::string& filename)
	{
		try
		{
			std::ifstream dataFile;
			dataFile.open(filename, std::ios_base::binary | std::ios_base::in);
			if (dataFile.fail())
			{
				return false;
			}
			
			Common::StdInputStream stream(dataFile);
			BinaryInputStreamSerializer in(stream);
			serialize(obj, in);
			return !dataFile.fail();
		}
		catch (std::exception&)
		{
			return false;
		}
	}

}

#if 0
template <>
struct binary_archive<true> : public binary_archive_base<std::ostream, true>
{
	explicit binary_archive(stream_type &s) : base_type(s) { }

	template <class T>
	void serialize_int(T v)
	{
		serialize_uint(static_cast<typename boost::make_unsigned<T>::type>(v));
	}
	template <class T>
	void serialize_uint(T v)
	{
		for (size_t i = 0; i < sizeof(T); i++)
		{
			stream_.put((char)(v & 0xff));
			if (1 < sizeof(T))
			{
				v >>= 8;
			}
		}
	}
	void serialize_blob(void *buf, size_t len, const char *delimiter = "")
	{
		stream_.write((char *)buf, len);
	}

	template <class T>
	void serialize_varint(T &v)
	{
		serialize_uvarint(*(typename boost::make_unsigned<T>::type *)(&v));
	}

	template <class T>
	void serialize_uvarint(T &v)
	{
		typedef std::ostreambuf_iterator<char> it;
		Tools::write_varint(it(stream_), v);
	}

	template <class T>
	void serialize_uint128_t(T &v)
	{
		uint64_t lowpart, highpart;
		uint128_t value = CryptoNote::uint128_b2n(v);

		highpart = value.upper();
		lowpart = value.lower();
		serialize_varint(highpart);
		serialize_varint(lowpart);
	}

	void begin_array(size_t s)
	{
		serialize_varint(s);
	}
	void begin_array() { }
	void delimit_array() { }
	void end_array() { }

	void begin_string(const char *delimiter = "\"") { }
	void end_string(const char *delimiter = "\"") { }

	void write_variant_tag(variant_tag_type t)
	{
		serialize_int(t);
	}
};
#endif