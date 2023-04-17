//
// Created by Kiril on 31.01.2023.
//

#ifndef DEEPLOM_COMPILER_H
#define DEEPLOM_COMPILER_H

#include "compilation_options.h"
#include "compilation_result.h"

namespace gorilla::shader_compiler
{

class compiler
{
public:
	using options = compilation_options;

	using result_spirv_code = compilation_result<std::vector<uint32_t>>;
	using result_spirv_asm = compilation_result<std::string>;
	using result_preprocessed_text = compilation_result<std::string>;

	static spirv_version spv_version();

	compiler() = default;
	~compiler();

	compiler(const compiler&) = delete;
	compiler& operator=(const compiler&) = delete;

	compiler(compiler&& move) noexcept;
	compiler& operator=(compiler&& move) noexcept;

	result_spirv_code compile_into_spv(
			const std::string& source,
			const shader_kind kind,
			const options& options,
			const std::string& input_filename = "",
			const std::string& entrypoint_name = "main") const;

	result_spirv_asm compile_into_spv_assembly(
			const std::string& source,
			const shader_kind kind,
			const options& options,
			const std::string& input_filename = "",
			const std::string& entrypoint_name = "main");

	result_preprocessed_text compile_into_preprocessed_text(
			const std::string& source,
			const shader_kind kind,
			const options& options,
			const std::string& input_filename = "",
			const std::string& entrypoint_name = "main");

	result_spirv_code compile_asm_into_spv(
			const std::string& source_asm,
			const options& options);

private:
	shaderc_compiler_t _compiler = shaderc_compiler_initialize();
};

} // gorilla

#endif //DEEPLOM_COMPILER_H
