#include "pbrpipeline.hpp"

#include <asset_loader/loader.h>

#include <filesystem>
#include <fstream>
#include <mesh/mesh.hpp>


namespace gorilla {

void pbr_pipeline::compile_shaders()
{
	shader_compiler::compilation_options options{};
	options.set_target_env(shader_compiler::environment::vulkan);
	options.set_target_spirv(shader_compiler::compiler::spv_version());
	// options.set_source_language(shader_compiler::language::glsl);
	shader_compiler::compiler cmp;

	const std::string shaders = "shaders";
	const std::string compiled_shaders = "compiled shaders";

	std::filesystem::path path("../Resources/Shaders/PBR_pipeline");
	for (const auto& dir_entry: std::filesystem::directory_iterator{path/shaders})
	{
		if (!dir_entry.is_regular_file())
			continue;

		std::filesystem::path name = dir_entry.path().filename();
		std::filesystem::path associated_compiled_shader = path/compiled_shaders/name;
		associated_compiled_shader += ".spv";

		if (exists(associated_compiled_shader) && last_write_time(dir_entry.path()) <= last_write_time(associated_compiled_shader))
			continue;

		shader_compiler::shader_kind kind = shader_compiler::shader_kind::fragment;
		if (dir_entry.path().extension() == ".vert")
			kind = shader_compiler::shader_kind::vertex;

		const std::vector<uint32_t> code = asset_loader::compile_shader((path/shaders/name).string(), cmp, options, kind);

		std::ofstream out(associated_compiled_shader, std::ios::binary);

		if (!out.is_open())
			std::terminate();

		out.write(reinterpret_cast<const char*>(code.data()), code.size() * 4);
		out.flush();

		out.close();
	}
}

pbr_pipeline::pbr_pipeline(
	const vk::raii::Device& dev,
	const vk::raii::RenderPass& render_pass,
	uint32_t subpass_index,
	const vk::raii::DescriptorSetLayout& scenewise_layout,
	const vk::raii::DescriptorSetLayout& objectwise_layout) :
	_scenewise_layout(*scenewise_layout),
	_objectwise_layout(*objectwise_layout)
{
	std::filesystem::path path{"../Resources/Shaders/PBR_pipeline/compiled shaders"};

	const std::vector<uint32_t> vert = asset_loader::load_shader((path/"shader.vert.spv").string());
	const std::vector<uint32_t> frag = asset_loader::load_shader((path/"shader.frag.spv").string());

	vk::ShaderModuleCreateInfo smci(
		{},
		vert);
	_stages.emplace_back(dev, smci);

	smci = vk::ShaderModuleCreateInfo(
		{},
		frag);
	_stages.emplace_back(dev, smci);

	std::vector stages{
		vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *_stages[0], "main"),
		vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *_stages[1], "main")
	};

	auto binding = vk_vertex_input_binding_description<struct mesh>(0);
	auto attributes = vk_vertex_input_attribute_description<struct mesh>(0);
	vk::PipelineVertexInputStateCreateInfo pvisci(
		{},
		binding,
		attributes);

	vk::PipelineInputAssemblyStateCreateInfo piasci(
		{},
		vk::PrimitiveTopology::eTriangleList,
		false);

	vk::PipelineTessellationStateCreateInfo ptsci{};

	vk::PipelineViewportStateCreateInfo pvsci(
		{},
		1,
		{},
		1,
		{});

	std::vector dyn_states{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};

	vk::PipelineRasterizationStateCreateInfo prsci(
		{},
		true,
		false,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise,
		false);

	vk::PipelineMultisampleStateCreateInfo pmsci(
		{},
		vk::SampleCountFlagBits::e1);

	vk::PipelineDepthStencilStateCreateInfo pdssci(
		{},
		true,
		true,
		vk::CompareOp::eLess,
		false,
		false);

	std::vector blend_states{
		vk::PipelineColorBlendAttachmentState(false)
	};
	blend_states[0].colorWriteMask = vk::ColorComponentFlagBits::eR
		| vk::ColorComponentFlagBits::eG
		|vk::ColorComponentFlagBits::eB
		|vk::ColorComponentFlagBits::eA;
	vk::PipelineColorBlendStateCreateInfo pcbsci(
		{},
		false,
		{},
		blend_states,
		std::array{ 0.0f, 0.0f, 0.0f, 0.0f });

	vk::PipelineDynamicStateCreateInfo pdsci(
		{},
		dyn_states);

	std::vector set_layouts{
		_scenewise_layout,
		_objectwise_layout
	};
	vk::PipelineLayoutCreateInfo plci(
		{},
		set_layouts,
		{});
	_layout = dev.createPipelineLayout(plci);

	vk::GraphicsPipelineCreateInfo gpci(
		{},
		stages,
		&pvisci,
		&piasci,
		&ptsci,
		&pvsci,
		&prsci,
		&pmsci,
		&pdssci,
		&pcbsci,
		&pdsci,
		*_layout,
		*render_pass,
		subpass_index);
	_pipeline = dev.createGraphicsPipeline(nullptr, gpci);
}


} // gorilla