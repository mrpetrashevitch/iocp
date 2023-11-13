#include "../../iocp_client/src/include.h"
#include <iostream>
#include <string>
#include <mutex>
#pragma warning(disable : 4996)

#include <thread>
#include <deque>
#include <queue>

const std::string current_date_time()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

std::mutex _mut_con;

bool fn_on_connected(web::io_base::i_connection* conn)
{
	std::lock_guard<std::mutex> lg(_mut_con);
	std::cout << current_date_time() << " on_connected: id " << conn->get_id() << std::endl;

	// closesocket(conn->get_socket());
	return true;
}

int fn_on_recv(web::io_base::i_connection* conn, const void* data, int size)
{
	/*if (web::packet::get_packet_type(packet_nt) == web::packet::packet_type::packet_str)
	{
		auto packet = web::packet::packet_cast<web::packet::packet_str>(packet_nt);
		std::cout << current_date_time() << " " << packet->str << std::endl;
	}*/

	return size;
}

void fn_on_disconnected(web::io_base::i_connection* conn)
{
	std::cout << current_date_time() << " on_disconnected: server is not available" << std::endl;
}

void thre(web::io_client::i_client* cl, int threadid, const void* data, int size)
{
	ULONG64 total_send = 0;
	for (size_t i = 0; i < 1; i++)
	{
		if (!cl->send_async(data, size))
			std::cout << "[" << threadid << "]" << current_date_time() << " failed to send packet " << i << std::endl;
		else
			total_send += size;
	}
	std::cout << "[" << threadid << "]" << current_date_time() << " total send " << total_send << std::endl;
}

int main()
{
	/*std::deque<int> d;

	for (size_t i = 0; i < 1000000000; i++)
	{
		d.push_back(i);
	}*/

	const char* path = "web_client.dll";

	web::io_client::web_client_dll_loader ld(path);
	std::string str;
	std::getline(std::cin, str);

	std::vector<std::shared_ptr<web::io_client::i_client>> clients;

	if (str.size() > 0)
	{
		for (size_t i = 0; i < 1; i++)
		{
			std::shared_ptr<web::io_client::i_client> client = nullptr;
			ld.create_fn("127.0.0.1", 5001, client);
			if (!client)
			{
				std::cout << "Failed to create client" << std::endl;
				continue;
			}
			client->set_on_connected(fn_on_connected);
			client->set_on_recv(fn_on_recv);
			client->set_on_disconnected(fn_on_disconnected);
			client->run();
			std::thread th(thre, client.get(), i, str.c_str(), strlen(str.c_str()) + sizeof(char));
			th.detach();

			clients.push_back(client);
		}
	}
	std::cin.get();
	return 0;
}