#pragma once

#include <queue>
#include <mutex>

namespace web
{
	namespace containers
	{
		template <class T>
		class safe_queue
		{
		public:
			safe_queue()
				: q()
				, m()
			{}

			~safe_queue()
			{}

			void enqueue(T t)
			{
				std::lock_guard<std::mutex> lock(m);
				q.push(t);
			}

			bool dequeue(T& ret)
			{
				std::unique_lock<std::mutex> lock(m);
				if (q.empty()) return false;
				ret = std::move(q.front());
				q.pop();
				return true;
			}

		private:
			std::queue<T> q;
			mutable std::mutex m;
		};
	}
}
