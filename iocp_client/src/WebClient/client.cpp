#include "client.h"
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
			static LPFN_CONNECTEX connectx_func = nullptr;
			if (!connectx_func)
			{
				GUID acceptex_guid = WSAID_CONNECTEX;
				DWORD bytes_returned;
				WSAIoctl(_socket_connect.get_socket(),
					SIO_GET_EXTENSION_FUNCTION_POINTER,
					&acceptex_guid,
					sizeof(acceptex_guid),
					&connectx_func,
					sizeof(connectx_func),
					&bytes_returned, NULL, NULL);
			}

			DWORD bytes = 0;

			std::unique_ptr<io_base::connection> conn(std::make_unique<io_base::connection>(_socket_connect.get_socket(), this));
			auto& addr = _socket_connect.get_socket_address();
			const int connect_ex_result = connectx_func
			(
				_socket_connect.get_socket(),
				(SOCKADDR*)&addr,
				sizeof(addr),
				0,
				0,
				&bytes,
				reinterpret_cast<LPOVERLAPPED>(&conn->connect_overlapped.overlapped)
			);

			_connection = std::move(conn);
		}

		client::client()
		{
		}

		client::~client()
		{
			_inited = false;
			_threads.clear();
			_connection.reset(nullptr);
		}

		void client::init(const SOCKADDR_IN& addr)
		{
			_wsa_init();

			int thread_count = std::thread::hardware_concurrency();

			_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, thread_count);
			_socket_connect.init(addr);
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(_socket_connect.get_socket()), _iocp, 0, 0);

			_socket_connect.bind_before_connect();

			for (int i = 0; i < thread_count; ++i)
			{
				std::unique_ptr<thread::thread> _worker_thread(std::make_unique<thread::thread>());
				_worker_thread->set_func(std::bind(&client::_worker, this));
				_worker_thread->set_exit([this]
					{
						PostQueuedCompletionStatus(_iocp, 0, 1, nullptr);
					}
				);
				_threads.push_back(std::move(_worker_thread));
			}
			_inited = true;
		}

		void client::run()
		{
			if (!_inited) return;
			for (auto& i : _threads) i->run();
			_connect();
		}


		void client::detach()
		{
			if (!_inited) return;
			closesocket(_connection->get_socket());
		}

		bool client::send(const void* data, int size)
		{
			if (!_inited) return false;
			return _send(reinterpret_cast<io_base::connection*>(_connection.get()), data, size);
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