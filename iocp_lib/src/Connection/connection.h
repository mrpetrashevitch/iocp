#pragma once
#include "i_connection.h"
#include "../WebBuffer/web_buffer_recv.h"
#include "../WebBuffer/web_buffer_send.h"

#include <memory>
#include <atomic>

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
			overlapped_base() : conn(nullptr), type(overlapped_type::nun) { memset(&overlapped, 0, sizeof(OVERLAPPED)); }
			OVERLAPPED overlapped;
			overlapped_type type;
			connection* conn;
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

			connection(SOCKET socket, int id);
			~connection();
			SOCKET& get_socket();
			void set_addr(const SOCKADDR_IN& addr);
			bool _recv_async();
			bool _send_async();
			void self_lock(std::shared_ptr<connection> conn);
			void self_unlock();
			std::shared_ptr<connection> get();

			int get_id() override;
			const std::string& get_addr() override;
			bool send_async(const void* data, int size) override;
			bool disconnect_async() override;

		private:
			int m_id;
			SOCKET m_socket;
			SOCKADDR_IN m_addr;
			std::string m_addr_str;
			std::atomic<std::shared_ptr<connection>> m_self;
		};
	}
}