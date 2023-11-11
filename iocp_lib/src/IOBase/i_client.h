#pragma once
#include "../CallBack/i_callback.h"

#include <memory> // shared_ptr

namespace web
{
	namespace io_client
	{
		struct i_client
		{
			virtual ~i_client() {};
			virtual void run() = 0;
			virtual void detach() = 0;

			virtual bool send(const void* data, int size) = 0;

			virtual void set_on_connected(callback::on_connected callback) = 0;
			virtual void set_on_recv(callback::on_recv callback) = 0;
			virtual void set_on_disconnected(callback::on_disconnected callback) = 0;
		};
	}
}