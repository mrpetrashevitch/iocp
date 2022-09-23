#include "connection.h"

#include <iphlpapi.h>
#include <IcmpAPI.h>
#pragma comment(lib, "Iphlpapi.lib")

namespace web
{
	namespace io_base
	{
		connection::connection(SOCKET socket, void* owner)
		{
			_owner = owner;
			ZeroMemory(&_addr, sizeof(_addr));
			_socket = socket;
			accept_overlapped.connection = this;
			connect_overlapped.connection = this;
			recv_overlapped.connection = this;
			send_overlapped.connection = this;
			_icmp_handle = IcmpCreateFile();
		}
		connection::~connection()
		{
			if (_icmp_handle)  IcmpCloseHandle(_icmp_handle);
			_icmp_handle = nullptr;
		}
		void* connection::get_owner()
		{
			return _owner;
		}
		SOCKET& connection::get_socket()
		{
			return _socket;
		}
		void connection::set_addr(const SOCKADDR_IN& addr)
		{
			_addr = addr;
		}
		SOCKADDR_IN& connection::get_addr()
		{
			return _addr;
		}
		int connection::get_ping()
		{
			if (!_icmp_handle) return -1;
           
			constexpr int payload_size = 1;
            byte payload[payload_size]{ 42 };

            constexpr DWORD reply_buf_size = sizeof(ICMP_ECHO_REPLY) + payload_size + 8;
            unsigned char reply_buf[reply_buf_size]{};

            DWORD reply_count = IcmpSendEcho(_icmp_handle, _addr.sin_addr.S_un.S_addr,
                payload, payload_size, NULL, reply_buf, reply_buf_size, 10000);

			if (reply_count == 0) return -1;

            const ICMP_ECHO_REPLY* r = (const ICMP_ECHO_REPLY*)reply_buf;
            return r->RoundTripTime;
		}
	}
}