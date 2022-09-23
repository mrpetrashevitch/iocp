#pragma once
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t, std::uint32_t,std::uint8_t
#include <basetsd.h> // __int3264

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef __int3264 int_t;
typedef unsigned __int3264 uint_t;

typedef std::uint8_t byte_net;
typedef std::int32_t int_net;
typedef std::uint32_t uint_net;