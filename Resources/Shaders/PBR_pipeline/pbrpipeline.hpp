#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace gorilla {

class pbr_pipeline {
public:
	pbr_pipeline(
		const vk::raii::Device& dev,
		const vk::raii::RenderPass& render_pass,
		uint32_t subpass_index,
		const vk::raii::DescriptorSetLayout& scenewise_layout,
		const vk::raii::DescriptorSetLayout& objectwise_layout);

	static void compile_shaders();

	[[nodiscard]] const vk::Pipeline& pipeline() const { return *_pipeline; }
	[[nodiscard]] const vk::PipelineLayout& layout() const { return *_layout; }
private:
	std::vector<vk::raii::ShaderModule> _stages;
	vk::DescriptorSetLayout _scenewise_layout;
	vk::DescriptorSetLayout _objectwise_layout;
	vk::raii::PipelineLayout _layout = nullptr;
	vk::raii::Pipeline _pipeline = nullptr;
};

}
