#pragma once
#include "winsock2.h" // WSABUF

#include <mutex>

namespace web
{
	namespace io_base
	{
		constexpr int web_buffer_send_min_size = 4096 * 128;

		class web_buffer_send
		{
		public:
			web_buffer_send(int size = web_buffer_send_min_size);
			void update_wsa_send();
			bool add_total_send(int len);
			bool add_data(const void* data, int size);
			bool empty();
			bool is_error();
			WSABUF* get_wsabuf();
			void lock();
			void unlock();

		private:
			bool m_is_error;
			unsigned char* m_buff;
			int m_buff_size;
			int m_total_send;
			int m_total_write;
			WSABUF m_wsa;
			std::mutex m_mut;
		};
	}
}