#pragma once
#include "i_connection.h"
#include "../WebBuffer/web_buffer_recv.h"
#include "../WebBuffer/web_buffer_send.h"


namespace web
{
	namespace io_base
	{
		class connection;

		enum class overlapped_type
		{
			nun,
			connect,
			disconnect,
			accept,
			recv,
			send,
		};

		struct overlapped_base
		{
			overlapped_base() : connection(nullptr), type(overlapped_type::nun) { memset(&overlapped, 0, sizeof(OVERLAPPED)); }
			OVERLAPPED overlapped;
			overlapped_type type;
			connection* connection;
		};

		struct overlapped_connect : public overlapped_base
		{
			overlapped_connect() { type = overlapped_type::connect; }
		};

		struct overlapped_disconnect : public overlapped_base
		{
			overlapped_disconnect() { type = overlapped_type::disconnect; }
		};

		struct overlapped_accept : public overlapped_base
		{
			overlapped_accept() { type = overlapped_type::accept; }
			byte buffer[(sizeof(sockaddr_in) + 16) * 2] {0};
		};

		struct overlapped_recv : public overlapped_base
		{
			overlapped_recv() { type = overlapped_type::recv; }
			web_buffer_recv buffer;
		};

		struct overlapped_send : public overlapped_base
		{
			overlapped_send() { type = overlapped_type::send; }
			web_buffer_send buffer;
		};

		class connection : public i_connection
		{
		public:
			overlapped_connect connect_overlapped;
			overlapped_disconnect disconnect_overlapped;
			overlapped_accept accept_overlapped;
			overlapped_recv recv_overlapped;
			overlapped_send send_overlapped;

			connection(SOCKET socket, void* owner);
			~connection();
			void* get_owner();
			SOCKET& get_socket() override;
			void set_addr(const SOCKADDR_IN& addr);
			SOCKADDR_IN& get_addr() override;

		private:
			void* m_owner;
			SOCKET m_socket;
			SOCKADDR_IN m_addr;
		};
	}
}