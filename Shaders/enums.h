//
// Created by Kiril on 16.02.2023.
//

#ifndef DEEPLOM_ENUMS_H
#define DEEPLOM_ENUMS_H

#include <shaderc/shaderc.h>

namespace gorilla::shaders
{

enum class environment : uint8_t
{
	vulkan = shaderc_target_env_vulkan,
	opengl = shaderc_target_env_opengl,
	opengl_compat = shaderc_target_env_opengl_compat,
	default_ = vulkan,
};

enum class environment_version :  uint32_t
{
	vulkan_1_0 = shaderc_env_version_vulkan_1_0,
	vulkan_1_1 = shaderc_env_version_vulkan_1_1,
	vulkan_1_2 = shaderc_env_version_vulkan_1_2,
	vulkan_1_3 = shaderc_env_version_vulkan_1_3,

	opengl_4_5 = shaderc_env_version_opengl_4_5,

	default_ = 0,
};

enum class spirv_version : uint32_t
{
	spirv_1_0 = shaderc_spirv_version_1_0,
	spirv_1_1 = shaderc_spirv_version_1_1,
	spirv_1_2 = shaderc_spirv_version_1_2,
	spirv_1_3 = shaderc_spirv_version_1_3,
	spirv_1_4 = shaderc_spirv_version_1_4,
	spirv_1_5 = shaderc_spirv_version_1_5,
	spirv_1_6 = shaderc_spirv_version_1_6,
};

enum class language : uint8_t
{
	glsl = shaderc_source_language_glsl,
	hlsl = shaderc_source_language_hlsl,
};

enum class shader_kind : uint8_t
{
	infer_src = shaderc_glsl_infer_from_source,

	vertex = shaderc_vertex_shader,
	fragment = shaderc_fragment_shader,
	compute = shaderc_compute_shader,
	geometry = shaderc_geometry_shader,
	tess_control = shaderc_tess_control_shader,
	tess_evaluation = shaderc_tess_evaluation_shader,

	fallback_vertex = shaderc_glsl_default_vertex_shader,
	fallback_fragment = shaderc_glsl_default_fragment_shader,
	fallback_compute = shaderc_glsl_default_compute_shader,
	fallback_geometry = shaderc_glsl_default_geometry_shader,
	fallback_tess_control = shaderc_glsl_default_tess_control_shader,
	fallback_tess_evaluation = shaderc_glsl_default_tess_evaluation_shader,

	spirv_asm = shaderc_spirv_assembly,

	rt_raygen = shaderc_raygen_shader,
	rt_anyhit = shaderc_anyhit_shader,
	rt_closesthit = shaderc_closesthit_shader,
	rt_miss = shaderc_miss_shader,
	rt_intersection = shaderc_intersection_shader,
	rt_callable = shaderc_callable_shader,

	rt_fallback_raygen = shaderc_glsl_default_raygen_shader,
	rt_fallback_anyhit = shaderc_glsl_default_anyhit_shader,
	rt_fallback_closesthit = shaderc_glsl_default_closesthit_shader,
	rt_fallback_miss = shaderc_glsl_default_miss_shader,
	rt_fallback_intersection = shaderc_glsl_default_intersection_shader,
	rt_fallback_callable = shaderc_glsl_default_callable_shader,

	task = shaderc_task_shader,
	mesh = shaderc_mesh_shader,

