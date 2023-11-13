#pragma once
#include "../connection/connection.h"
#include "../callback/callbacks_holder.h"

#include <Windows.h>

namespace web
{
	namespace io_base
	{
		enum class completion_key
		{
			io,
			shutdown
		};

		class base : public web::callback::callbacks_holder
		{
		protected:
			HANDLE m_iocp = nullptr;

		protected:
			bool _wsa_init();
			bool _accept_handler(connection* conn);
			bool _connect_handler(connection* conn);
			bool _recv_handler(connection* conn, DWORD bytes_transferred);
			bool _send_handler(connection* conn, DWORD bytes_transferred);
			bool _disconnect_handler(connection* conn);
			void _worker();
		};
	}
}
