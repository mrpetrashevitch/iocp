#include "connection.h"

#include <Ws2tcpip.h>
#include <mswsock.h>

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

		bool wsa_disconnectex(_In_ SOCKET s,
			_Inout_opt_ LPOVERLAPPED lpOverlapped,
			_In_ DWORD  dwFlags,
			_In_ DWORD  dwReserved)
		{
			static LPFN_DISCONNECTEX disconnectex_func = nullptr;
			if (!disconnectex_func)
			{
				GUID disconnectex_guid = WSAID_DISCONNECTEX;
				DWORD bytes_returned;
				if (WSAIoctl(s,
					SIO_GET_EXTENSION_FUNCTION_POINTER,
					&disconnectex_guid,
					sizeof(disconnectex_guid),
					&disconnectex_func,
					sizeof(disconnectex_func),
					&bytes_returned,
					NULL,
					NULL))
					return false;
			}

			const int result = disconnectex_func
			(
				s,
				lpOverlapped,
				dwFlags,
				dwReserved
			);

			return result == TRUE || WSAGetLastError() == WSA_IO_PENDING;
		}

		bool connection::disconnect_async()
		{
			DWORD bytes = 0;
			return wsa_disconnectex
			(
				m_socket.load(),
				&disconnect_overlapped.overlapped,
				TF_REUSE_SOCKET,
				0
			);
		}

	}
}