//
// Created by Kiril on 16.02.2023.
//

#ifndef DEEPLOM_COMPILATION_OPTIONS_H
#define DEEPLOM_COMPILATION_OPTIONS_H

#include <string>
#include <functional>

#include "enums.h"

namespace gorilla::shader_compiler
{

class compilation_options
{
	friend class compiler;

public:
	enum class include_type : uint8_t
	{
		relative = shaderc_include_type_relative,
		standard = shaderc_include_type_standard,
	};
	struct include_result
	{
		std::string source_name;
		std::string content;
	};

	using resolver_function = std::function<include_result(
			const std::string& requested_name,
			const std::string& requesting_name,
			const include_type type,
			const size_t include_depth)>;

	compilation_options() = default;

	~compilation_options();

	compilation_options(const compilation_options& copy);

	compilation_options& operator=(const compilation_options& copy);

	compilation_options(compilation_options&& move) noexcept;

	compilation_options& operator=(compilation_options&& move) noexcept;

	void add_macro(
			const std::string& name,
			const std::string& value = "");

	void set_source_language(
			const language lang);

	void enable_debug_info();

	void set_optimization_level(
			const optimization_level level);

	void force_version_profile(
			const int version,
			const shader_profile profile);

	void set_include_callback(
			const resolver_function& resolver);

	void suppress_warnings();

	void set_target_env(
			const environment target,
			const environment_version version = environment_version::default_);

	void set_target_spirv(
			const spirv_version version);

	void set_warnings_as_errors();

	void set_limit(
			const resource_limit limit,
			int32_t value);

	void set_auto_bind_uniforms(
			const bool value);

	void set_auto_combined_image_sampler(
			const bool value);

	void set_hlsl_io_mapping(
			const bool value);

	void set_hlsl_offsets(
			const bool value);

	void set_binding_base(
			const uniform_kind kind,
			const uint32_t base);

	void set_binding_base_for_stage(
			const shader_kind kind,
			const uniform_kind ukind,
			const uint32_t base);

	void set_auto_map_locations(
			const bool enable);

	void set_hlsl_register_set_and_binding(
			const std::string& reg,
			const std::string& set,
			const std::string& binding);

	void set_hlsl_register_set_and_binding_for_stage(
			const shader_kind kind,
			const std::string& reg,
			const std::string& set,
			const std::string& binding);

	void set_hlsl_functionality1(
			const bool enable);

	void enable_16bit_types(
			const bool enable);

	void set_invert_y(
			const bool enable);

	void set_nan_clamp(
			const bool enable);

private:
	shaderc_compile_options_t _options = shaderc_compile_options_initialize();
	resolver_function _resolver;
};

} // gorilla::shader_compiler

#endif //DEEPLOM_COMPILATION_OPTIONS_H
