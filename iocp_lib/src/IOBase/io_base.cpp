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

		void base::_recv_async(connection* conn)
		{
			DWORD flags = 0;
			DWORD bytes_transferred = 0;
			WSARecv(conn->get_socket(), conn->recv_overlapped.buffer.get_wsabuf(), 1, &bytes_transferred, &flags, &conn->recv_overlapped.overlapped, 0);
		}

		void base::_send_async(connection* conn)
		{
			DWORD bytes;
			WSASend(conn->get_socket(), conn->send_overlapped.buffer.get_wsabuf(), 1, &bytes, 0, &conn->send_overlapped.overlapped, 0);
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
				_send_async(conn);
			return true;
		}

		void terminate_connection(connection* conn)
		{
			shutdown(conn->get_socket(), SD_BOTH);
			closesocket(conn->get_socket());
		}

		void base::_worker()
		{
			try
			{
				DWORD success;
				DWORD bytes_transferred;
				ULONG_PTR completion_key;
				overlapped_base* overlapped = nullptr;

				while (true)
				{
					success = GetQueuedCompletionStatus(_iocp, &bytes_transferred, &completion_key, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

					// terminate flag
					if (completion_key == 1)
					{
						break;
					}

					if (!success)
					{
						if (on_disconnected)
							on_disconnected(overlapped->connection);
						continue;
					}

					if (overlapped->type == overlapped_type::accept)
					{
						overlapped->connection->set_addr(*(sockaddr_in*)&overlapped->connection->accept_overlapped.buffer[38]);

						if (on_accepted)
						{
							if (!on_accepted(overlapped->connection, overlapped->connection->get_socket()))
								terminate_connection(overlapped->connection);
						}

						// start recv packets
						_recv_async(overlapped->connection);
						continue;
					}

					if (overlapped->type == overlapped_type::connect)
					{
						if (on_connected)
							on_connected(overlapped->connection, overlapped->connection->get_socket());

						// start recv packets
						_recv_async(overlapped->connection);
						continue;
					}

					if (bytes_transferred == 0)
					{
						if (on_disconnected)
							on_disconnected(overlapped->connection);
						continue;
					}

					if (overlapped->type == overlapped_type::recv)
					{
						auto& over = overlapped->connection->recv_overlapped;
						if (over.buffer.add_total_recv(bytes_transferred))
						{
							while (42)
							{
								auto buff = over.buffer.get_curr_buffer();
								auto size = over.buffer.get_cerr_buffer_size();

								if (!size)
									break;

								if (on_recv)
									size = on_recv(overlapped->connection, buff, size);

								if (!over.buffer.add_total_read(size))
								{
									break;
								}
							}

							if (!over.buffer.is_error())
							{
								over.buffer.fit();
								_recv_async(overlapped->connection);
							}
							else
							{
								terminate_connection(overlapped->connection);
							}
						}
						else
						{
							terminate_connection(overlapped->connection);
						}
						continue;
					}

					if (overlapped->type == overlapped_type::send)
					{
						auto& over = overlapped->connection->send_overlapped;

						bool error = true;
						bool empty = true;
						{
							std::lock_guard<web_buffer_send> lg(over.buffer);
							error = !over.buffer.add_total_send(bytes_transferred);
							empty = over.buffer.empty();
						}

						if (error)
						{
							terminate_connection(overlapped->connection);
						}
						else if (!empty)
						{
							_send_async(overlapped->connection);
						}
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