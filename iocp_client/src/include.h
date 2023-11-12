#pragma once
#include "../../iocp_lib/src/IOBase/i_client.h"

namespace web
{
	namespace io_client
	{
		typedef i_client* (*create)(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port);
		typedef bool (*destroy)(i_client* web);

		class web_client_dll_loader
		{
			HMODULE _h = nullptr;
		public:
			create create_fn = nullptr;
			destroy destroy_fn = nullptr;

			web_client_dll_loader(const char* path_dll)
			{
				_h = LoadLibraryA(path_dll);
				if (_h)
				{
					create_fn = (create)GetProcAddress(_h, "create");
					destroy_fn = (destroy)GetProcAddress(_h, "destroy");
				}
			}
		};
	}
}