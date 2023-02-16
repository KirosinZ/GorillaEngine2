//
// Created by Kiril on 16.02.2023.
//

#ifndef DEEPLOM_COMPILATION_RESULT_H
#define DEEPLOM_COMPILATION_RESULT_H

#include <string>
#include <vector>

#include <shaderc/shaderc.h>

namespace gorilla::shaders
{

template<typename Result>
class compilation_result
{
	friend class compiler;

public:
	enum class status : uint8_t
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

	status status() const;

	size_t size() const;

	size_t n_warnings() const;

	size_t n_errors() const;

	std::string error_message() const;

private:
	explicit compilation_result(shaderc_compilation_result_t res);

	shaderc_compilation_result_t _result = nullptr;
};

}

#include "compilation_result.tpp"
#endif //DEEPLOM_COMPILATION_RESULT_H
