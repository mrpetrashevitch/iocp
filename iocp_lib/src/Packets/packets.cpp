#include "packets.h"
#include <string>

namespace web
{
	namespace packet
	{
		packet_type get_packet_type(packet_network* p)
		{
			return reinterpret_cast<packet*>(&p->data[0])->type;
		}

		packet_network* packet_str::get_buffer() { return (packet_network*)&_size; }
		packet_str::packet_str(const char* s)
		{
			if (!memcpy_s(str, sizeof(str), s, strlen(s) + sizeof(char)))
				_size += strlen(s) + sizeof(char);
		}

		packet_network* packet_data::get_buffer() { return (packet_network*)&_size; }
		packet_data::packet_data(const void* data, const int data_size)
		{
			if (!memcpy_s(&this->data, sizeof(this->data), data, data_size))
				_size += data_size;
		}
	}
}