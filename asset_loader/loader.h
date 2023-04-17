//
// Created by Kiril on 16.03.2023.
//

#ifndef DEEPLOM_LOADER_H
#define DEEPLOM_LOADER_H

#include <string>

#include <obj/obj.h>
#include <img/image.h>
#include <shader_compiler/compiler.h>

namespace gorilla::asset_loader
{

using raw_spirv = std::vector<uint32_t>;

geom::obj load_mesh(const std::string& filename);
std::vector<geom::obj> load_meshes(const std::vector<std::string>& filenames);

img::image load_image(const std::string& filename);
std::vector<img::image> load_images(const std::vector<std::string>& filenames);

raw_spirv load_shader(const std::string& filename);
raw_spirv compile_shader(const std::string& filename, const shader_compiler::compiler& compiler, const shader_compiler::compilation_options& options, gorilla::shader_compiler::shader_kind kind);
raw_spirv load_or_compile_shader(const std::string& filename, const shader_compiler::compiler& compiler, const shader_compiler::compilation_options& options);
std::vector<raw_spirv> load_shaders(const std::vector<std::string>& filenames);
std::vector<raw_spirv> load_shaders(const std::vector<std::string>& filenames, const shader_compiler::compiler& compiler, const shader_compiler::compilation_options& options);

} // gorilla::asset_loader

#endif //DEEPLOM_LOADER_H
