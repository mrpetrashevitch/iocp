#include "../../iocp_client/src/include.h"

#include "../../packet.h"

#include <iostream>
#include <string>
#include <mutex>
#pragma warning(disable : 4996)

#include <atomic>
#include <thread>
#include <deque>
#include <queue>
#include <unordered_map>

int random(int min, int max) //range : [min, max]
{
	static bool first = true;
	if (first)
	{
		srand(time(NULL)); //seeding for the first time only!
		first = false;
	}
	return min + rand() % ((max + 1) - min);
}

const std::string current_date_time()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

std::atomic<int> a_total_run;
std::atomic<int> a_total_com;


std::mutex _mut_con;

std::mutex mut;
std::unordered_map<web::io_base::i_connection*, std::shared_ptr<unsigned long long>> map;

bool fn_on_connected(web::io_base::i_connection* conn)
{
	{
		std::lock_guard gl(mut);
		map.insert(std::pair(conn, std::make_shared<unsigned long long>(0)));
	}

	std::lock_guard<std::mutex> lg(_mut_con);
	std::cout << current_date_time() << " on_connected: id " << conn->get_id() << std::endl;

	// closesocket(conn->get_socket());
	return true;
}

int fn_on_recv(web::io_base::i_connection* conn, const void* data, int size)
{
	if (size < sizeof(packet_header))
		return 0;

	packet_header* ph = (packet_header*)data;

	if (ph->size > size)
		return 0;

	switch (ph->type)
	{
	case packet_type::get:
	{
		if (ph->size - sizeof(packet_header) != sizeof(unsigned long long))
		{
			break;
		}
		auto hash_server = *((unsigned long long*) & ((packet_header*)data)[1]);

		std::shared_ptr<unsigned long long> hash = nullptr;
		{
			std::lock_guard gl(mut);
			hash = map.at(conn);
		}

		if (hash_server == *hash)
		{
			a_total_com++;
			std::cout << current_date_time() << conn << " completed" << std::endl;
		}
		conn->disconnect_async();
	}
	break;
	default:
		break;
	}

	return ph->size;
}

void fn_on_disconnected(web::io_base::i_connection* conn)
{
	{
		std::lock_guard gl(mut);
		map.erase(conn);
	}
	std::cout << current_date_time() << " on_disconnected" << std::endl;
}

void thre(web::io_client::i_client* cl, int threadid, const void* data, int size)
{
	a_total_run++;
	cl->run("127.0.0.1", 5001, 1);

	size = random(1, size);

	bool connected = false;
	while (!connected)
	{
		{
			std::lock_guard gl(mut);
			connected = map.contains(cl->get_connection().get());
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(random(1, 100)));
	}

	auto conn = cl->get_connection();
	if (!conn)
	{
		std::cout << current_date_time() << " connection empty" << std::endl;
		return;
	}

	packet_header p_reset{
			.size = sizeof(packet_header),
			.type = packet_type::reset
	};

	packet_header p_add{
			.size = (int)sizeof(packet_header) + size,
			.type = packet_type::add
	};

	packet_header p_get{
			.size = sizeof(packet_header),
			.type = packet_type::get
	};

	conn->send_async(&p_reset, sizeof(p_reset));

	std::shared_ptr<unsigned long long> hash = nullptr;
	{
		std::lock_guard gl(mut);
		hash = map.at(cl->get_connection().get());
	}

	for (size_t i = 0; i < random(10000, 100000); i++)
	{
		conn->send_async(&p_add, sizeof(p_add));
		for (size_t i = 0; i < size; i++)
		{
			(*hash) += ((char*)data)[i];
		}
		conn->send_async(data, size);
		std::this_thread::sleep_for(std::chrono::milliseconds(random(1, 10)));
	}
	conn->send_async(&p_get, sizeof(p_get));
}

int main()
{
	const char* path = "web_client.dll";

	web::io_client::web_client_dll_loader ld(path);
	std::string str;
	std::getline(std::cin, str);

	std::vector<std::shared_ptr<web::io_client::i_client>> clients;

	if (str.size() > 0)
	{
		for (size_t i = 0; i < 10; i++)
		{
			std::shared_ptr<web::io_client::i_client> client = nullptr;
			ld.create_fn(client);
			if (!client)
			{
				std::cout << "Failed to create client" << std::endl;
				continue;
			}
			client->set_on_connected(fn_on_connected);
			client->set_on_recv(fn_on_recv);
			client->set_on_disconnected(fn_on_disconnected);
			std::thread th(thre, client.get(), i, str.c_str(), strlen(str.c_str()) + sizeof(char));
			th.detach();

			clients.push_back(client);

			//std::this_thread::sleep_for(std::chrono::milliseconds(random(1, 10)));
		}
	}
	std::cin.get();
	std::cout << a_total_run << " " << a_total_com << std::endl;
	std::cin.get();
	return 0;
}