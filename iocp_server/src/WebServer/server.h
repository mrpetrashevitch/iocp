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
			bool _inited = false;

			io_base::socket _socket_accept;
			std::vector<std::unique_ptr<thread::thread>> _threads;

			std::mutex _mut_v;
			std::vector<std::unique_ptr<io_base::connection>> _connections;

			void _accept();

			callback::on_accepted _on_accepted;
			void _on_accept(io_base::i_connection* conn, SOCKET& socket);
			callback::on_disconnected _on_disconnected;
			void _on_disconnect(io_base::i_connection* conn);
		public:
			server();
			~server();

			void init(const SOCKADDR_IN& addr);

			void run() override;
			void detach(io_base::i_connection* conn) override;

			void send_packet_async(io_base::i_connection* conn, packet::i_packet_network* packet) override;
			void send_packet_async(io_base::i_connection* conn, const std::shared_ptr<packet::i_packet_network>& packet) override;

			void set_on_accepted(callback::on_accepted callback) override;
			void set_on_recv(callback::on_recv callback) override;
			void set_on_send(callback::on_send callback) override;
			void set_on_disconnected(callback::on_disconnected callback) override;
		};
	}
}