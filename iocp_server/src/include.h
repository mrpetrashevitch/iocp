#pragma once
#include "../../iocp_lib/src/IOBase/i_server.h"

namespace web
{
	namespace io_server
	{
		typedef i_server* (*create)(byte s_b1, byte s_b2, byte s_b3, byte s_b4, ushort port);
		typedef bool (*destroy)(i_server* web);

		class web_server_dll_loader
		{
			HMODULE _h = nullptr;
		public:
			create create_fn = nullptr;
			destroy destroy_fn = nullptr;

			web_server_dll_loader(const char* path_dll)
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