#pragma once
#include "../defs.h"
#include <memory>

namespace web::web_cryptor
{
	class i_cryptor
	{
	public:
		virtual int need_size(byte* src, int size)
		{
			return size;
		}
		virtual bool encrypt(byte* src, int size, byte* out_buff, int* out_size)
		{
			if (!src || !out_buff || !out_size || size < 1) return false;
			memcpy(out_buff, src, size);
			*out_size = size;
			return true;
		}
		virtual bool decrypt(byte* src, int size, byte* out_buff, int* out_size)
		{
			if (!src || !out_buff || !out_size || size < 1) return false;
			memcpy(out_buff, src, size);
			*out_size = size;
			return true;
		}
	};

	typedef std::unique_ptr<web::web_cryptor::i_cryptor> cryptor_ptr;
}