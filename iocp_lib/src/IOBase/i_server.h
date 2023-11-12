#pragma once
#include "../CallBack/callback.h"

#include <memory> // shared_ptr

namespace web
{
	namespace io_server
	{
		struct i_server
		{
			virtual ~i_server() {};
			virtual void run() = 0;
			virtual void detach(io_base::i_connection* conn) = 0;

			virtual bool send(io_base::i_connection* conn, const void* data, int size) = 0;

			virtual void set_on_accepted(callback::on_accepted callback) = 0;
			virtual void set_on_recv(callback::on_recv callback) = 0;
			virtual void set_on_disconnected(callback::on_disconnected callback) = 0;
		};
	}
}