#pragma once
#include "i_client.h"
#include "../../../iocp_lib/src/io_base/io_base.h"
#include "../../../iocp_lib/src/Thread/thread.h"
#include "../../../iocp_lib/src/socket/socket.h"

namespace web
{
	namespace io_client
	{
		class client : public io_client::i_client, public io_base::base
		{
		public:
			client();
			~client();

			void run(const char* addr, unsigned short port, int thread_max) override;
			void stop() override;
			const std::shared_ptr<io_base::i_connection>& get_connection() override;

			void set_on_connected(callback::on_connected callback) override;
			void set_on_recv(callback::on_recv callback) override;
			void set_on_disconnected(callback::on_disconnected callback) override;

		private:
			HANDLE m_iocp;
			int m_thread_max;
			std::atomic<int> m_thread_working;
			io_base::socket m_socket_connect;
			std::shared_ptr<io_base::connection> m_connection;

		private:
			void _connect();
		};
	}
}
