#pragma once
#include "../defs.h"
#include "winsock2.h" // SOCKET, SOCKADDR_IN

namespace web
{
	namespace io_base
	{
		struct i_connection
		{
			virtual void* get_owner() = 0;
			virtual SOCKET& get_socket() = 0;
			virtual SOCKADDR_IN& get_addr() = 0;
			virtual int get_ping() = 0;
		};
	}
}