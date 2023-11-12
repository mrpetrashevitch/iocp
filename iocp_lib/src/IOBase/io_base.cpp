#include "io_base.h"

#include <Ws2tcpip.h>
#include <mswsock.h>


#pragma warning(disable: 4996)
#pragma warning(disable: 4200)

namespace web
{
	namespace io_base
	{
		bool base::_wsa_init()
		{
			WSADATA wsaData;
			if (WSAStartup(WINSOCK_VERSION, &wsaData))
			{
				//WSACleanup();
				return false;
			}
			return true;
		}

		bool base::_recv_async(connection* conn)
		{
			DWORD flags = 0;
			auto result = WSARecv(conn->get_socket(), conn->recv_overlapped.buffer.get_wsabuf(), 1, nullptr, &flags, &conn->recv_overlapped.overlapped, nullptr);
			return result == 0 || (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING);
		}

		bool base::_send_async(connection* conn)
		{
			DWORD bytes;
			auto result = WSASend(conn->get_socket(), conn->send_overlapped.buffer.get_wsabuf(), 1, &bytes, 0, &conn->send_overlapped.overlapped, nullptr);
			return result == 0 || (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING);
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

		bool _disconnect_async(connection* conn)
		{
			DWORD bytes = 0;
			return wsa_disconnectex
			(
				conn->get_socket(),
				&conn->disconnect_overlapped.overlapped,
				TF_REUSE_SOCKET,
				0
			);
		}

		bool base::_send(connection* conn, const void* data, int size)
		{
			web_buffer_send& buffer = conn->send_overlapped.buffer;
			bool error = true;
			bool empty = true;
			{
				std::lock_guard<web_buffer_send> lg(buffer);
				empty = buffer.empty();
				error = !buffer.add_data(data, size);
			}

			if (error)
				return false;

			if (empty)
				return _send_async(conn);
			return true;
		}

		void base::accept_handler(connection* conn)
		{
			conn->set_addr(*(sockaddr_in*)&conn->accept_overlapped.buffer[38]);

			if (on_accepted)
				if (!on_accepted(conn, conn->get_socket()))
					_disconnect_async(conn);

			if (!_recv_async(conn))
				_disconnect_async(conn);
		}

		void base::connect_handler(connection* conn)
		{
			if (on_connected)
				on_connected(conn, conn->get_socket());

			if (!_recv_async(conn))
				_disconnect_async(conn);
		}

		void base::disconnect_handler(connection* conn)
		{
			shutdown(conn->get_socket(), SD_BOTH);
			closesocket(conn->get_socket());

			if (on_disconnected)
				on_disconnected(conn);
		}

		void base::recv_handler(connection* conn, DWORD bytes_transferred)
		{
			auto& over = conn->recv_overlapped;
			if (!over.buffer.add_total_recv(bytes_transferred))
			{
				_disconnect_async(conn);
				return;
			}

			while (42)
			{
				auto size = over.buffer.get_cerr_buffer_size();
				if (!size) break;
				auto buff = over.buffer.get_curr_buffer();

				if (on_recv)
					size = on_recv(conn, buff, size);

				if (!over.buffer.add_total_read(size)) break;
			}

			if (over.buffer.is_error())
			{
				_disconnect_async(conn);
				return;
			}

			over.buffer.fit();
			if (!_recv_async(conn))
				_disconnect_async(conn);
		}

		void base::send_handler(connection* conn, DWORD bytes_transferred)
		{
			auto& over = conn->send_overlapped;

			bool error = true;
			bool empty = true;
			{
				std::lock_guard<web_buffer_send> lg(over.buffer);
				error = !over.buffer.add_total_send(bytes_transferred);
				empty = over.buffer.empty();
			}

			if (error)
			{
				_disconnect_async(conn);
				return;
			}
			
			if (!empty)			
				if (!_send_async(conn))
					_disconnect_async(conn);
		}

		void base::_worker()
		{
			try
			{
				DWORD success;
				DWORD bytes_transferred;
				ULONG_PTR key;
				overlapped_base* overlapped = nullptr;

				while (42)
				{
					success = GetQueuedCompletionStatus(_iocp, &bytes_transferred, &key, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

					if (static_cast<completion_key>(key) == completion_key::shutdown)
					{
						break;
					}

					if (overlapped == nullptr)
					{
						continue;
					}

					if (!success || overlapped->type == overlapped_type::disconnect)
					{
						disconnect_handler(overlapped->connection);
						continue;
					}

					if (overlapped->type == overlapped_type::accept)
					{
						accept_handler(overlapped->connection);
						continue;
					}

					if (overlapped->type == overlapped_type::connect)
					{
						connect_handler(overlapped->connection);
						continue;
					}

					if (bytes_transferred == 0)
					{
						disconnect_handler(overlapped->connection);
						continue;
					}

					if (overlapped->type == overlapped_type::recv)
					{
						recv_handler(overlapped->connection, bytes_transferred);
						continue;
					}

					if (overlapped->type == overlapped_type::send)
					{
						send_handler(overlapped->connection, bytes_transferred);
						continue;
					}

				}
			}
			catch (const std::exception&)
			{
				int a = GetLastError();
			}
		}

		SOCKADDR_IN base::get_sockaddr(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port)
		{
			byte addres[4]{ s_b1,s_b2, s_b3, s_b4 };
			return get_sockaddr(*((unsigned long*)&addres), port);
		}

		SOCKADDR_IN base::get_sockaddr(const char* addres, ushort port)
		{
			return get_sockaddr(inet_addr(addres), port);
		}

		SOCKADDR_IN base::get_sockaddr(uint addres, ushort port)
		{
			SOCKADDR_IN serv_adr;
			serv_adr.sin_family = AF_INET;
			serv_adr.sin_addr.s_addr = addres;
			serv_adr.sin_port = htons(port);
			return serv_adr;
		}
	}
}