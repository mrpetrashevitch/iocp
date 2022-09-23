#include "web_buffer_send.h"
#pragma warning(disable : 4996)

namespace web
{
	namespace io_base
	{
		web_send_task::web_send_task(packet::i_packet_network* packet) : _is_ready(false), _packet(packet), _total_recv(0)
		{
			_need = packet->get_buffer()->size + sizeof(packet->get_buffer()->size);
			_wsa = { (unsigned long)(_need),(CHAR*)_packet->get_buffer() };
		}
		web_send_task::web_send_task(const std::shared_ptr<packet::i_packet_network>& ptr) : web_send_task(ptr.get())
		{
			_ptr = ptr;
		}
		bool web_send_task::_move(int len)
		{
			if (_is_ready) return true;
			_total_recv += len;
			if (_need == _total_recv) _is_ready = true;
			else _wsa = { (unsigned long)(_need - _total_recv),(CHAR*)_packet + _total_recv };
			return _is_ready;
		}

		bool web_buffer_send::empty()
		{
			return !_front;
		}
		void web_buffer_send::push(const web_send_task& task)
		{
			_q.push_back(task);
			_front = &_q.front();
		}
		web_send_task web_buffer_send::pop()
		{
			web_send_task ret = _q.front();
			_q.pop_front();

			/*auto allock_item = _q.max_size() / sizeof(web_send_task);
			if (allock_item > 1000 && _q.size()*10 < allock_item)
			{
				_q.shrink_to_fit();
			}*/

			if (_q.empty()) _front = nullptr;
			else _front = &_q.front();
			return ret;
		}
		bool web_buffer_send::move(int len)
		{
			if (!_front)
				throw;
			return _front->_move(len);
		}
		WSABUF* web_buffer_send::get_wsabuf()
		{
			if (!_front)
				throw;
			return &_front->_wsa;
		}
		packet::packet_network* web_buffer_send::get_packet()
		{
			if (!_front)
				throw;
			return _front->_packet->get_buffer();
		}
	}
}