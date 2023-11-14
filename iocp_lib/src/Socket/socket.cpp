#include "socket.h"
#pragma warning(disable: 4996)
#pragma warning(disable: 4200)

namespace web
{
	namespace io_base
	{
		socket::socket() :m_socket(), m_inited(false)
		{
			ZeroMemory(&m_socket_address, sizeof(m_socket_address));
		}

		bool socket::init(const char* addres, unsigned short port)
		{
			sockaddr_in serv_adr;
			serv_adr.sin_family = AF_INET;
			serv_adr.sin_addr.s_addr = inet_addr(addres);
			serv_adr.sin_port = htons(port);
			m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (m_socket == INVALID_SOCKET)
				return false;
			m_socket_address = serv_adr;
			m_inited = true;
			return true;
		}

		bool socket::bind()
		{
			const unsigned so_reuseaddr = 1;
			//::setsockopt(_socket_accept, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&so_reuseaddr), sizeof(so_reuseaddr));
			return ::bind(m_socket, reinterpret_cast<SOCKADDR*>(&m_socket_address), sizeof(m_socket_address)) != INVALID_SOCKET;
		}

		bool socket::bind_before_connect()
		{
			// ConnectEx requires the socket to be initially bound
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = 0;

			return ::bind(m_socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) != INVALID_SOCKET;
		}

		bool socket::listen()
		{
			return listen(1);
		}

		bool socket::listen(int backlog)
		{
			return ::listen(m_socket, backlog) != INVALID_SOCKET;
		}

		bool socket::close()
		{
			auto res = shutdown(m_socket, SD_BOTH);
			auto err = WSAGetLastError();
			auto res2 = closesocket(m_socket) == 0;
			auto err2 = WSAGetLastError();
			/*if (shutdown(m_socket, SD_BOTH) == 0 && closesocket(m_socket) == 0)
			{
				m_socket = 0;
				return true;
			}*/
			return false;
		}

		const SOCKET& socket::get_socket() const
		{
			return m_socket;
		}

		const sockaddr_in& socket::get_socket_address() const
		{
			return m_socket_address;
		}
	}
}