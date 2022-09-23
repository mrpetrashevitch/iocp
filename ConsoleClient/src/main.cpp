#include "../../iocp_client/src/include.h"
#include <iostream>
#include <string>
#include <mutex>
#pragma warning(disable : 4996)

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

void fn_on_connected(web::io_base::i_connection* conn, const SOCKET& socket)
{
	std::lock_guard<std::mutex> lg(_mut_con);
	std::cout << current_date_time() << " on_connected: socket " << socket << std::endl;
}
void fn_on_recv(web::io_base::i_connection* conn, web::packet::packet_network* packet_nt)
{
	if (web::packet::get_packet_type(packet_nt) == web::packet::packet_type::packet_str)
	{
		auto packet = web::packet::packet_cast<web::packet::packet_str>(packet_nt);
		std::cout << current_date_time() << " " << packet->str << std::endl;
	}
}
void fn_on_send(web::io_base::i_connection* conn, web::packet::packet_network* packet)
{
	//closesocket(*(const SOCKET*)conn);
	/*if (packet->packet.type == web::web_base::packet_type::str)
	{
		std::cout << current_date_time() << " " << (const char*)packet->packet.body << std::endl;
	}*/
}
void fn_on_disconnected(web::io_base::i_connection* conn)
{
	std::cout << current_date_time() << " on_disconnected: server is not available" << std::endl;
}


void Clear()
{
#if defined _WIN32
	system("cls");
	//clrscr(); // including header file : conio.h
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
	system("clear");
	//std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
#elif defined (__APPLE__)
	system("clear");
#endif
}

#include <thread>
#include <deque>
#include <queue>

void thre(web::io_client::i_client* cl, web::packet::packet_str* p)
{
	for (size_t i = 0; i < 1000000; i++)
	{
		cl->send_packet_async(p);
	}
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

	if (str.size() > 0)
	{
		web::packet::packet_str p(str.c_str());
		for (size_t i = 0; i < 20; i++)
		{
			auto client = ld.create_fn(192, 168, 1, 5, 5001);
			client->set_on_connected(fn_on_connected);
			client->set_on_recv(fn_on_recv);
			client->set_on_send(fn_on_send);
			client->set_on_disconnected(fn_on_disconnected);
			client->run();
			std::thread th(thre, client, &p);
			th.detach();
		}
	}
	std::cin.get();
	return 0;
}