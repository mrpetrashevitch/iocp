#pragma once
#include "winsock2.h" // WSABUF

namespace web
{
	namespace io_base
	{

		constexpr int web_buffer_recv_min_size = 512;

		class web_buffer_recv
		{
		public:
			web_buffer_recv(int size = web_buffer_recv_min_size);
			bool add_total_recv(int len);
			bool add_total_read(int len);
			const void* get_curr_buffer();
			int get_cerr_buffer_size();
			void fit();
			bool is_error();
			WSABUF* get_wsabuf();
		private:
			bool m_is_error;
			byte* m_buff;
			int m_buff_size;
			int m_total_recv;
			int m_total_read;
			WSABUF m_wsa;
		};
	}
}
