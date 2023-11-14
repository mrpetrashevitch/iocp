#include "../../iocp_lib/src/server/server_crater.h"

extern "C"
{
	__declspec(dllexport) void create(std::shared_ptr<web::io_server::i_server>& out_server)
	{
		out_server = web::io_server::server_crater::create();
	}
}