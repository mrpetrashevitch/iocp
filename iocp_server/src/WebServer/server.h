#pragma once
#include "../../../iocp_lib/src/IOBase/i_server.h"
#include "../../../iocp_lib/src/IOBase/io_base.h"
#include "../../../iocp_lib/src/Thread/thread.h"
#include "../../../iocp_lib/src/Socket/socket.h"

#include <vector>

namespace web
{
	namespace io_server
	{
		class server : public io_server::i_server, public io_base::base
		{
		public:
			server();
			~server();

			void init(const SOCKADDR_IN& addr);

			void run() override;

			void set_on_accepted(callback::on_accepted callback) override;
			void set_on_recv(callback::on_recv callback) override;
			void set_on_disconnected(callback::on_disconnected callback) override;

		private:
			bool _inited = false;

			std::atomic_int m_connection_counter = 0;

			io_base::socket _socket_accept;
			std::vector<std::unique_ptr<thread::thread>> _threads;

			std::mutex _mut_v;
			std::vector<std::shared_ptr<io_base::connection>> _connections;

			void _accept();

			callback::on_accepted _on_accepted;
			bool _on_accept(io_base::i_connection* conn);
			callback::on_disconnected _on_disconnected;
			void _on_disconnect(io_base::i_connection* conn);
		
		};
	}
}