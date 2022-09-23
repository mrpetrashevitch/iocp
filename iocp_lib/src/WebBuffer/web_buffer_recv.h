#pragma once
#include "../Packets/i_packet_network.h"
#include "winsock2.h" // WSABUF

namespace web
{
	namespace io_base
	{
		class web_buffer_recv
		{
			bool _is_error;
			byte* _buff;
			int _buff_size;
			int _total_recv;
			int _total_read;
			WSABUF _wsa;
		public:
			web_buffer_recv(int size = 0);
			bool move(int len);
			bool is_error();
			packet::packet_network* get_packet();
			WSABUF* get_wsabuf();
		};
	}
}
