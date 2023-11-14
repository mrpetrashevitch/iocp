#include "client.h"
#include "../wsa_ex/wsa_ex.h"

#include <WinInet.h>
#include <Ws2tcpip.h>
#include <mswsock.h>

#pragma warning(disable: 4996)
#pragma warning(disable: 4200)

namespace web
{
	namespace io_client
	{
		void client::_connect()
		{
			std::shared_ptr<io_base::connection> conn(std::make_unique<io_base::connection>(m_socket_connect.get_socket(), 0));
			conn->self_lock(conn);

			auto& addr = m_socket_connect.get_socket_address();
			DWORD bytes = 0;
			const int connect_ex_result = wsa::ConnectEx
			(
				m_socket_connect.get_socket(),
				(SOCKADDR*)&addr,
				sizeof(addr),
				0,
				0,
				&bytes,
				reinterpret_cast<LPOVERLAPPED>(&conn->connect_overlapped.overlapped)
			);

			m_connection = std::move(conn);
		}

		client::client()
		{
		}

		client::~client()
		{
			m_inited = false;
			m_threads.clear();
			m_connection = nullptr;
		}

		void client::init(const char* addr, unsigned short port)
		{
			_wsa_init();

			int thread_count = std::thread::hardware_concurrency();

			m_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, thread_count);
			m_socket_connect.init(addr, port);
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_socket_connect.get_socket()), m_iocp, static_cast<ULONG_PTR>(io_base::completion_key::io), 0);

			m_socket_connect.bind_before_connect();

			/*for (int i = 0; i < thread_count; ++i)
			{
				std::unique_ptr<thread::thread> _worker_thread(std::make_unique<thread::thread>());
				_worker_thread->set_func(std::bind(&client::_worker, this));
				_worker_thread->set_exit([this]
					{
						PostQueuedCompletionStatus(m_iocp, 0, static_cast<ULONG_PTR>(io_base::completion_key::shutdown), nullptr);
					}
				);
				m_threads.push_back(std::move(_worker_thread));
			}*/
			m_inited = true;
		}

		void client::run()
		{
			if (!m_inited) return;
			for (auto& i : m_threads) i->run();
			_connect();
		}

		bool client::send_async(const void* data, int size)
		{
			if (!m_connection) return false;
			return m_connection->send_async(data, size);
		}

		bool client::disconnect_async()
		{
			if (!m_connection) return false;
			return m_connection->disconnect_async();
		}

		void client::set_on_connected(callback::on_connected callback)
		{
			base::set_on_connected(callback);
		}

		void client::set_on_recv(callback::on_recv callback)
		{
			base::set_on_recv(callback);
		}

		void client::set_on_disconnected(callback::on_disconnected callback)
		{
			base::set_on_disconnected(callback);
		}
	}
}