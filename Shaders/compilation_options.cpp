//
// Created by Kiril on 16.02.2023.
//

#include "compilation_options.h"

using namespace gorilla::shaders;

compilation_options::~compilation_options()
{
	shaderc_compile_options_release(_options);
	_options = nullptr;
}

compilation_options::compilation_options(const compilation_options& copy)
{
	_options = shaderc_compile_options_clone(copy._options);
}

compilation_options& compilation_options::operator=(const compilation_options& copy)
{
	if (this->_options == copy._options)
		return *this;

	shaderc_compile_options_release(this->_options);
	_options = shaderc_compile_options_clone(copy._options);

	return *this;
}

compilation_options::compilation_options(compilation_options&& move) noexcept
{
	_options = move._options;
	move._options = nullptr;
}

compilation_options& compilation_options::operator=(compilation_options&& move) noexcept
{
	if (this->_options == move._options)
	return *this;

	shaderc_compile_options_release(_options);
	_options = move._options;
	move._options = nullptr;

	return *this;
}

void compilation_options::add_macro(
		const std::string& name,
		const std::string& value)
{
	shaderc_compile_options_add_macro_definition(
			_options,
			name.c_str(),
			name.size(),
			value.c_str(),
			value.size());
}

void compilation_options::set_source_language(
		const language lang)
{
	shaderc_compile_options_set_source_language(
			_options,
			(shaderc_source_language)lang);
}

void compilation_options::enable_debug_info()
{
	shaderc_compile_options_set_generate_debug_info(
			_options);
}

void compilation_options::set_optimization_level(
		const optimization_level level)
{
	shaderc_compile_options_set_optimization_level(
			_options,
			(shaderc_optimization_level)level);
}

void compilation_options::force_version_profile(
		const int version,
		const shader_profile profile)
{
	shaderc_compile_options_set_forced_version_profile(
			_options,
			version,
			(shaderc_profile)profile);
}

void compilation_options::set_include_callback(
		const resolver_function& resolver)
{
	_resolver = resolver;

	const auto resolver_fn = [](void* user_data, const char* requested_source, int type, const char* requesting_source, size_t include_depth) {
		const auto* fn = static_cast<const resolver_function*>(user_data);

		const include_result& include = (*fn)(
				{requested_source},
				{requesting_source},
				include_type(type),
				include_depth);

		auto* res = new shaderc_include_result;
		res->source_name = include.source_name.c_str();
		res->source_name_length = include.source_name.size();
		res->content = include.content.c_str();
		res->content_length = include.content.size();
		res->user_data = nullptr;

		return res;
	};

	const auto releaser_fn = [](void* user_data, shaderc_include_result* include_result) {
		delete include_result;
	};

	shaderc_compile_options_set_include_callbacks(
			_options,
			resolver_fn,
			releaser_fn,
			static_cast<void*>(&_resolver));
}

void compilation_options::suppress_warnings()
{
	shaderc_compile_options_set_suppress_warnings(
			_options);
}

void compilation_options::set_target_env(
		const environment target,
		const environment_version version)
{
	shaderc_compile_options_set_target_env(
			_options,
			shaderc_target_env(target),
			uint32_t(version));
}

void compilation_options::set_target_spirv(
		const spirv_version version)
{
	shaderc_compile_options_set_target_spirv(
			_options,
			shaderc_spirv_version(version));
}


void compilation_options::set_warnings_as_errors()
{
	shaderc_compile_options_set_warnings_as_errors(
			_options);
}

void compilation_options::set_limit(
		const resource_limit limit,
		int32_t value)
{
	shaderc_compile_options_set_limit(
			_options,
			shaderc_limit(limit),
			value);
}

void compilation_options::set_auto_bind_uniforms(
		const bool value)
{
	shaderc_compile_options_set_auto_bind_uniforms(
			_options,
			value);
}

void compilation_options::set_auto_combined_image_sampler(
		const bool value)
{
	shaderc_compile_options_set_auto_combined_image_sampler(
			_options,
			value);
}

void compilation_options::set_hlsl_io_mapping(
		const bool value)
{
	shaderc_compile_options_set_hlsl_io_mapping(
			_options,
			value);
}

void compilation_options::set_hlsl_offsets(
		const bool value)
{
	shaderc_compile_options_set_hlsl_offsets(
			_options,
			value);
}

void compilation_options::set_binding_base(
		const uniform_kind kind,
		const uint32_t base)
{
	shaderc_compile_options_set_binding_base(
			_options,
			shaderc_uniform_kind(kind),
			base);
}

void compilation_options::set_binding_base_for_stage(
		const shader_kind kind,
		const uniform_kind ukind,
		const uint32_t base)
{
	shaderc_compile_options_set_binding_base_for_stage(
			_options,
			shaderc_shader_kind(kind),
			shaderc_uniform_kind(ukind),
			base);
}

void compilation_options::set_auto_map_locations(
		const bool enable)
{
	shaderc_compile_options_set_auto_map_locations(
			_options,
			enable);
}

void compilation_options::set_hlsl_register_set_and_binding(
		const std::string& reg,
		const std::string& set,
		const std::string& binding)
{
	shaderc_compile_options_set_hlsl_register_set_and_binding(
			_options,
			reg.c_str(),
			set.c_str(),
			binding.c_str());
}

void compilation_options::set_hlsl_register_set_and_binding_for_stage(
		const shader_kind kind,
		const std::string& reg,
		const std::string& set,
		const std::string& binding)
{
	shaderc_compile_options_set_hlsl_register_set_and_binding_for_stage(
			_options,
			shaderc_shader_kind(kind),
			reg.c_str(),
			set.c_str(),
			binding.c_str());
}

void compilation_options::set_hlsl_functionality1(
		const bool enable)
{
	shaderc_compile_options_set_hlsl_functionality1(
			_options,
			enable);
}

void compilation_options::enable_16bit_types(
		const bool enable)
{
	shaderc_compile_options_set_hlsl_16bit_types(
			_options,
			enable);
}

void compilation_options::set_invert_y(
		const bool enable)
{
	shaderc_compile_options_set_invert_y(
			_options,
			enable);
}

void compilation_options::set_nan_clamp(
		const bool enable)
{
	shaderc_compile_options_set_nan_clamp(
			_options,
			enable);
}
