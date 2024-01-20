//
// Created by Kiril on 16.03.2023.
//

#include <fstream>
#include <iostream>
#include "loader.h"


namespace gorilla::asset_loader
{

asset::obj load_mesh(const std::string& filename)
{
	return asset::obj::load_obj(filename);
}

std::vector<asset::obj> load_meshes(const std::vector<std::string>& filenames)
{
	std::vector<asset::obj> meshes;
	for (const std::string& filename : filenames)
		meshes.emplace_back(load_mesh(filename));

	return meshes;
}

asset::image load_image(const std::string& filename)
{
	return *asset::image::load_image(filename);
}

std::vector<asset::image> load_images(const std::vector<std::string>& filenames)
{
	std::vector<asset::image> images;

	for (const std::string& filename : filenames)
		images.emplace_back(load_image(filename));

	return images;
}

raw_spirv load_shader(const std::string& filename)
{
	// TODO: implement validation

	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		throw std::runtime_error("Cannot open file");

	const int size = file.tellg();
	file.seekg(0);

	std::vector<uint32_t> bytes((size - 1) / 4 + 1);
	file.read(reinterpret_cast<char*>(bytes.data()), size);
	file.close();

	return bytes;
}

raw_spirv compile_shader(const std::string& filename, const shader_compiler::compiler& compiler, const shader_compiler::compilation_options& options, gorilla::shader_compiler::shader_kind kind)
{
	std::ifstream file(filename, std::ios::in | std::ios::ate);
	if (!file.is_open())
		throw std::runtime_error("Cannot open file");

	const int size = file.tellg();
	file.seekg(0);

	std::string source;
	source.resize(size);

	file.read(source.data(), size);
	file.close();

	shader_compiler::compiler::result_spirv_code result = compiler.compile_into_spv(source, kind, options, filename);

	if (result.n_errors() > 0)
	{
		std::cout << result.error_message() << "\n";
		throw std::runtime_error("Shader compilation failed!");
	}

	return result.data();
}

raw_spirv load_or_compile_shader(const std::string& filename, const shader_compiler::compiler& compiler, const shader_compiler::compilation_options& options)
{
	std::string ext{};

	// TODO: implement extension

	if (ext == ".spv")
		return load_shader(filename);

	return compile_shader(filename, compiler, options, gorilla::shader_compiler::shader_kind::infer_src);
}

std::vector<raw_spirv> load_shaders(const std::vector<std::string>& filenames)
{
	std::vector<raw_spirv> shaders;

	for (const std::string& filename : filenames)
		shaders.emplace_back(load_shader(filename));

	return shaders;
}

std::vector<raw_spirv> load_shaders(const std::vector<std::string>& filenames, const shader_compiler::compiler& compiler, const shader_compiler::compilation_options& options)
{
	std::vector<raw_spirv> shaders;

	for (const std::string& filename : filenames)
		shaders.emplace_back(load_or_compile_shader(filename, compiler, options));

	return shaders;
}

} // gorilla::asset_loader