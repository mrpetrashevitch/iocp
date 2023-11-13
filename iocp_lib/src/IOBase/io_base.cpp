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

		bool base::accept_handler(connection* conn)
		{
			conn->set_addr(*(sockaddr_in*)&conn->accept_overlapped.buffer[38]);

			if (on_accepted)
				if (!on_accepted(conn))
					return false;

			if (!conn->_recv_async())
				return false;
			return true;
		}

		bool base::connect_handler(connection* conn)
		{
			if (on_connected)
				on_connected(conn);

			if (!conn->_recv_async())
				return false;
			return true;
		}

		bool base::disconnect_handler(connection* conn)
		{
			auto soket = conn->m_socket.exchange(0);
			if (!soket)
				return false;

			shutdown(soket, SD_BOTH);
			closesocket(soket);

			if (on_disconnected)
				on_disconnected(conn);
			return true;
		}

		bool base::recv_handler(connection* conn, DWORD bytes_transferred)
		{
			auto& over = conn->recv_overlapped;
			if (!over.buffer.add_total_recv(bytes_transferred))
				return false;

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
				return false;

			over.buffer.fit();
			if (!conn->_recv_async())
				return false;
			return true;
		}

		bool base::send_handler(connection* conn, DWORD bytes_transferred)
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
				return false;

			if (!empty)
				if (!conn->_send_async())
					return false;
			return true;
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

					std::shared_ptr<connection> conn = nullptr;
					if (overlapped == nullptr || (conn = overlapped->conn->get(), conn == nullptr))
					{
						continue;
					}

					if (!success || overlapped->type == overlapped_type::disconnect)
					{
						disconnect_handler(conn.get());
						continue;
					}

					if (overlapped->type == overlapped_type::accept)
					{
						if (!accept_handler(conn.get()))
							disconnect_handler(conn.get());
						continue;
					}

					if (overlapped->type == overlapped_type::connect)
					{
						if (!connect_handler(conn.get()))
							disconnect_handler(conn.get());
						continue;
					}

					if (bytes_transferred == 0)
					{
						disconnect_handler(conn.get());
						continue;
					}

					if (overlapped->type == overlapped_type::recv)
					{
						if (!recv_handler(conn.get(), bytes_transferred))
							disconnect_handler(conn.get());
						continue;
					}

					if (overlapped->type == overlapped_type::send)
					{
						if (!send_handler(conn.get(), bytes_transferred))
							disconnect_handler(conn.get());
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