#include "server.h"
#include "../wsa_ex/wsa_ex.h"

#include <algorithm>
#include <Ws2tcpip.h>
#include <mswsock.h>

#pragma warning(disable: 4996)
#pragma warning(disable: 4200)


namespace web
{
	namespace io_server
	{
		server::server()
			: m_state(server_state::stoped), m_error(false), m_connection_counter(0), m_iocp(nullptr)
		{
			if (!_wsa_init())
				_set_error("Failed to init wsa");

			on_accepted = std::bind(&server::_on_accept, this, std::placeholders::_1);
			on_disconnected = std::bind(&server::_on_disconnect, this, std::placeholders::_1);
		}

		server::~server()
		{
			stop();
		}

		bool server::run(const char* addr, unsigned short port, int thread_max, int connection_max)
		{
			if (m_error) return false;

			if (connection_max < 1)
				connection_max = 10000;
			m_connection_max = connection_max;

			if (thread_max < 1)
				thread_max = std::thread::hardware_concurrency() * 2;
			m_thread_max = thread_max;

			m_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, m_thread_max);
			if (m_iocp == 0)
			{ 
				_set_error("Failed to create io completion port");
				return false;
			}

			if (!m_socket_accept.init(addr, port))
			{
				_set_error("Failed to init accept socket");
				return false;
			}

			if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_socket_accept.get_socket()), m_iocp, 0, 0))
			{
				_set_error("Failed to add accept socket to iocp");
				return false;
			}

			if (!m_socket_accept.bind())
			{
				_set_error("Failed to bind accept socket");
				return false;
			}

			if (!m_socket_accept.listen(1))
			{
				_set_error("Failed to listen accept socket");
				return false;
			}

			m_state = server_state::runing;

			for (size_t i = 0; i < m_thread_max; i++)
			{
				std::thread worker_thread(&server::_worker, this, m_iocp, std::ref(m_thread_working));
				worker_thread.detach();
			}

			{
				std::lock_guard<std::mutex> lg(m_mut_v);
				m_connections.clear();
				if (m_connections.size() < m_connection_max)
					_accept();
			}

			return true;
		}

		void server::stop()
		{
			auto state = m_state.exchange(server_state::stoping);
			if (state == server_state::stoped || state == server_state::stoping)
				return;

			m_socket_accept.close();

			{
				std::lock_guard<std::mutex> lg(m_mut_v);

				for (auto& conn: m_connections)
				{
					conn->disconnect_async();
				}
			}

			int total_conn;
			do
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				std::lock_guard<std::mutex> lg(m_mut_v);
				total_conn = (int)m_connections.size();
			} while (total_conn > 0);

			for (size_t i = 0; i < m_thread_max; i++)
			{
				PostQueuedCompletionStatus(m_iocp, 0, static_cast<ULONG_PTR>(io_base::completion_key::shutdown), nullptr);
			}

			do
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			} while (m_thread_working > 0);

			

		}

		void server::set_on_accepted(callback::on_accepted callback)
		{
			m_on_accepted = callback;
		}

		void server::set_on_recv(callback::on_recv callback)
		{
			base::set_on_recv(callback);
		}

		void server::set_on_disconnected(callback::on_disconnected callback)
		{
			m_on_disconnected = callback;
		}

		bool server::_accept()
		{
			SOCKET client_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

			if (client_socket == INVALID_SOCKET)
			{
				_set_error("Failed to create client socket");
				return false;
			}

			// disble send buffering
			int opt = 0;
			setsockopt(client_socket, SOL_SOCKET, SO_SNDBUF, (char*)&opt, sizeof(opt));

			DWORD bytes = 0;

			int id = m_connection_counter++;

			std::shared_ptr<io_base::connection> conn(std::make_shared<io_base::connection>(client_socket, id));
			conn->self_lock(conn);

			if (!wsa::AcceptEx
			(
				m_socket_accept.get_socket(),
				client_socket,
				conn->accept_overlapped.buffer,
				0,
				sizeof(sockaddr_in) + 16,
				sizeof(sockaddr_in) + 16,
				&bytes,
				reinterpret_cast<LPOVERLAPPED>(&conn->accept_overlapped.overlapped)
			))
			{
				conn->self_unlock();
				closesocket(client_socket);
				_set_error("Failed to acceptex client socket");
				return false;
			}

			if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_socket), m_iocp, static_cast<ULONG_PTR>(io_base::completion_key::io), 0))
			{
				conn->self_unlock();
				closesocket(client_socket);
				_set_error("Failed to add client socket to iocp");
				return false;
			}

			m_connections.push_back(std::move(conn));

			return true;
		}

		void server::_on_accept(io_base::i_connection* conn)
		{
			//m_socket_accept.close();
			{
				std::lock_guard<std::mutex> lg(m_mut_v);
				if (m_connections.size() < m_connection_max && m_state == server_state::runing)
					_accept();
			}
			// only TCP
			/*int opt_on = 1;
			if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt_on, sizeof(opt_on)) == SOCKET_ERROR) {
				return false;
			}*/

			if (m_on_accepted)
				m_on_accepted(conn);
		}

		void server::_on_disconnect(io_base::i_connection* conn)
		{
			bool accepted = false;
			{
				std::lock_guard<std::mutex> lg(m_mut_v);
				int use_accept = false;

				auto item = std::find_if(m_connections.begin(), m_connections.end(), [&conn](const std::shared_ptr<io_base::connection>& c) { return c->get_id() == conn->get_id(); });
				if (item != m_connections.end())
				{
					(*item)->self_unlock();
					accepted = (*item)->accepted;
					use_accept = m_connections.size() == m_connection_max;
					m_connections.erase(item);
				}

				if (use_accept && m_state == server_state::runing)
					_accept();
			}

			if (m_on_disconnected && accepted)
				m_on_disconnected(conn);
		}

		void server::_set_error(const char* msg)
		{
			m_error = true;
			int err_no = WSAGetLastError();
			LPVOID lpMsgBuf;
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, NULL);

			std::string s = std::string(msg);
			s += std::string(", error: (") + std::to_string(err_no) + std::string(") ");
			s += std::string((LPCSTR)lpMsgBuf);
			s += std::string("\n");
			LocalFree(lpMsgBuf);

			std::lock_guard<std::mutex> lock(m_error_msg_lock);
			m_error_msgs += s;
		}
	}
}