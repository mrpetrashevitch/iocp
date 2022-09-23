#pragma once
#include "../defs.h"
#include "i_packet_network.h"

namespace web
{
	namespace packet
	{
		enum class packet_type : int_net
		{
			packet_str,
			packet_data,
		};

		struct packet
		{
			packet_type type;
			byte_net body[WEB_BASE_PACKET_BODY_MAX_SIZE];
		};

		packet_type get_packet_type(packet_network* p);

		template<typename T>
		T* packet_cast(packet_network* p)
		{
			return reinterpret_cast<T*>(((byte*)p)-sizeof(void*));
		}

		class packet_str : public i_packet_network
		{
			int_net _size = sizeof(packet_type);
			const packet_type _type = packet_type::packet_str;
		public:
			packet_network* get_buffer() override;
			packet_str(const char* str);

			char str[WEB_BASE_PACKET_BODY_MAX_SIZE];
		};

		class packet_data : protected i_packet_network
		{
			int_net _size = sizeof(packet_type);
			const packet_type _type = packet_type::packet_data;
		public:
			packet_network* get_buffer() override;
			packet_data(const void* data, const int data_size);

			byte data[WEB_BASE_PACKET_BODY_MAX_SIZE];
		};
	}
}
