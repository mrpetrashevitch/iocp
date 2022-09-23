#include "WebServer/server.h"

extern "C"
{
	__declspec(dllexport) web::io_server::i_server* create(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port)
	{
		web::io_server::server* web = new web::io_server::server();
		web->init(web->get_sockaddr(s_b1, s_b2, s_b3, s_b4, port));
		return web;
	}

	__declspec(dllexport) bool destroy(web::io_server::i_server* web)
	{
		if (!web) return false;
		delete web;
		return true;
	}
}