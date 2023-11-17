#pragma once
#include "../callback/callback.h"

#include <memory>

namespace web
{
	namespace io_client
	{
		struct i_client
		{
			virtual ~i_client() {};
			virtual void run(const char* addr, unsigned short port, int thread_max) = 0;
			virtual void stop() = 0;

			virtual const std::shared_ptr<io_base::i_connection>& get_connection () = 0;

			virtual void set_on_connected(callback::on_connected callback) = 0;
			virtual void set_on_recv(callback::on_recv callback) = 0;
			virtual void set_on_disconnected(callback::on_disconnected callback) = 0;
		};
	}
}