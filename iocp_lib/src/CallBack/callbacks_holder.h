#pragma once
#include "callback.h"

namespace web
{
		namespace callback
		{
			class callbacks_holder
			{
			protected:
				on_accepted on_accepted;
				on_connected on_connected;
				on_recv on_recv;
				on_disconnected on_disconnected;
			public:
				void set_on_accepted(callback::on_accepted callback);
				void set_on_connected(callback::on_connected callback);
				void set_on_recv(callback::on_recv callback);
				void set_on_disconnected(callback::on_disconnected callback);
				virtual ~callbacks_holder() {}
			};
		}
}