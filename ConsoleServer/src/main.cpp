#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

#include "../../iocp_server/src/include.h"
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
std::atomic<int> total_conn = 0;

void fn_on_accepted(web::io_base::i_connection* conn, const SOCKET& socket)
{
	total_conn++;
	/*{
		std::lock_guard<std::mutex> lg(mut_cout);
		std::cout << current_date_time() << " on_accepted: socket " << socket << std::endl;
	}*/

	std::string str("[user");
	str += std::to_string(conn->get_socket());
	str += "] has been connected";

	std::shared_ptr<web::packet::packet_str> p(std::make_unique<web::packet::packet_str>(str.c_str()));

	{
		std::lock_guard<std::recursive_mutex> lg(mut_conn);
		_connestions.push_back(conn);
	}

	/*web::io_server::i_server* server = reinterpret_cast<web::io_server::i_server*>(conn->get_owner());
	{
		std::lock_guard<std::recursive_mutex> lg(mut_conn);
		for (auto& i : _connestions)
			if (i->get_socket() != socket)
				server->send_packet_async(i, p);
	}*/
}

void fn_on_recv(web::io_base::i_connection* conn, web::packet::packet_network* packet_nt)
{
	total_pack++;
	//SOCKET sock = conn->get_socket();
	/*{
		std::lock_guard<std::mutex> lg(mut_cout);
		std::cout << current_date_time() << " on_recved from " << sock << ": size " << packet->size << std::endl;
	}*/

	/*if (web::packet::get_packet_type(packet_nt) == web::packet::packet_type::packet_str)
	{
		auto packet = web::packet::packet_cast<web::packet::packet_str>(packet_nt);
		std::cout << current_date_time() << " " << packet->str << std::endl;
	}*/

	/*if (packet->packet.type == web::web_base::packet_type::str)
	{
		std::string str("[user");
		str += std::to_string(sock);
		str += "]: ";
		str += (const char*)packet->packet.body;

		std::shared_ptr<web::web_base::packet_str> p(std::make_shared<web::web_base::packet_str>(str.c_str()));
		web::web_server::i_server* server = reinterpret_cast<web::web_server::i_server*>(conn->get_owner());
		{
			std::lock_guard<std::recursive_mutex> lg(mut_conn);
			for (auto& i : _connestions)
				server->send_packet_async(i, p);
		}
	}*/
}
void fn_on_send(web::io_base::i_connection* conn, web::packet::packet_network* packet)
{
	/*SOCKET sock = conn->get_socket();

	{
		std::lock_guard<std::mutex> lg(mut_cout);
		std::cout << current_date_time() << " on_sended to " << sock << ": size " << packet->size << std::endl;
	}*/
}
void fn_on_disconnected(web::io_base::i_connection* conn)
{
	total_conn--;
	/*{
		std::lock_guard<std::mutex> lg(mut_cout);
		std::cout << current_date_time() << " on_disconnected: socket " << conn->get_socket() << std::endl;
	}*/

	std::string str("[user");
	str += std::to_string(conn->get_socket());
	str += "] ";
	str += " has been disconnected";

	std::shared_ptr<web::packet::packet_str> p(std::make_shared<web::packet::packet_str>(str.c_str()));
	web::io_server::i_server* server = reinterpret_cast<web::io_server::i_server*>(conn->get_owner());

	{
		std::lock_guard<std::recursive_mutex> lg(mut_conn);
		auto item = std::find_if(_connestions.begin(), _connestions.end(), [&conn](web::io_base::i_connection* c) { return c->get_socket() == conn->get_socket(); });
		if (item != _connestions.end())
			_connestions.erase(item);

		/*for (auto& i : _connestions)
			if (i->get_socket() != conn->get_socket())
			{
				server->send_packet_async(i, p);
			}*/
	}
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


int main()
{
	const char* path = "web_server.dll";

	web::io_server::web_server_dll_loader ld(path);

	web::io_server::i_server* server = ld.create_fn(192, 168, 1, 5, 5001);

	server->set_on_accepted(fn_on_accepted);
	server->set_on_recv(fn_on_recv);
	server->set_on_send(fn_on_send);
	server->set_on_disconnected(fn_on_disconnected);

	server->run();

	std::vector<uint> vs(20);
	int last = 0;

	for (;;)
	{
		printf("Total connection: %d, total packets: %d (%d)                     \r", total_conn.load(), total_pack.load(), total_pack.load() - last);
		last = total_pack;
		Sleep(1000);
	}

	std::cin.get();
	return 0;
}