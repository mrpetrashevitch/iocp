#include "connection.h"
#include "../wsa_ex/wsa_ex.h"

#include <mswsock.h>
#include <Ws2tcpip.h>

namespace web
{
	namespace io_base
	{
		connection::connection(SOCKET socket, int id)
		{
			m_id = id;
			ZeroMemory(&m_addr, sizeof(m_addr));
			m_socket = socket;
			accept_overlapped.conn = this;
			connect_overlapped.conn = this;
			disconnect_overlapped.conn = this;
			recv_overlapped.conn = this;
			send_overlapped.conn = this;
		}

		connection::~connection()
		{
			int deasd = 1;
		}

		void connection::set_addr(const SOCKADDR_IN& addr)
		{
			char s[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET, &addr.sin_addr, s, sizeof s);

			m_addr = addr;
			m_addr_str = std::string(s);
		}

		int connection::get_id()
		{
			return m_id;
		}

		const std::string& connection::get_addr()
		{
			return m_addr_str;
		}

		bool connection::_recv_async()
		{
			DWORD flags = 0;
			auto result = WSARecv(m_socket.load(), recv_overlapped.buffer.get_wsabuf(), 1, nullptr, &flags, &recv_overlapped.overlapped, nullptr);
			return result == 0 || (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING);
		}

		bool connection::_send_async()
		{
			DWORD bytes;
			auto result = WSASend(m_socket.load(), send_overlapped.buffer.get_wsabuf(), 1, &bytes, 0, &send_overlapped.overlapped, nullptr);
			return result == 0 || (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING);
		}

		void connection::self_lock(std::shared_ptr<connection> conn)
		{
			m_self = conn;
		}

		void connection::self_unlock()
		{
			m_self = std::shared_ptr<connection>();
		}

		std::shared_ptr<connection> connection::get()
		{
			return m_self.load();
		}

		bool connection::send_async(const void* data, int size)
		{
			bool error = true;
			bool empty = true;
			{
				std::lock_guard<web_buffer_send> lg(send_overlapped.buffer);
				empty = send_overlapped.buffer.empty();
				error = !send_overlapped.buffer.add_data(data, size);
			}

			if (error)
				return false;

			if (empty)
				return _send_async();
			return true;
		}

		bool connection::disconnect_async()
		{
			DWORD bytes = 0;
			return wsa::DisconnectEx
			(
				m_socket.load(),
				&disconnect_overlapped.overlapped,
				TF_REUSE_SOCKET,
				0
			);
		}

	}
}