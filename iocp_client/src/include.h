#pragma once
#include "../../iocp_lib/src/client/client_crater.h"
#include <windows.h>

namespace web
{
	namespace io_client
	{
		typedef void (*create)(std::shared_ptr<web::io_client::i_client>& out_client);

		class web_client_dll_loader
		{
			HMODULE _h = nullptr;
		public:
			create create_fn = nullptr;

			web_client_dll_loader(const char* path_dll)
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