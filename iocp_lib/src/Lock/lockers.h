#pragma once
#include <mutex>
#include <atomic>

namespace web
{
	namespace lockers
	{
		struct mutex_copy : std::mutex
		{
			mutex_copy() = default;
			mutex_copy(mutex_copy const&) noexcept : std::mutex() {}
			bool operator==(mutex_copy const& other) noexcept { return this == &other; }
		};

		class spin_lock
		{
			std::atomic_flag _locked = ATOMIC_FLAG_INIT;
		public:
			void lock() 
			{
				while (_locked.test_and_set(std::memory_order_acquire)) { ; }
			}
			bool try_lock()
			{
				return !_locked.test_and_set(std::memory_order_acquire);
			}
			void unlock() 
			{
				_locked.clear(std::memory_order_release);
			}
		};
	}
}