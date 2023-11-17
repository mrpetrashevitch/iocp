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
		
		client::client() : m_iocp(nullptr), m_thread_max(0)
		{
			_wsa_init();
		}

		client::~client()
		{
			m_connection = nullptr;
		}

		void client::run(const char* addr, unsigned short port, int thread_max)
		{
			if (thread_max < 1)
				thread_max = std::thread::hardware_concurrency() * 2;
			m_thread_max = thread_max;

			m_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, m_thread_max);
			m_socket_connect.init(addr, port);
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_socket_connect.get_socket()), m_iocp, static_cast<ULONG_PTR>(io_base::completion_key::io), 0);

			m_socket_connect.bind_before_connect();

			for (int i = 0; i < m_thread_max; ++i)
			{
				std::thread worker_thread(&client::_worker, this, m_iocp, std::ref(m_thread_working));
				worker_thread.detach();
			}

			_connect();
		}

		void client::stop()
		{
		}

		const std::shared_ptr<io_base::i_connection>& client::get_connection()
		{
			return m_connection;
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

	}
}