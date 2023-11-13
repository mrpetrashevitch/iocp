#pragma once
#include "i_server.h"
#include "../../../iocp_lib/src/io_base/io_base.h"
#include "../../../iocp_lib/src/Thread/thread.h"
#include "../../../iocp_lib/src/socket/socket.h"

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

			void init(const char* addr, unsigned short port);
			void run() override;
			void set_on_accepted(callback::on_accepted callback) override;
			void set_on_recv(callback::on_recv callback) override;
			void set_on_disconnected(callback::on_disconnected callback) override;

		private:
			bool m_inited = false;
			std::atomic_int m_connection_counter = 0;
			io_base::socket m_socket_accept;
			std::vector<std::unique_ptr<thread::thread>> m_threads;
			std::mutex m_mut_v;
			std::vector<std::shared_ptr<io_base::connection>> m_connections;
			callback::on_accepted m_on_accepted;
			callback::on_disconnected m_on_disconnected;

		private:
			void _accept();
			bool _on_accept(io_base::i_connection* conn);
			void _on_disconnect(io_base::i_connection* conn);
		};
	}
}