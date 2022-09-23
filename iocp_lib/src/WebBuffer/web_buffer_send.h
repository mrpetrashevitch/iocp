#pragma once
#include "../Packets/i_packet_network.h"
#include "../Lock/lockers.h"
#include "winsock2.h" // WSABUF
#include <deque>


namespace web
{
	namespace io_base
	{
		class web_buffer_send;

		class web_send_task
		{
			bool _is_ready;
			packet::i_packet_network* _packet;
			int _need;
			int _total_recv;
			WSABUF _wsa;
			std::shared_ptr<web::packet::i_packet_network> _ptr;
			bool _move(int len);
		public:
			web_send_task(web::packet::i_packet_network* packet);
			web_send_task(const std::shared_ptr<packet::i_packet_network>& ptr);
			friend web_buffer_send;
		};

		class web_buffer_send
		{
			lockers::spin_lock _mut;
			std::deque<web_send_task> _q;
			web_send_task* _front = nullptr;
		public:
			bool empty();
			void push(const web_send_task& task);
			web_send_task pop();

			bool move(int len);
			WSABUF* get_wsabuf();
			packet::packet_network* get_packet();

			void lock()
			{
				_mut.lock();
			}
			void unlock()
			{
				_mut.unlock();
			}
		};
	}
}