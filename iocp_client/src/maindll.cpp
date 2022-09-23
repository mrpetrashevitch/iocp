#include "WebClient/client.h"

extern "C"
{
	__declspec(dllexport) web::io_client::client* create(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port)
	{
		web::io_client::client* web = new web::io_client::client();
		web->init(web->get_sockaddr(s_b1, s_b2, s_b3, s_b4, port));
		return web;
	}

	__declspec(dllexport) bool destroy(web::io_client::i_client* web)
	{
		if (!web) return false;
		delete web;
		return true;
	}
}