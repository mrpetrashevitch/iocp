#pragma once
#include "../defs.h"
#include "../Connection/connection.h"
#include "../CallBack/callback.h"

#include <Windows.h>


namespace web
{
	namespace io_base
	{
		class base : public web::callback::callbacks_holder
		{
		protected:
			HANDLE _iocp = nullptr;

			bool _wsa_init();

			void _recv_async(connection* conn);
			void _send_async(connection* conn);
			void _send_packet_async(connection* conn, const web_send_task& task);

			void _worker();
		public:
			static SOCKADDR_IN get_sockaddr(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port);
			static SOCKADDR_IN get_sockaddr(const char* addres, ushort port);
			static SOCKADDR_IN get_sockaddr(uint addres, ushort port);
		};
	}
}
