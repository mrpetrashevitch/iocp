#include "client_crater.h"
#include "client.h"

namespace web
{
	namespace io_client
	{
		std::shared_ptr<i_client> client_crater::create()
		{
			return std::make_shared<client>();
		}
	}
}