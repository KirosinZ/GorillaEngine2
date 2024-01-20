#include "asserts.hpp"


using namespace gorilla;

void gorilla::asserts(
	bool expression,
	std::string_view message,
	const std::source_location& loc)
{
	if constexpr (platform::is_debug())
	{
		if (expression)
			return;

		platform::err()
			<< '['
			<< loc.file_name()
			<< ':'
			<< loc.line()
			<< ':'
			<< loc.column()
			<< "]: assertion failed";

		if (!message.empty())
			platform::err()
				<< ": "
				<< message;

		platform::err()
			<< std::endl;

		platform::terminate();
	}
}
