#include "connection.h"

namespace web
{
	namespace io_base
	{
		connection::connection(SOCKET socket, void* owner)
		{
			m_owner = owner;
			ZeroMemory(&m_addr, sizeof(m_addr));
			m_socket = socket;
			accept_overlapped.connection = this;
			connect_overlapped.connection = this;
			disconnect_overlapped.connection = this;
			recv_overlapped.connection = this;
			send_overlapped.connection = this;
		}

		connection::~connection()
		{
		}

		void* connection::get_owner()
		{
			return m_owner;
		}

		SOCKET& connection::get_socket()
		{
			return m_socket;
		}

		void connection::set_addr(const SOCKADDR_IN& addr)
		{
			m_addr = addr;
		}

		SOCKADDR_IN& connection::get_addr()
		{
			return m_addr;
		}
	}
}