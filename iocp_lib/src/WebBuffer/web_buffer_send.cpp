#include "web_buffer_send.h"
#pragma warning(disable : 4996)

namespace web
{
	namespace io_base
	{
		web_buffer_send::web_buffer_send(int size) : m_is_error(false), m_total_send(0), m_total_write(0)
		{
			if (size < web_buffer_send_min_size)
				size = web_buffer_send_min_size;

			m_buff_size = size;
			m_buff = new unsigned char[m_buff_size];
			m_wsa = { (unsigned long)(m_total_write - m_total_send), (CHAR*)&m_buff[m_total_send] };
		}

		void web_buffer_send::update_wsa_send()
		{
			if (m_total_write < m_total_send)
			{
				m_wsa = { (unsigned long)(m_buff_size - m_total_send), (CHAR*)&m_buff[m_total_send] };
			}
			else
			{
				m_wsa = { (unsigned long)(m_total_write - m_total_send), (CHAR*)&m_buff[m_total_send] };
			}
		}

		bool web_buffer_send::add_total_send(int len)
		{
			if (m_total_write == m_total_send)
			{
				return false;
			}

			if (m_total_write < m_total_send)
			{
				if (len + m_total_send > m_buff_size)
				{
					return false;
				}

				m_total_send += len;

				if (m_total_send == m_buff_size)
				{
					// buffer end, return to start
					m_total_send = 0;
				}
			}
			else
			{
				if (len + m_total_send > m_total_write)
				{
					return false;
				}

				m_total_send += len;

				if (m_total_send == m_total_write)
				{
					// reset buffer, buffer is empty
					m_total_send = 0;
					m_total_write = 0;
				}
			}

			update_wsa_send();

			return true;
		}

		bool web_buffer_send::add_data(const void* data, int size)
		{
			if (size < 1)
				return false;

			bool empty = this->empty();

			if (m_total_write < m_total_send)
			{
				if (size + m_total_write >= m_total_send)
				{
					// buff overflow
					return false;
				}

				memcpy(&m_buff[m_total_write], data, size);
				m_total_write += size;
			}
			else
			{
				if (size >= (m_buff_size - m_total_write) + m_total_send)
				{
					// buff overflow
					return false;
				}

				int space_end = m_buff_size - m_total_write;
				
				if (size > space_end)
				{
					memcpy(&m_buff[m_total_write], data, space_end);
					memcpy(&m_buff[0], &(reinterpret_cast<const char*>(data)[space_end]), size - space_end);
					m_total_write = size - space_end;
				}
				else
				{
					memcpy(&m_buff[m_total_write], data, size);
					m_total_write += size;
				}

				if (m_total_write == m_buff_size)
				{
					// buffer end, return to start
					m_total_write = 0;
				}
			}

			if (empty)
				update_wsa_send();

			return true;
		}

		bool web_buffer_send::empty()
		{
			return m_wsa.len == 0;
		}

		bool web_buffer_send::is_error()
		{
			return m_is_error;
		}

		WSABUF* web_buffer_send::get_wsabuf()
		{
			return &m_wsa;
		}

		void web_buffer_send::lock()
		{
			m_mut.lock();
		}

		void web_buffer_send::unlock()
		{
			m_mut.unlock();
		}

		//web_send_task::web_send_task(packet::i_packet_network* packet) : _is_ready(false), _packet(packet), _total_recv(0)
		//{
		//	_need = packet->get_buffer()->size + sizeof(packet->get_buffer()->size);
		//	_wsa = { (unsigned long)(_need),(CHAR*)_packet->get_buffer() };
		//}

		//web_send_task::web_send_task(const std::shared_ptr<packet::i_packet_network>& ptr) : web_send_task(ptr.get())
		//{
		//	_ptr = ptr;
		//}

		//bool web_send_task::_move(int len)
		//{
		//	if (_is_ready) 
		//		return true;
		//	_total_recv += len;
		//	if (_need == _total_recv) 
		//		_is_ready = true;
		//	else 
		//		_wsa = { (unsigned long)(_need - _total_recv), (CHAR*)_packet + _total_recv };
		//	return _is_ready;
		//}

		//bool web_buffer_send::empty()
		//{
		//	return !_front;
		//}

		//void web_buffer_send::push(const web_send_task& task)
		//{
		//	_q.push_back(task);
		//	_front = &_q.front();
		//}

		//web_send_task web_buffer_send::pop()
		//{
		//	web_send_task ret = _q.front();
		//	_q.pop_front();

		//	/*auto allock_item = _q.max_size() / sizeof(web_send_task);
		//	if (allock_item > 1000 && _q.size()*10 < allock_item)
		//	{
		//		_q.shrink_to_fit();
		//	}*/

		//	if (_q.empty()) 
		//		_front = nullptr;
		//	else 
		//		_front = &_q.front();
		//	return ret;
		//}

		//bool web_buffer_send::move(int len)
		//{
		//	if (!_front)
		//		throw;
		//	return _front->_move(len);
		//}

		//WSABUF* web_buffer_send::get_wsabuf()
		//{
		//	if (!_front)
		//		throw;
		//	return &_front->_wsa;
		//}

		//packet::packet_network* web_buffer_send::get_packet()
		//{
		//	if (!_front)
		//		throw;
		//	return _front->_packet->get_buffer();
		//}
	}
}