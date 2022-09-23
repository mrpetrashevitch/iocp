#pragma once
#include "../defs.h"
#include "winsock2.h" // SOCKET, SOCKADDR_IN

namespace web
{
	namespace io_base
	{
		class socket
		{
			SOCKET _socket;
			sockaddr_in _socket_address;
			bool _inited;
		public:
			socket();
			void init(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port);
			void init(uint addres, ushort port);
			void init(const char* addres, ushort port);
			void init(const sockaddr_in& addr);
			void bind();
			void bind_before_connect();
			void listen();
			void listen(int members);
			const SOCKET& get_socket() const;
			const sockaddr_in& get_socket_address() const;
		};
	}
}