#pragma once
#include "i_server.h"
#include <memory>

namespace web
{
	namespace io_server
	{
		struct server_crater
		{
			static std::shared_ptr<i_server> create(const char* addr, unsigned short port);
		};
	}
}
