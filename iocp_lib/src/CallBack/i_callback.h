#pragma once
#include "../Connection/i_connection.h"
#include "../Packets/i_packet_network.h"
#include <functional> // std::function

namespace web
{
	namespace callback
	{
		typedef std::function<void(io_base::i_connection* conn, SOCKET& socket)> on_accepted;
		typedef std::function<void(io_base::i_connection* conn, SOCKET& socket)> on_connected;
		typedef std::function<void(io_base::i_connection* conn, web::packet::packet_network* packet)> on_recv;
		typedef std::function<void(io_base::i_connection* conn, web::packet::packet_network* packet)> on_send;
		typedef std::function<void(io_base::i_connection* conn)> on_disconnected;
	}
}