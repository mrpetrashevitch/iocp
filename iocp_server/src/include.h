#pragma once
#include "../../iocp_lib/src/server/server_crater.h"
#include <windows.h>

namespace web
{
	namespace io_server
	{
		typedef void (*create)(std::shared_ptr<web::io_server::i_server>& out_server);

		class web_server_dll_loader
		{
			HMODULE _h = nullptr;
		public:
			create create_fn = nullptr;

			web_server_dll_loader(const char* path_dll)
			{
				_h = LoadLibraryA(path_dll);
				if (_h)
				{
					create_fn = (create)GetProcAddress(_h, "create");
				}
			}
		};
	}
}