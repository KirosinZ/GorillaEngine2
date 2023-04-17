//
// Created by Kiril on 31.01.2023.
//

#include "compiler.h"
#include "compilation_options.h"

using namespace gorilla::shader_compiler;

spirv_version compiler::spv_version()
{
	uint32_t major, minor;
	shaderc_get_spv_version(
			&major,
			&minor);
	return spirv_version(major);
}

compiler::~compiler()
{
	shaderc_compiler_release(_compiler);
	_compiler = nullptr;
}

compiler::compiler(compiler&& move) noexcept
{
	_compiler = move._compiler;
	move._compiler = nullptr;
}

compiler& compiler::operator=(compiler&& move) noexcept
{
	if (_compiler == move._compiler)
		return *this;

	shaderc_compiler_release(_compiler);
	_compiler = move._compiler;
	move._compiler = nullptr;

	return *this;
}

compiler::result_spirv_code compiler::compile_into_spv(
		const std::string& source,
		const shader_kind kind,
		const options& options,
		const std::string& input_filename,
		const std::string& entrypoint_name) const
{
	return result_spirv_code(
			shaderc_compile_into_spv(
					_compiler,
					source.c_str(),
					source.size(),
					shaderc_shader_kind(kind),
					input_filename.c_str(),
					entrypoint_name.c_str(),
					options._options));
}

compiler::result_spirv_asm compiler::compile_into_spv_assembly(
		const std::string& source,
		const shader_kind kind,
		const options& options,
		const std::string& input_filename,
		const std::string& entrypoint_name)
{
	return result_spirv_asm(
			shaderc_compile_into_spv_assembly(
					_compiler,
					source.c_str(),
					source.size(),
					shaderc_shader_kind(kind),
					input_filename.c_str(),
					entrypoint_name.c_str(),
					options._options));
}

compiler::result_preprocessed_text compiler::compile_into_preprocessed_text(
		const std::string& source,
		const shader_kind kind,
		const options& options,
		const std::string& input_filename,
		const std::string& entrypoint_name)
{
	return result_preprocessed_text(
			shaderc_compile_into_preprocessed_text(
					_compiler,
					source.c_str(),
					source.size(),
					shaderc_shader_kind(kind),
					input_filename.c_str(),
					entrypoint_name.c_str(),
					options._options));
}

compiler::result_spirv_code compiler::compile_asm_into_spv(
		const std::string& source_asm,
		const options& options)
{
	return result_spirv_code(
			shaderc_assemble_into_spv(
					_compiler,
					source_asm.c_str(),
					source_asm.size(),
					options._options));
}