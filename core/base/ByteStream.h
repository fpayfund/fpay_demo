#ifndef __BASE_BYTE_STREAM_H__
#define __BASE_BYTE_STREAM_H__

#include <string>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>
#include <set>
#include <stdexcept>

#include "BlockBuffer.h"
#include "varstr.h"

inline uint16_t XHTONS(uint16_t i16)
{
    return ((i16 << 8) | (i16 >> 8));
}
inline uint32_t XHTONL(uint32_t i32)
{
    return ((uint32_t(XHTONS(i32)) << 16) | XHTONS(i32>>16));
}
inline uint64_t XHTONLL(uint64_t i64)
{
    return ((uint64_t(XHTONL((uint32_t)i64)) << 32) |XHTONL((uint32_t(i64>>32))));
}

#define XNTOHS XHTONS
#define XNTOHL XHTONL
#define XNTOHLL XHTONLL

namespace proto {

class StreamExn
    : public std::runtime_error
{
public:
    StreamExn(const std::string & w)
        : std::runtime_error(w)
    {}
};

class WriteStreamExn
    : public StreamExn
{
public:
    WriteStreamExn(const std::string & w)
        : StreamExn(w)
    {}
};

class ReadStreamExn
    : public StreamExn
{
public:
    ReadStreamExn(const std::string & w)
        : StreamExn(w)
    {}
};

class PackBuffer
{
public:
    char * data()
    {
        return bb.data();
    }
    size_t size() const
    {
        return bb.size();
    }
    void resize(size_t n)
    {
        if (bb.resize(n))
            return;
        throw WriteStreamExn("resize buf overflow");
    }
    void append(const char * data, size_t size)
    {
        if (bb.append(data, size))
            return;
        throw WriteStreamExn("append buf overflow");
    }
    void append(const char * data)
    {
        append(data, ::strlen(data));
    }
    void replace(size_t pos, const char * rep, size_t n)
    {
        if (bb.replace(pos, rep, n)) return;
        throw WriteStreamExn("replace buf overflow");
    }
    void reserve(size_t n)
    {
        if (bb.reserve(n)) return;
        throw WriteStreamExn("reserve buf overflow");
    }

private:
    typedef BlockBuffer<def_block_alloc_16k, 65536> Block_t;
    Block_t bb;
};

class BinWriteStream
{
private:
    BinWriteStream (const BinWriteStream & o);
    BinWriteStream & operator = (const BinWriteStream& o);
public:
    uint16_t xhtons(uint16_t i16) { return XHTONS(i16); }
    uint32_t xhtonl(uint32_t i32) { return XHTONL(i32); }
    uint64_t xhtonll(uint64_t i64) { return XHTONLL(i64); }

    BinWriteStream(PackBuffer & pb, size_t off = 0) : m_buffer(pb)
    {
        m_offset = pb.size() + off;
        m_buffer.resize(m_offset);
    }

    char * data()       { return m_buffer.data() + m_offset; }
    const char * data()  const { return m_buffer.data() + m_offset; }
    size_t size() const { return m_buffer.size() - m_offset; }

    BinWriteStream & writeByte(const void * s, size_t n) { m_buffer.append((const char *)s, n); return *this; }
    BinWriteStream & writeByte(const void * s)           { m_buffer.append((const char *)s); return *this; }

    BinWriteStream & writeUint8(uint8_t u8)    { return writeByte(&u8, 1); }
    BinWriteStream & writeUint16(uint16_t u16) { u16 = xhtons(u16); return writeByte(&u16, 2); }
    BinWriteStream & writeUint32(uint32_t u32) { u32 = xhtonl(u32); return writeByte(&u32, 4); }
    BinWriteStream & writeUint64(uint64_t u64) { u64 = xhtonll(u64); return writeByte(&u64, 8); }

    BinWriteStream & writeVarstr(const Varstr & vs)     { return writeVarstr(vs.data(), vs.size()); }
    BinWriteStream & writeVarstr(const void * s)        { return writeVarstr(s, strlen((const char *)s)); }
    BinWriteStream & writeVarstr(const std::string & s) { return writeVarstr(s.data(), s.size()); }
    BinWriteStream & writeVarstr(const void * s, size_t len)
    {
        if (len > 0xFFFF) throw WriteStreamExn("writeVarstr: varstr too big");
        return writeUint16(uint16_t(len)).writeByte(s, len);
    }
    BinWriteStream & writeVarstr32(const void * s, size_t len)
    {
        if (len > 0xFFFFFFFF) throw WriteStreamExn("writeVarstr32: varstr too big");
        return writeUint32(uint32_t(len)).writeByte(s, len);
    }
    BinWriteStream & writeVarwstring32(const std::wstring &ws){
        size_t len = ws.size() * 2;
        return writeUint32((uint32_t)len).writeByte(ws.data(), len);
    }
    virtual ~BinWriteStream() {}
public:
    size_t replace(size_t pos, const void * data, size_t rplen) {
        m_buffer.replace(pos, (const char*)data, rplen);
        return pos + rplen;
    }
    size_t replaceUint8(size_t pos, uint8_t u8)    { return replace(pos, &u8, 1); }
    size_t replaceUint16(size_t pos, uint16_t u16) {
        u16 = xhtons(u16);
        return replace(pos, &u16, 2);
    }
    size_t replaceUint32(size_t pos, uint32_t u32) {
        u32 = xhtonl(u32);
        return replace(pos, &u32, 4);
    }
protected:
    PackBuffer & m_buffer;
    size_t m_offset;
};

class BinReadStream
{
public:
    uint16_t xntohs(uint16_t i16) const { return XNTOHS(i16); }
    uint32_t xntohl(uint32_t i32) const { return XNTOHL(i32); }
    uint64_t xntohll(uint64_t i64) const { return XNTOHLL(i64); }

