#pragma once
#include "i_server.h"
#include "../io_base/io_base.h"
#include "../socket/socket.h"

#include <vector>

namespace web
{
	namespace io_server
	{
		enum class server_state : char
		{
			stoped,
			runing,
			stoping
		};


		class server : public io_server::i_server, public io_base::base
		{
		public:
			server();
			~server();

			bool run(const char* addr, unsigned short port, int thread_max, int connection_max) override;
			void stop() override;
			void set_on_accepted(callback::on_accepted callback) override;
			void set_on_recv(callback::on_recv callback) override;
			void set_on_disconnected(callback::on_disconnected callback) override;

		private:
			std::atomic<server_state> m_state;
			bool m_error;
			std::mutex m_error_msg_lock;
			std::string m_error_msgs;

			HANDLE m_iocp;

			std::atomic<int> m_connection_counter;
			io_base::socket m_socket_accept;

			int m_thread_max;
			std::atomic<int> m_thread_working;

			std::mutex m_mut_v;
			int m_connection_max;
			std::vector<std::shared_ptr<io_base::connection>> m_connections;

			callback::on_accepted m_on_accepted;
			callback::on_disconnected m_on_disconnected;

		private:
			bool _accept();
			void _on_accept(io_base::i_connection* conn);
			void _on_disconnect(io_base::i_connection* conn);
			void _set_error(const char* msg);
		};
	}
}