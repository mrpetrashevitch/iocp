#pragma once
#include "../../../iocp_lib/src/IOBase/i_client.h"
#include "../../../iocp_lib/src/IOBase/io_base.h"
#include "../../../iocp_lib/src/Thread/thread.h"
#include "../../../iocp_lib/src/Socket/socket.h"

namespace web
{
	namespace io_client
	{
		class client : public io_client::i_client, public io_base::base
		{
		public:
			client();
			~client();

			void init(const SOCKADDR_IN& addr);
			void run() override;

			bool send_async(const void* data, int size) override;
			bool disconnect_async() override;

			void set_on_connected(callback::on_connected callback) override;
			void set_on_recv(callback::on_recv callback) override;
			void set_on_disconnected(callback::on_disconnected callback) override;
		private:

			bool _inited = false;

			io_base::socket _socket_connect;

			std::vector<std::unique_ptr<thread::thread>> _threads;
			std::shared_ptr<io_base::connection> _connection;

			void _connect();
		
		};
	}
}