    BinReadStream(const void * data, size_t size)
    {
        reset(data, size);
    }
    void reset(const void * data, size_t size) const
    {
        m_data = (const char *)data;
        m_size = size;
    }

    virtual ~BinReadStream() { m_data = NULL;  }

    operator const void *() const { return m_data; }
    bool operator!() const  { return (NULL == m_data); }

    std::string readVarstr() const
    {
        Varstr vs = readVarstr_ptr();
        return std::string(vs.data(), vs.size());
    }

    std::string readVarstr32() const
    {
        Varstr vs = readVarstr32_ptr();
        return std::string(vs.data(), vs.size());
    }
    std::wstring readVarwstring32() const
    {
        Varstr vs = readVarstr32_ptr();
        return std::wstring((wchar_t *)vs.data(), vs.size() / 2);
    }
    std::string pop_fetch(size_t k) const
    {
        return std::string(pop_fetch_ptr(k), k);
    }

    void finish() const
    {
        if (!empty()) throw ReadStreamExn("finish: too much data");
    }

    uint8_t readUint8() const
    {
        if (m_size < 1u)
            throw ReadStreamExn("readUint8: not enough data");

        uint8_t i8 = *((uint8_t*)m_data);
        m_data += 1u; m_size -= 1u;
        return i8;
    }

    uint16_t readUint16() const
    {
        if (m_size < 2u)
            throw ReadStreamExn("readUint16: not enough data");

        uint16_t i16 = *((uint16_t*)m_data);
        i16 = xntohs(i16);

        m_data += 2u; m_size -= 2u;
        return i16;
    }

    uint32_t readUint32() const
    {
        if (m_size < 4u)
            throw ReadStreamExn("readUint32: not enough data");
        uint32_t i32 = *((uint32_t*)m_data);
        i32 = xntohl(i32);
        m_data += 4u; m_size -= 4u;
        return i32;
    }

    uint32_t peekUint32() const
    {
        if (m_size < 4u)
            throw ReadStreamExn("peekUint32: not enough data");
        uint32_t i32 = *((uint32_t*)m_data);
        i32 = xntohl(i32);
        return i32;
    }

    uint64_t readUint64() const
    {
        if (m_size < 8u)
            throw ReadStreamExn("readUint64: not enough data");
        uint64_t i64 = *((uint64_t*)m_data);
        i64 = xntohll(i64);
        m_data += 8u; m_size -= 8u;
        return i64;
    }

    Varstr readVarstr_ptr() const
    {
        Varstr vs;
        vs.m_size = readUint16();
        vs.m_data = pop_fetch_ptr(vs.m_size);
        return vs;
    }

    Varstr readVarstr32_ptr() const
    {
        Varstr vs;
        vs.m_size = readUint32();
        vs.m_data = pop_fetch_ptr(vs.m_size);
        return vs;
    }

    const char * pop_fetch_ptr(size_t k) const
    {
        if (m_size < k) {
            throw ReadStreamExn("pop_fetch_ptr: not enough data");
        }

        const char * p = m_data;
        m_data += k; m_size -= k;
        return p;
    }

