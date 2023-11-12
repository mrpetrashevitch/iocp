#include "callbacks_holder.h"

namespace web
{
	void callback::callbacks_holder::set_on_accepted(callback::on_accepted callback) { on_accepted = callback; }
	void callback::callbacks_holder::set_on_connected(callback::on_connected callback) { on_connected = callback; }
	void callback::callbacks_holder::set_on_recv(callback::on_recv callback) { on_recv = callback; }
	void callback::callbacks_holder::set_on_disconnected(callback::on_disconnected callback) { on_disconnected = callback; }
}
