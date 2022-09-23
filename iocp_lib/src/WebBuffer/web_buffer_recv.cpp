#include "web_buffer_recv.h"

namespace web
{
	namespace io_base
	{
		web_buffer_recv::web_buffer_recv(int size) : _is_error(false), _total_recv(0), _total_read(0)
		{
			if (size < WEB_BASE_PACKET_NETWORK_MAX_SIZE * 2) size = WEB_BASE_PACKET_NETWORK_MAX_SIZE * 2;

			_buff_size = size;
			_buff = new byte[_buff_size];
			_wsa = { (unsigned long)(_buff_size - _total_recv),(CHAR*)&_buff[_total_recv] };
		}
		bool web_buffer_recv::move(int len)
		{
			if (len + _total_recv > _buff_size)
				return false;
			_total_recv += len;
			return true;
		}
		packet::packet_network* web_buffer_recv::get_packet()
		{
			int read_can = _total_recv - _total_read;

			if (read_can < sizeof(int_net))
			{
				if (_buff_size == _total_recv)
				{
					int size_copy = _buff_size - _total_read;
					if (size_copy) memcpy(_buff, &_buff[_total_read], size_copy);
					_total_recv = size_copy;
					_total_read = 0;
				}
				return nullptr;
			}

			int_net size_p = *(int_net*)&_buff[_total_read];
			if (size_p > WEB_BASE_PACKET_MAX_SIZE) { _is_error = true;  return nullptr; }
			read_can -= sizeof(int_net);

			if (size_p > read_can)
			{
				if (_buff_size == _total_recv)
				{
					int size_copy = _buff_size - _total_read;
					if (size_copy) memcpy(_buff, &_buff[_total_read], size_copy);
					_total_recv = size_copy;
					_total_read = 0;
				}
				return nullptr;
			}

			packet::packet_network* packet = (packet::packet_network*)&_buff[_total_read];
			_total_read += sizeof(int_net);
			_total_read += size_p;
			return packet;
		}

		bool web_buffer_recv::is_error()
		{
			return _is_error;
		}

		WSABUF* web_buffer_recv::get_wsabuf()
		{
			_wsa = { (unsigned long)(_buff_size - _total_recv),(CHAR*)&_buff[_total_recv] };
			return &_wsa;
		}
	}
}