#pragma once
#include "../defs.h"
//#pragma warning(disable : 4200) // byte_net packet[0];

#define WEB_BASE_PACKET_NETWORK_MAX_SIZE 512
#define WEB_BASE_PACKET_MAX_SIZE (WEB_BASE_PACKET_NETWORK_MAX_SIZE - sizeof(int_net))
#define WEB_BASE_PACKET_MIN_SIZE (sizeof(int_net) + 1)
#define WEB_BASE_PACKET_BODY_MAX_SIZE (WEB_BASE_PACKET_MAX_SIZE - sizeof(int_net))

namespace web
{
	namespace packet
	{
		struct packet_network
		{
			int_net size;
			byte_net data[WEB_BASE_PACKET_MAX_SIZE];
		};

		struct i_packet_network
		{
			virtual packet_network* get_buffer() = 0;
		};
	}
}