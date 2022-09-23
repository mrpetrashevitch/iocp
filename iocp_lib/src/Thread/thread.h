#pragma once
#include <functional>
#include <future>
#include <mutex>
#include <thread>

namespace thread
{
	class thread
	{
		std::function<void()>_task;
		std::function<void()>_exit;
		std::thread _thread;
		std::recursive_mutex _mut;
		std::condition_variable_any _con;
		bool _task_work = false;
		template <typename F>
		void set_task(const F& task)
		{
			const std::scoped_lock lock(_mut);
			exit();
			_task = std::function<void()>(task);
			_exit = nullptr;
		}
	public:
		thread();
		~thread();
		thread(const thread&) = delete;
		void operator=(const thread&) = delete;

		template <typename F, typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<F>>>>>
		void set_func(const F& task)
		{
			set_task([this, task]
				{
					try
					{
						task();
					}
					catch (...) {}

					std::unique_lock ul(_mut);
					_task_work = false;
					_con.notify_all();
					ul.unlock();
				});
		}

		template <typename F, typename... A, typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>>>
		void set_func(const F& task, const A &...args)
		{
			set_task([this, task, args...]
				{
					try
					{
						task(args...);
					}
					catch (...) {}
					std::unique_lock ul(_mut);
					_task_work = false;
					_con.notify_all();
					ul.unlock();
				});
		}

		template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>, typename = std::enable_if_t<!std::is_void_v<R>>>
		std::future<R> set_func(const F& task)
		{
			std::shared_ptr<std::promise<R>> task_promise(std::make_shared<std::promise<R>>());
			std::future<R> future = task_promise->get_future();
			set_task([task, task_promise, this]
				{
					try
					{
						task_promise->set_value(task());
					}
					catch (...)
					{
						try
						{
							task_promise->set_exception(std::current_exception());
						}
						catch (...)
						{
						}
					}
					std::unique_lock ul(_mut);
					_task_work = false;
					_con.notify_all();
					ul.unlock();
				});
			return future;
		}

		template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>, typename = std::enable_if_t<!std::is_void_v<R>>>
		std::future<R> set_func(const F& task, const A &...args)
		{
			std::shared_ptr<std::promise<R>> task_promise(std::make_shared<std::promise<R>>());
			std::future<R> future = task_promise->get_future();
			set_task([this, task, args..., task_promise]
				{
					try
					{
						task_promise->set_value(task(args...));
					}
					catch (...)
					{
						try
						{
							task_promise->set_exception(std::current_exception());
						}
						catch (...)
						{
						}
					}
					std::unique_lock ul(_mut);
					_task_work = false;
					_con.notify_all();
					ul.unlock();
				});
			return future;
		}

		template <typename F>
		void set_exit(const F& task)
		{
			const std::scoped_lock lock(_mut);
			_exit = std::function<void()>(task);
		}

		template <typename F, typename... A>
		void set_exit(const F& task, const A &...args)
		{
			push_exit([task, args...]
				{ task(args...); });
		}

		void run();

		void exit();

		bool is_work();
	};
}