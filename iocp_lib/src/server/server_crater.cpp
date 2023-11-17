#include "server_crater.h"
#include "server.h"

namespace web
{
	namespace io_server
	{
		std::shared_ptr<i_server> server_crater::create()
		{
			return std::make_shared<server>();
		}
	}
}