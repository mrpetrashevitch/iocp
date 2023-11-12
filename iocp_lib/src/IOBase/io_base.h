#pragma once
#include "../defs.h"
#include "../Connection/connection.h"
#include "../CallBack/callbacks_holder.h"

#include <Windows.h>


namespace web
{
	namespace io_base
	{
		enum class completion_key {
			io,
			shutdown
		};

		class base : public web::callback::callbacks_holder
		{
		public:
			static SOCKADDR_IN get_sockaddr(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port);
			static SOCKADDR_IN get_sockaddr(const char* addres, ushort port);
			static SOCKADDR_IN get_sockaddr(uint addres, ushort port);

		protected:
			HANDLE _iocp = nullptr;

			bool _wsa_init();

			void accept_handler(connection* conn);
			void connect_handler(connection* conn);
			void recv_handler(connection* conn, DWORD bytes_transferred);
			void send_handler(connection* conn, DWORD bytes_transferred);
			void disconnect_handler(connection* conn);

			bool _recv_async(connection* conn);
			bool _send_async(connection* conn);
			bool _send(connection* conn, const void* data, int size);

			void _worker();
		
		};
	}
}
