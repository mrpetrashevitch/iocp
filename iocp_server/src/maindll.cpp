#include "../../iocp_lib/src/server/server_crater.h"

extern "C"
{
	__declspec(dllexport) void create(const char* addr, unsigned short port, std::shared_ptr<web::io_server::i_server>& out_server)
	{
		out_server = web::io_server::server_crater::create(addr, port);
	}
}