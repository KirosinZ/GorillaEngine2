//
// Created by Kiril on 16.02.2023.
//

#ifndef DEEPLOM_COMPILATION_RESULT_H
#define DEEPLOM_COMPILATION_RESULT_H

#include <string>
#include <vector>

#include <shaderc/shaderc.h>

namespace gorilla::shader_compiler
{

template<typename Result>
class compilation_result
{
	friend class compiler;

public:
	enum class status_t : uint8_t
	{
		success = shaderc_compilation_status_success,
		invalid_stage = shaderc_compilation_status_invalid_stage,
		compilation_error = shaderc_compilation_status_compilation_error,
		internal_error = shaderc_compilation_status_internal_error,
		null_result = shaderc_compilation_status_null_result_object,
		invalid_assembly = shaderc_compilation_status_invalid_assembly,
		validation_error = shaderc_compilation_status_validation_error,
		transformation_error = shaderc_compilation_status_transformation_error,
		configuration_error = shaderc_compilation_status_configuration_error,
	};

	~compilation_result();

	Result data() const;

	status_t status() const;

	size_t size() const;

	size_t n_warnings() const;

	size_t n_errors() const;

	std::string error_message() const;

private:
	explicit compilation_result(shaderc_compilation_result_t res);

	shaderc_compilation_result_t _result = nullptr;
};

}








// IMPLEMENTATION


template<typename Result>
gorilla::shader_compiler::compilation_result<Result>::compilation_result(shaderc_compilation_result_t res)
		: _result(res)
{}

template<typename Result>
gorilla::shader_compiler::compilation_result<Result>::~compilation_result()
{
	shaderc_result_release(
			_result);
	_result = nullptr;
}

template<typename Result>
Result gorilla::shader_compiler::compilation_result<Result>::data() const
{
	const char* raw_bytes = shaderc_result_get_bytes(
			_result);
	const size_t code_size = this->size();

	if constexpr (std::is_same_v <Result, std::vector<uint32_t>>)
	{
		return std::vector<uint32_t>((uint32_t*)raw_bytes,
		                             (uint32_t*)raw_bytes + code_size / sizeof(uint32_t));
	}
	if constexpr (std::is_same_v<Result, std::string>)
	{
		return std::string(
				raw_bytes,
				code_size);
	}
}

template<typename Result>
typename gorilla::shader_compiler::compilation_result<Result>::status_t gorilla::shader_compiler::compilation_result<Result>::status() const
{
	return static_cast<enum status_t>(shaderc_result_get_compilation_status(
			_result));
}

template<typename Result>
size_t gorilla::shader_compiler::compilation_result<Result>::size() const
{
	return shaderc_result_get_length(
			_result);
}

template<typename Result>
size_t gorilla::shader_compiler::compilation_result<Result>::n_warnings() const
{
	return shaderc_result_get_num_warnings(
			_result);
}

template<typename Result>
size_t gorilla::shader_compiler::compilation_result<Result>::n_errors() const
{
	return shaderc_result_get_num_errors(
			_result);
}

template<typename Result>
std::string gorilla::shader_compiler::compilation_result<Result>::error_message() const
{
	return shaderc_result_get_error_message(
			_result);
}

#endif //DEEPLOM_COMPILATION_RESULT_H