	fallback_task = shaderc_glsl_default_task_shader,
	fallback_mesh = shaderc_glsl_default_mesh_shader,
};

enum class shader_profile : uint8_t
{
	none = shaderc_profile_none,
	core = shaderc_profile_core,
	es   = shaderc_profile_es,
};

enum class optimization_level : uint8_t
{
	zero        = shaderc_optimization_level_zero,
	size        = shaderc_optimization_level_size,
	performance = shaderc_optimization_level_performance,
};

enum class resource_limit : uint8_t
{
	max_lights = shaderc_limit_max_lights,
	max_clip_planes = shaderc_limit_max_clip_planes,
	max_texture_units = shaderc_limit_max_texture_units,
	max_texture_coords = shaderc_limit_max_texture_coords,
	max_vertex_attribs = shaderc_limit_max_vertex_attribs,
	max_vertex_uniform_coords = shaderc_limit_max_vertex_uniform_components,
	max_varying_floats = shaderc_limit_max_varying_floats,
	max_vertex_texture_image_units = shaderc_limit_max_vertex_texture_image_units,
	max_combined_texture_image_units = shaderc_limit_max_combined_texture_image_units,
	max_texture_image_units = shaderc_limit_max_texture_image_units,
	max_fragment_uniform_components = shaderc_limit_max_fragment_uniform_components,
	max_draw_buffers = shaderc_limit_max_draw_buffers,
	max_vertex_uniform_vectors = shaderc_limit_max_vertex_uniform_vectors,
	max_varying_vectors = shaderc_limit_max_varying_vectors,
	max_fragment_uniform_vectors = shaderc_limit_max_fragment_uniform_vectors,
	max_vertex_output_vectors = shaderc_limit_max_vertex_output_vectors,
	max_fragment_input_vectors = shaderc_limit_max_fragment_input_vectors,
	min_program_texel_offset = shaderc_limit_min_program_texel_offset,
	max_program_texel_offset = shaderc_limit_max_program_texel_offset,
	max_clip_distances = shaderc_limit_max_clip_distances,
	max_compute_work_group_count_x = shaderc_limit_max_compute_work_group_count_x,
	max_compute_work_group_count_y = shaderc_limit_max_compute_work_group_count_y,
	max_compute_work_group_count_z = shaderc_limit_max_compute_work_group_count_z,
	max_compute_work_group_size_x = shaderc_limit_max_compute_work_group_size_x,
	max_compute_work_group_size_y = shaderc_limit_max_compute_work_group_size_y,
	max_compute_work_group_size_z = shaderc_limit_max_compute_work_group_size_z,
	max_compute_uniform_components = shaderc_limit_max_compute_uniform_components,
	max_compute_texture_image_units = shaderc_limit_max_compute_texture_image_units,
	max_compute_image_uniforms = shaderc_limit_max_compute_image_uniforms,
	max_compute_atomic_counters = shaderc_limit_max_compute_atomic_counters,
	max_compute_atomic_counter_buffers = shaderc_limit_max_compute_atomic_counter_buffers,
	max_varying_components = shaderc_limit_max_varying_components,
	max_vertex_output_components = shaderc_limit_max_vertex_output_components,
	max_geometry_input_components = shaderc_limit_max_geometry_input_components,
	max_geometry_output_components = shaderc_limit_max_geometry_output_components,
	max_fragment_input_components = shaderc_limit_max_fragment_input_components,
	max_image_units = shaderc_limit_max_image_units,
	max_combined_image_units_and_fragment_outputs = shaderc_limit_max_combined_image_units_and_fragment_outputs,
	max_combined_shader_output_resources = shaderc_limit_max_combined_shader_output_resources,
	max_image_samples = shaderc_limit_max_image_samples,
	max_vertex_image_uniforms = shaderc_limit_max_vertex_image_uniforms,
	max_tess_control_image_uniforms = shaderc_limit_max_tess_control_image_uniforms,
	max_tess_evaluation_image_uniforms = shaderc_limit_max_tess_evaluation_image_uniforms,
	max_geometry_image_uniforms = shaderc_limit_max_geometry_image_uniforms,
	max_fragment_image_uniforms = shaderc_limit_max_fragment_image_uniforms,
	max_combined_image_uniforms = shaderc_limit_max_combined_image_uniforms,
	max_geometry_texture_image_units = shaderc_limit_max_geometry_texture_image_units,
	max_geometry_output_vertices = shaderc_limit_max_geometry_output_vertices,
	max_geometry_total_output_components = shaderc_limit_max_geometry_total_output_components,
	max_geometry_uniform_components = shaderc_limit_max_geometry_uniform_components,
	max_geometry_varying_components = shaderc_limit_max_geometry_varying_components,
	max_tess_control_input_components = shaderc_limit_max_tess_control_input_components,
	max_tess_control_output_components = shaderc_limit_max_tess_control_output_components,
	max_tess_control_texture_image_units = shaderc_limit_max_tess_control_texture_image_units,
	max_tess_control_uniform_components = shaderc_limit_max_tess_control_uniform_components,
	max_tess_control_total_output_components = shaderc_limit_max_tess_control_total_output_components,
	max_tess_evaluation_input_components = shaderc_limit_max_tess_evaluation_input_components,
	max_tess_evaluation_output_components = shaderc_limit_max_tess_evaluation_output_components,
	max_tess_evaluation_texture_image_units = shaderc_limit_max_tess_evaluation_texture_image_units,
	max_tess_evaluation_uniform_components = shaderc_limit_max_tess_evaluation_uniform_components,
	max_tess_patch_components = shaderc_limit_max_tess_patch_components,
	max_patch_vertices = shaderc_limit_max_patch_vertices,
	max_tess_gen_level = shaderc_limit_max_tess_gen_level,
	max_viewports = shaderc_limit_max_viewports,
	max_vertex_atomic_counters = shaderc_limit_max_vertex_atomic_counters,
	max_tess_control_atomic_counters = shaderc_limit_max_tess_control_atomic_counters,
	max_tess_evaluation_atomic_counters = shaderc_limit_max_tess_evaluation_atomic_counters,
	max_geometry_atomic_counters = shaderc_limit_max_geometry_atomic_counters,
	max_fragment_atomic_counters = shaderc_limit_max_fragment_atomic_counters,
	max_combined_atomic_counters = shaderc_limit_max_combined_atomic_counters,
	max_atomic_counter_bindings = shaderc_limit_max_atomic_counter_bindings,
	max_vertex_atomic_counter_buffers = shaderc_limit_max_vertex_atomic_counter_buffers,
	max_tess_control_atomic_counter_buffers = shaderc_limit_max_tess_control_atomic_counter_buffers,
	max_tess_evaluation_atomic_counter_buffers = shaderc_limit_max_tess_evaluation_atomic_counter_buffers,
	max_geometry_atomic_counter_buffers = shaderc_limit_max_geometry_atomic_counter_buffers,
	max_fragment_atomic_counter_buffers = shaderc_limit_max_fragment_atomic_counter_buffers,
	max_combined_atomic_counter_buffers = shaderc_limit_max_combined_atomic_counter_buffers,
	max_atomic_counter_buffer_size = shaderc_limit_max_atomic_counter_buffer_size,
	max_transform_feedback_buffers = shaderc_limit_max_transform_feedback_buffers,
	max_transform_feedback_interleaved_components = shaderc_limit_max_transform_feedback_interleaved_components,
	max_cull_distances = shaderc_limit_max_cull_distances,
	max_combined_clip_and_cull_distances = shaderc_limit_max_combined_clip_and_cull_distances,
	max_samples = shaderc_limit_max_samples,
};

enum class uniform_kind : uint8_t
{
	image                 = shaderc_uniform_kind_image,
	sampler               = shaderc_uniform_kind_sampler,
	texture               = shaderc_uniform_kind_texture,
	buffer                = shaderc_uniform_kind_buffer,
	storage_buffer        = shaderc_uniform_kind_storage_buffer,
	unordered_access_view = shaderc_uniform_kind_unordered_access_view,
};

}

#endif //DEEPLOM_ENUMS_H
