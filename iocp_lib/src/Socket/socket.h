#pragma once
#include "winsock2.h" // SOCKET, SOCKADDR_IN

namespace web
{
	namespace io_base
	{
		class socket
		{
		public:
			socket();
			void init(const char* addres, unsigned short port);
			void bind();
			void bind_before_connect();
			void listen();
			void listen(int members);
			const SOCKET& get_socket() const;
			const sockaddr_in& get_socket_address() const;

		private:
			SOCKET m_socket;
			sockaddr_in m_socket_address;
			bool m_inited;
		};
	}
}