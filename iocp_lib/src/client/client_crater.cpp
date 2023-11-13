#include "client_crater.h"
#include "client.h"

namespace web
{
	namespace io_client
	{
		std::shared_ptr<i_client> client_crater::create(const char* addr, unsigned short port)
		{
			std::shared_ptr<client> s = std::make_shared<client>();
			s->init(addr, port);
			return s;
		}
	}
}