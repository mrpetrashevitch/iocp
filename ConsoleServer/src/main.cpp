#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <unordered_map>

#include "../../iocp_server/src/include.h"
#include "../../packet.h"
#pragma warning(disable : 4996)

std::mutex mut_cout;
std::recursive_mutex mut_conn;

std::string current_date_time()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

std::vector<web::io_base::i_connection*> _connestions;
std::atomic<int> total_pack = 0;
std::atomic<unsigned long long> total_size = 0;
std::atomic<int> total_conn = 0;

std::mutex mut;
std::unordered_map<int, std::shared_ptr<unsigned long long>> map;

void fn_on_accepted(web::io_base::i_connection* conn)
{
	{
		std::lock_guard gl(mut);
		map.insert(std::pair(conn->get_id(), std::make_shared<unsigned long long>(0)));
	}

	total_conn++;

	std::string str("[user");
	str += std::to_string(conn->get_id());
	str += "] has been connected";

	{
		std::lock_guard<std::recursive_mutex> lg(mut_conn);
		_connestions.push_back(conn);
	}
}

int fn_on_recv(web::io_base::i_connection* conn, const void* data, int size)
{
	if (size < sizeof(packet_header))
		return 0;

	packet_header* ph = (packet_header*)data;

	if (ph->size > size)
		return 0;

	std::shared_ptr<unsigned long long> hash = nullptr;
	{
		std::lock_guard gl(mut);
		hash = map.at(conn->get_id());
	}

	if (!hash)
	{
		printf("hash is empty!!!!!!!");
	}

	switch (ph->type)
	{
	case packet_type::reset:
		(*hash) = 0;
		break;
	case packet_type::add:
		for (size_t i = sizeof(packet_header); i < ph->size; i++)
		{
			(*hash) += ((char*)data)[i];
		}
		break;
	case packet_type::sub:
		for (size_t i = sizeof(packet_header); i < ph->size; i++)
		{
			(*hash) -= ((char*)data)[i];
		}
		break;
	case packet_type::get:
	{
		packet_header p{
			.size = sizeof(packet_header) + sizeof((*hash)),
			.type = packet_type::get
		};
		conn->send_async(&p, sizeof(p));
		conn->send_async(hash.get(), sizeof(*hash));
	}
	break;
	default:
		break;
	}

	//server->detach(conn);
	//auto s = total_size.load();

	//conn->send_async(&s, sizeof(s));
	//conn->close();
	//Sleep(1000);
	//conn->disconnect_async();

	total_pack++;
	total_size += ph->size;
	return ph->size;
}

void fn_on_disconnected(web::io_base::i_connection* conn)
{
	{
		std::lock_guard gl(mut);
		map.erase(conn->get_id());
	}

	total_conn--;

	std::string str("[user");
	str += std::to_string(conn->get_id());
	str += "] ";
	str += " has been disconnected";

	{
		std::lock_guard<std::recursive_mutex> lg(mut_conn);
		auto item = std::find_if(_connestions.begin(), _connestions.end(), [&conn](web::io_base::i_connection* c) { return c->get_id() == conn->get_id(); });
		if (item != _connestions.end())
			_connestions.erase(item);
	}
}

int main()
{
	const char* path = "web_server.dll";

	web::io_server::web_server_dll_loader ld(path);

	std::shared_ptr<web::io_server::i_server> server = nullptr;
	ld.create_fn(server);

	server->set_on_accepted(fn_on_accepted);
	server->set_on_recv(fn_on_recv);
	server->set_on_disconnected(fn_on_disconnected);

	server->run("127.0.0.1", 5001, 0, 0);

	int last = 0;

	for (;;)
	{
		printf("Total connection: %d, total packets: %d (%d), total size %llu                     \r",
			total_conn.load(), total_pack.load(), total_pack.load() - last, total_size.load());
		last = total_pack;
		Sleep(1000);
	}
	std::cin.get();
	server->stop();

	std::cin.get();
	return 0;
}