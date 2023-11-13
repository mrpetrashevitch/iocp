#include "server.h"
#include <algorithm>

#include <Ws2tcpip.h>
#include <mswsock.h>

#pragma warning(disable: 4996)
#pragma warning(disable: 4200)


namespace web
{
	namespace io_server
	{
		bool wsa_acceptex(
			_In_ SOCKET sListenSocket,
			_In_ SOCKET sAcceptSocket,
			_Out_writes_bytes_(dwReceiveDataLength + dwLocalAddressLength + dwRemoteAddressLength) PVOID lpOutputBuffer,
			_In_ DWORD dwReceiveDataLength,
			_In_ DWORD dwLocalAddressLength,
			_In_ DWORD dwRemoteAddressLength,
			_Out_ LPDWORD lpdwBytesReceived,
			_Inout_ LPOVERLAPPED lpOverlapped)
		{
			static LPFN_ACCEPTEX acceptex_func = nullptr;
			if (!acceptex_func)
			{
				GUID acceptex_guid = WSAID_ACCEPTEX;
				DWORD bytes_returned;
				if (WSAIoctl(sListenSocket,
					SIO_GET_EXTENSION_FUNCTION_POINTER,
					&acceptex_guid,
					sizeof(acceptex_guid),
					&acceptex_func,
					sizeof(acceptex_func),
					&bytes_returned, NULL, NULL))
					return false;
			}

			const int result = acceptex_func
			(
				sListenSocket,
				sAcceptSocket,
				lpOutputBuffer,
				dwReceiveDataLength,
				dwLocalAddressLength, 
				dwRemoteAddressLength,
				lpdwBytesReceived, 
				lpOverlapped
			);

			return result == TRUE || WSAGetLastError() == WSA_IO_PENDING;
		}

		void server::_accept()
		{
			SOCKET accepted_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

			// disble send buffering
			int opt = 0;
			setsockopt(accepted_socket, SOL_SOCKET, SO_SNDBUF, (char*)&opt, sizeof(opt));

			DWORD bytes = 0;

			int id = m_connection_counter++;

			std::shared_ptr<io_base::connection> conn(std::make_shared<io_base::connection>(accepted_socket, id));
			conn->self_lock(conn);

			const int accept_ex_result = wsa_acceptex
			(
				m_socket_accept.get_socket(),
				accepted_socket,
				conn->accept_overlapped.buffer,
				0,
				sizeof(sockaddr_in) + 16,
				sizeof(sockaddr_in) + 16,
				&bytes, 
				reinterpret_cast<LPOVERLAPPED>(&conn->accept_overlapped.overlapped)
			);
			
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(accepted_socket), m_iocp, static_cast<ULONG_PTR>(io_base::completion_key::io), 0);

			{
				std::lock_guard<std::mutex> lg(m_mut_v);
				m_connections.push_back(std::move(conn));
			}
		}

		bool server::_on_accept(io_base::i_connection* conn)
		{
			_accept(); // next accept

			// only TCP
			/*int opt_on = 1;
			if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt_on, sizeof(opt_on)) == SOCKET_ERROR) {
				return false;
			}*/

			if (m_on_accepted) 
				return m_on_accepted(conn);
			return true;
		}

		void server::_on_disconnect(io_base::i_connection* conn)
		{
			if (m_on_disconnected) 
				m_on_disconnected(conn);

			{
				std::lock_guard<std::mutex> lg(m_mut_v);
				auto item = std::find_if(m_connections.begin(), m_connections.end(), [&conn](const std::shared_ptr<io_base::connection>& c) { return c->get_id() == conn->get_id(); });
				if (item != m_connections.end())
				{
					(*item)->self_unlock();
					m_connections.erase(item);
				}
			}
		}

		server::server()
		{
			on_accepted = std::bind(&server::_on_accept, this, std::placeholders::_1);
			on_disconnected = std::bind(&server::_on_disconnect, this, std::placeholders::_1);
		}

		server::~server()
		{
			m_inited = false;
			m_threads.clear();
			m_connections.clear();
		}

		void server::init(const char* addr, unsigned short port)
		{
			_wsa_init();

			int thread_count = std::thread::hardware_concurrency();
			//thread_count = 1;

			m_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, thread_count);
			m_socket_accept.init(addr, port);
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_socket_accept.get_socket()), m_iocp, 0, 0);

			m_socket_accept.bind();
			m_socket_accept.listen(1);

			for (int i = 0; i < thread_count; ++i)
			{
				std::unique_ptr<thread::thread> _worker_thread(std::make_unique<thread::thread>());
				_worker_thread->set_func(std::bind(&server::_worker, this));
				_worker_thread->set_exit([this]
					{
						PostQueuedCompletionStatus(m_iocp, 0, static_cast<ULONG_PTR>(io_base::completion_key::shutdown), nullptr);
					}
				);
				m_threads.push_back(std::move(_worker_thread));
			}
			m_inited = true;
		}

		void server::run()
		{
			if (!m_inited) return;
			for (auto& i : m_threads) i->run();
			_accept();
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
	}
}