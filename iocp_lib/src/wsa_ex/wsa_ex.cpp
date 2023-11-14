#include "wsa_ex.h"

#include <Ws2tcpip.h>
#include <mswsock.h>

namespace web
{
	namespace wsa
	{
		bool ConnectEx(_In_ SOCKET s,
			_In_reads_bytes_(namelen) const struct sockaddr FAR* name,
			_In_ int namelen,
			_In_reads_bytes_opt_(dwSendDataLength) PVOID lpSendBuffer,
			_In_ DWORD dwSendDataLength,
			_Out_ LPDWORD lpdwBytesSent,
			_Inout_ LPOVERLAPPED lpOverlapped)
		{
			static LPFN_CONNECTEX connectx_func = nullptr;
			if (!connectx_func)
			{
				GUID acceptex_guid = WSAID_CONNECTEX;
				DWORD bytes_returned;
				if (WSAIoctl(s,
					SIO_GET_EXTENSION_FUNCTION_POINTER,
					&acceptex_guid,
					sizeof(acceptex_guid),
					&connectx_func,
					sizeof(connectx_func),
					&bytes_returned, NULL, NULL))
					return false;
			}

			const int result = connectx_func
			(
				s,
				name,
				namelen,
				lpSendBuffer,
				dwSendDataLength,
				lpdwBytesSent,
				lpOverlapped
			);
			return result == TRUE || WSAGetLastError() == WSA_IO_PENDING;
		}

		bool AcceptEx(
			_In_ SOCKET sListenSocket,
			_In_ SOCKET sAcceptSocket,
			_Out_writes_bytes_(dwReceiveDataLength + dwLocalAddressLength + dwRemoteAddressLength) PVOID lpOutputBuffer,
			_In_ DWORD dwReceiveDataLength,
			_In_ DWORD dwLocalAddressLength,
			_In_ DWORD dwRemoteAddressLength,
			_Out_ LPDWORD lpdwBytesReceived,
			_Inout_ LPOVERLAPPED lpOverlapped)
		{
			static LPFN_ACCEPTEX acceptex_func = nullptr;
			if (!acceptex_func)
			{
				GUID acceptex_guid = WSAID_ACCEPTEX;
				DWORD bytes_returned;
				if (WSAIoctl(sListenSocket,
					SIO_GET_EXTENSION_FUNCTION_POINTER,
					&acceptex_guid,
					sizeof(acceptex_guid),
					&acceptex_func,
					sizeof(acceptex_func),
					&bytes_returned, NULL, NULL))
					return false;
			}

			const int result = acceptex_func
			(
				sListenSocket,
				sAcceptSocket,
				lpOutputBuffer,
				dwReceiveDataLength,
				dwLocalAddressLength,
				dwRemoteAddressLength,
				lpdwBytesReceived,
				lpOverlapped
			);

			return result == TRUE || WSAGetLastError() == WSA_IO_PENDING;
		}

		bool DisconnectEx(_In_ SOCKET s,
			_Inout_opt_ LPOVERLAPPED lpOverlapped,
			_In_ DWORD  dwFlags,
			_In_ DWORD  dwReserved)
		{
			static LPFN_DISCONNECTEX disconnectex_func = nullptr;
			if (!disconnectex_func)
			{
				GUID disconnectex_guid = WSAID_DISCONNECTEX;
				DWORD bytes_returned;
				if (WSAIoctl(s,
					SIO_GET_EXTENSION_FUNCTION_POINTER,
					&disconnectex_guid,
					sizeof(disconnectex_guid),
					&disconnectex_func,
					sizeof(disconnectex_func),
					&bytes_returned,
					NULL,
					NULL))
					return false;
			}

			const int result = disconnectex_func
			(
				s,
				lpOverlapped,
				dwFlags,
				dwReserved
			);

			return result == TRUE || WSAGetLastError() == WSA_IO_PENDING;
		}
	}
}