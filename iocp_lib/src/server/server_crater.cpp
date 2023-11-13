#include "server_crater.h"
#include "server.h"

namespace web
{
	namespace io_server
	{
		std::shared_ptr<i_server> server_crater::create(const char* addr, unsigned short port)
		{
			std::shared_ptr<server> s = std::make_shared<server>();
			s->init(addr, port);
			return s;
		}
	}
}