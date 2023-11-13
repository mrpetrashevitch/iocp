#include "../../iocp_lib/src/client/client_crater.h"

extern "C"
{
	__declspec(dllexport) void create(const char* addr, unsigned short port, std::shared_ptr<web::io_client::i_client>& out_client)
	{
		out_client = web::io_client::client_crater::create(addr, port);
	}
}