#pragma once

#include <source_location>

#include <iostream>

#include "platform.hpp"


namespace gorilla
{

void asserts(bool expression, std::string_view message = "", const std::source_location& loc = std::source_location::current());

void asserts_eq(
	auto lhs,
	auto rhs,
	std::string_view message = "",
	const std::source_location& loc = std::source_location::current())
{
	if constexpr (platform::is_debug())
	{
		if (lhs == rhs)
			return;

		platform::err()
			<< '['
			<< loc.file_name()
			<< ':'
			<< loc.line()
			<< ':'
			<< loc.column()
			<< "]: compared values are not equal";

		if (!message.empty())
			platform::err()
				<< ": "
				<< message;

		platform::err()
			<< std::endl;

		platform::terminate();
	}
}

}
