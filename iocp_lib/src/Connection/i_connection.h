#pragma once
#include "../defs.h"
//#include "winsock2.h" // SOCKET, SOCKADDR_IN
#include <string>

namespace web
{
	namespace io_base
	{
		struct i_connection
		{
			virtual int get_id() = 0;
			virtual const std::string& get_addr() = 0;
			virtual bool send_async(const void* data, int size) = 0;
			virtual bool disconnect_async() = 0;
		};
	}
}