    bool empty() const    { return m_size == 0; }
    const char * data() const { return m_data; }
    size_t size() const   { return m_size; }

private:
    mutable const char * m_data;
    mutable size_t m_size;
};

inline BinWriteStream & operator << (BinWriteStream & p, bool sign)
{
    p.writeUint8(sign ? 1 : 0);
    return p;
}

inline BinWriteStream & operator << (BinWriteStream & p, uint8_t  i8)
{
    p.writeUint8(i8);
    return p;
}

inline BinWriteStream & operator << (BinWriteStream & p, uint16_t  i16)
{
    p.writeUint16(i16);
    return p;
}

inline BinWriteStream & operator << (BinWriteStream & p, uint32_t  i32)
{
    p.writeUint32(i32);
    return p;
}
inline BinWriteStream & operator << (BinWriteStream & p, uint64_t  i64)
{
        p.writeUint64(i64);
        return p;
}

inline BinWriteStream & operator << (BinWriteStream & p, const std::string & str)
{
    p.writeVarstr(str);
    return p;
}
inline BinWriteStream & operator << (BinWriteStream & p, const std::wstring & str)
{
    p.writeVarwstring32(str);
    return p;
}
inline BinWriteStream & operator << (BinWriteStream & p, const Varstr & pstr)
{
    p.writeVarstr(pstr);
    return p;
}

inline const BinReadStream & operator >> (const BinReadStream & p, Varstr & pstr)
{
    pstr = p.readVarstr_ptr();
    return p;
}

inline const BinReadStream & operator >> (const BinReadStream & p, uint32_t & i32)
{
    i32 =  p.readUint32();
    return p;
}

inline const BinReadStream & operator >> (const BinReadStream & p, uint64_t & i64)
{
        i64 =  p.readUint64();
        return p;
}

inline const BinReadStream & operator >> (const BinReadStream & p, std::string & str)
{
    str = p.readVarstr();
    return p;
}
inline const BinReadStream & operator >> (const BinReadStream & p, std::wstring & str)
{
    str = p.readVarwstring32();
    return p;
}
inline const BinReadStream & operator >> (const BinReadStream & p, uint16_t & i16)
{
    i16 =  p.readUint16();
    return p;
}

inline const BinReadStream & operator >> (const BinReadStream & p, uint8_t & i8)
{
    i8 =  p.readUint8();
    return p;
}

inline const BinReadStream & operator >> (const BinReadStream & p, bool & sign)
{
    sign =  (p.readUint8() == 0) ? false : true;
    return p;
}


template <class T1, class T2>
inline std::ostream& operator << (std::ostream& s, const std::pair<T1, T2>& p)
{
    s << p.first << '=' << p.second;
    return s;
}

template <class T1, class T2>
inline BinWriteStream& operator << (BinWriteStream& s, const std::pair<T1, T2>& p)
{
    s << p.first << p.second;
    return s;
}

template <class T1, class T2>
inline const BinReadStream& operator >> (const BinReadStream& s, std::pair<const T1, T2>& p)
{
    const T1& m = p.first;
    T1 & m2 = const_cast<T1 &>(m);
    s >> m2 >> p.second;
    return s;
}

template <class T1, class T2>
inline const BinReadStream& operator >> (const BinReadStream& s, std::pair<T1, T2>& p)
{
    s >> p.first >> p.second;
    return s;
}


template <class T>
inline BinWriteStream& operator << (BinWriteStream& p, const std::vector<T>& vec)
{
    marshal_container(p, vec);
    return p;
}

template <class T>
inline const BinReadStream& operator >> (const BinReadStream& p, std::vector<T>& vec)
{
    unmarshal_container(p, std::back_inserter(vec));
    return p;
}

template <class T>
inline BinWriteStream& operator << (BinWriteStream& p, const std::set<T>& set)
{
    marshal_container(p, set);
    return p;
}

template <class T1, class T2>
inline BinWriteStream& operator << (BinWriteStream& p, const std::map<T1,T2>& mapField)
{
  marshal_container(p, mapField);
  return p;
}

template <class T>
inline const BinReadStream& operator >> (const BinReadStream& p, std::set<T>& set)
{
    unmarshal_container(p, std::inserter(set, set.begin()));
    return p;
}

template <class T1, class T2>
inline const BinReadStream& operator >> (const BinReadStream& p, std::map<T1,T2>& mapField)
{
  unmarshal_container(p, std::inserter(mapField, mapField.end()));
  return p;
}

template < typename ContainerClass >
inline void marshal_container(BinWriteStream & p, const ContainerClass & c)
{
    p.writeUint32(uint32_t(c.size()));
    for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
        p << *i;
}

template < typename OutputIterator >
inline void unmarshal_container(const BinReadStream & p, OutputIterator i)
{
    for (uint32_t count = p.readUint32(); count > 0; --count)
    {
        typename OutputIterator::container_type::value_type tmp;
        p >> tmp;
        *i = tmp;
        ++i;
    }
}

template < typename ContainerClass >
inline std::ostream & trace_container(std::ostream & os, const ContainerClass & c, char div='\n')
{
    for (typename ContainerClass::const_iterator i = c.begin(); i != c.end(); ++i)
        os << *i << div;
    return os;
}

class Marshallable
{
public:
    virtual void marshal(BinWriteStream &) const = 0;
    virtual void unmarshal(const BinReadStream &) = 0;
    virtual ~Marshallable() {}
    virtual std::ostream & trace(std::ostream & os) const
    {
        return os << "trace Marshallable [ not immplement ]";
    }
};

inline std::ostream & operator << (std::ostream & os, const Marshallable & m)
{
    return m.trace(os);
}

inline BinWriteStream & operator << (BinWriteStream & p, const Marshallable & m)
{
    m.marshal(p);
    return p;
}

inline const BinReadStream & operator >> (const BinReadStream & p, const Marshallable & m)
{
    const_cast<Marshallable &>(m).unmarshal(p);
    return p;
}

inline void PacketToString(const Marshallable &in, std::string &out)
{
    PackBuffer buffer;
    BinWriteStream pack(buffer);
    in.marshal(pack);
    out.assign(pack.data(), pack.size());
}

inline bool StringToPacket(const std::string &strIn, Marshallable &objOut)
{
    try {
        BinReadStream unpack(strIn.data(), strIn.length());
        objOut.unmarshal(unpack);
    } catch (ReadStreamExn e)
    {
        return false;
    }
    return true;
}
}
#endif
