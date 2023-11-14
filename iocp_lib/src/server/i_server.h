#pragma once
#include "../callback/callback.h"

namespace web
{
	namespace io_server
	{
		struct i_server
		{
			virtual ~i_server() {};
			virtual bool run(const char* addr, unsigned short port, int thread_max, int connection_max) = 0;
			virtual void stop() = 0;
			virtual void set_on_accepted(callback::on_accepted callback) = 0;
			virtual void set_on_recv(callback::on_recv callback) = 0;
			virtual void set_on_disconnected(callback::on_disconnected callback) = 0;
		};
	}
}