#pragma once
#include "../CallBack/i_callback.h"

#include <memory> // shared_ptr

namespace web
{
	namespace io_server
	{
		struct i_server
		{
			virtual ~i_server() {};
			virtual void run() = 0;
			virtual void detach(io_base::i_connection* conn) = 0;

			virtual void send_packet_async(io_base::i_connection* conn, packet::i_packet_network* packet) = 0;
			virtual void send_packet_async(io_base::i_connection* conn, const std::shared_ptr<packet::i_packet_network>& packet) = 0;

			virtual void set_on_accepted(callback::on_accepted callback) = 0;
			virtual void set_on_recv(callback::on_recv callback) = 0;
			virtual void set_on_send(callback::on_send callback) = 0;
			virtual void set_on_disconnected(callback::on_disconnected callback) = 0;
		};
	}
}