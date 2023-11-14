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
			bool init(const char* addres, unsigned short port);
			bool bind();
			bool bind_before_connect();
			bool listen();
			bool listen(int members);
			bool close();
			const SOCKET& get_socket() const;
			const sockaddr_in& get_socket_address() const;

		private:
			SOCKET m_socket;
			sockaddr_in m_socket_address;
			bool m_inited;
		};
	}
}