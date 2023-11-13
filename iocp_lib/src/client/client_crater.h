#pragma once
#include "i_client.h"
#include <memory>

namespace web
{
	namespace io_client
	{
		struct client_crater
		{
			static std::shared_ptr<i_client> create(const char* addr, unsigned short port);
		};
	}
}
