#include "web_buffer_recv.h"

namespace web
{
	namespace io_base
	{
		web_buffer_recv::web_buffer_recv(int size) : m_is_error(false), m_total_recv(0), m_total_read(0)
		{
			if (size < web_buffer_recv_min_size)
				size = web_buffer_recv_min_size;

			m_buff_size = size;
			m_buff = new byte[m_buff_size];
			m_wsa = { (unsigned long)(m_buff_size - m_total_recv), (CHAR*)&m_buff[m_total_recv] };
		}

		bool web_buffer_recv::add_total_recv(int len)
		{
			if (len + m_total_recv > m_buff_size)
			{
				m_is_error = true;
				return false;
			}
			m_total_recv += len;
			return true;
		}

		bool web_buffer_recv::add_total_read(int len)
		{
			if (len + m_total_read > m_total_recv)
			{
				m_is_error = true;
				return false;
			}
			m_total_read += len;
			return true;
		}

		const void* web_buffer_recv::get_curr_buffer()
		{
			return reinterpret_cast<const void*>(&m_buff[m_total_read]);
		}

		int web_buffer_recv::get_cerr_buffer_size()
		{
			return m_total_recv - m_total_read;
		}

		void web_buffer_recv::fit()
		{
			if (m_buff_size == m_total_recv)
			{
				int size_copy = m_buff_size - m_total_read;
				if (size_copy > 0) 
					memcpy(m_buff, &m_buff[m_total_read], size_copy);
				m_total_recv = size_copy;
				m_total_read = 0;
			}
		}

		bool web_buffer_recv::is_error()
		{
			return m_is_error;
		}

		WSABUF* web_buffer_recv::get_wsabuf()
		{
			m_wsa = { (unsigned long)(m_buff_size - m_total_recv), (CHAR*)&m_buff[m_total_recv] };
			return &m_wsa;
		}
	}
}