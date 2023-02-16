#include "compilation_result.h"

template<typename Result>
gorilla::shaders::compilation_result<Result>::compilation_result(shaderc_compilation_result_t res)
		: _result(res)
{}

template<typename Result>
gorilla::shaders::compilation_result<Result>::~compilation_result()
{
	shaderc_result_release(
			_result);
	_result = nullptr;
}

template<typename Result>
Result gorilla::shaders::compilation_result<Result>::data() const
{
	const char*raw_bytes = shaderc_result_get_bytes(
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
typename gorilla::shaders::compilation_result<Result>::status gorilla::shaders::compilation_result<Result>::status() const
{
	return static_cast<enum status>(shaderc_result_get_compilation_status(
			_result));
}

template<typename Result>
size_t gorilla::shaders::compilation_result<Result>::size() const
{
	return shaderc_result_get_length(
			_result);
}

template<typename Result>
size_t gorilla::shaders::compilation_result<Result>::n_warnings() const
{
	return shaderc_result_get_num_warnings(
			_result);
}

template<typename Result>
size_t gorilla::shaders::compilation_result<Result>::n_errors() const
{
	return shaderc_result_get_num_errors(
			_result);
}

template<typename Result>
std::string gorilla::shaders::compilation_result<Result>::error_message() const
{
	return shaderc_result_get_error_message(
			_result);
}