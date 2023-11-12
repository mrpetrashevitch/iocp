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

			std::unique_ptr<io_base::connection> connection_(std::make_unique<io_base::connection>(accepted_socket, this));

			const int accept_ex_result = wsa_acceptex
			(
				_socket_accept.get_socket(),
				accepted_socket,
				connection_->accept_overlapped.buffer,
				0,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
				&bytes, reinterpret_cast<LPOVERLAPPED>(&connection_->accept_overlapped.overlapped)
			);
			
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(accepted_socket), _iocp, static_cast<ULONG_PTR>(io_base::completion_key::io), 0);

			{
				std::lock_guard<std::mutex> lg(_mut_v);
				_connections.push_back(std::move(connection_));
			}
		}

		bool server::_on_accept(io_base::i_connection* conn, SOCKET& socket)
		{
			_accept(); // next accept

			// only TCP
			/*int opt_on = 1;
			if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt_on, sizeof(opt_on)) == SOCKET_ERROR) {
				return false;
			}*/

			if (_on_accepted) 
				return _on_accepted(conn, socket);
			return true;
		}

		void server::_on_disconnect(io_base::i_connection* conn)
		{
			if (_on_disconnected) 
				_on_disconnected(conn);

			{
				std::lock_guard<std::mutex> lg(_mut_v);
				auto item = std::find_if(_connections.begin(), _connections.end(), [&conn](const std::unique_ptr<io_base::connection>& c) { return c->get_socket() == conn->get_socket(); });
				if (item != _connections.end())
					_connections.erase(item);
			}
		}

		server::server()
		{
			on_accepted = std::bind(&server::_on_accept, this, std::placeholders::_1, std::placeholders::_2);
			on_disconnected = std::bind(&server::_on_disconnect, this, std::placeholders::_1);
		}

		server::~server()
		{
			_inited = false;
			_threads.clear();
			_connections.clear();
		}

		void server::init(const SOCKADDR_IN& addr)
		{
			_wsa_init();

			int thread_count = std::thread::hardware_concurrency();
			//thread_count = 1;

			_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, thread_count);
			_socket_accept.init(addr);
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(_socket_accept.get_socket()), _iocp, 0, 0);

			_socket_accept.bind();
			_socket_accept.listen(1);

			for (int i = 0; i < thread_count; ++i)
			{
				std::unique_ptr<thread::thread> _worker_thread(std::make_unique<thread::thread>());
				_worker_thread->set_func(std::bind(&server::_worker, this));
				_worker_thread->set_exit([this]
					{
						PostQueuedCompletionStatus(_iocp, 0, static_cast<ULONG_PTR>(io_base::completion_key::shutdown), nullptr);
					}
				);
				_threads.push_back(std::move(_worker_thread));
			}
			_inited = true;
		}

		void server::run()
		{
			if (!_inited) return;
			for (auto& i : _threads) i->run();
			_accept();
		}

		void server::detach(io_base::i_connection* conn)
		{
			shutdown(conn->get_socket(), SD_BOTH);
			closesocket(conn->get_socket());
		}

		bool server::send(io_base::i_connection* conn, const void* data, int size)
		{
			if (!_inited) return false;
			return _send(reinterpret_cast<io_base::connection*>(conn), data, size);
		}

		void server::set_on_accepted(callback::on_accepted callback)
		{
			_on_accepted = callback;
		}

		void server::set_on_recv(callback::on_recv callback)
		{
			base::set_on_recv(callback);
		}

		void server::set_on_disconnected(callback::on_disconnected callback)
		{
			_on_disconnected = callback;
		}
	}
}