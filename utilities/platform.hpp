#pragma once

#include <filesystem>


namespace gorilla
{

struct platform
{
	enum class build_profile
	{
		debug,
		release
	};

#ifdef PROJECT_PROFILE_DEBUG
	static constexpr build_profile profile = build_profile::debug;
#endif

#ifdef PROJECT_PROFILE_RELEASE
	static constexpr build_profile profile = build_profile::release;
#endif

	inline static const std::filesystem::path project_dir{ PROJECT_PATH };
	inline static const std::filesystem::path resource_dir{ RESOURCE_PATH };

	static constexpr bool is_debug() { return profile == build_profile::debug; }

	static constexpr std::ostream& out() { return std::cout; }
	static constexpr std::ostream& err() { return std::cerr; }

	[[noreturn]] static void terminate(std::string_view msg = "")
	{
		if (!msg.empty())
			err() << msg;

		std::terminate();
	}

	static void initialize(int argc, char** argv);
	static void deinitialize();
};

}
