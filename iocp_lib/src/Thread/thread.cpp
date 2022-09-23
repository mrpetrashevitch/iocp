#include "thread.h"

namespace thread
{
	thread::thread()
	{
	}

	thread::~thread()
	{
		exit();
	}

	void thread::run()
	{
		std::lock_guard lock(_mut);
		if (!_task) return;
		exit();
		_task_work = true;
		std::thread t(_task);
		_thread.swap(t);
	}

	void thread::exit()
	{
		std::unique_lock lock(_mut);
		if (_task_work)
		{
			if (_exit) _exit();
			_con.wait(lock, [this] {return !_task_work; });
		}
		if (_thread.joinable()) _thread.join();
	}

	bool thread::is_work()
	{
		std::unique_lock lock(_mut);
		return _task_work;
	}
}