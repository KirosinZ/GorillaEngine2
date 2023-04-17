//
// Created by Kiril on 21.02.2023.
//

#ifndef DEEPLOM_EVENT_H
#define DEEPLOM_EVENT_H

#include <functional>

namespace gorilla::events
{

template<typename Owner, typename T>
class event;

template<typename Owner, typename... Args>
class event<Owner, void(Args...)>
{
	friend Owner;
public:
	using function_t = std::function<void(Args...)>;

	event() = default;

	event(const event& copy) = delete;
	event& operator=(const event& copy) = delete;

	event(event&& move) noexcept = default;
	event& operator=(event&& move) noexcept = default;

	event& operator+=(const function_t func)
	{
		functions.push_back(func);

		return *this;
	}

private:
	std::vector<function_t> functions;

	void operator()(Args... args)
	{
		for (int i = 0; i < functions.size(); i++)
		{
			functions[i](args...);
		}
	}
};

}

#endif //DEEPLOM_EVENT_H
