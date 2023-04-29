//
// Created by Kiril on 16.03.2023.
//

#ifndef DEEPLOM_MESHGROUP_HPP
#define DEEPLOM_MESHGROUP_HPP

#include <vulkan/vulkan_raii.hpp>

#include "renderable_unit.hpp"

namespace gorilla::engine
{

class scene1
{
public:
	inline const std::vector<asset::image>& images() const { return _images; }
	inline std::vector<asset::image>& images() { return _images; }

	inline const std::vector<vk::raii::Pipeline>& pipelines() const { return _pipelines; }
private:
	std::vector<asset::image> _images;
	std::vector<vk::raii::Pipeline> _pipelines;

	std::vector<renderable_unit> _units;
};

} // gorilla::engine

#endif //DEEPLOM_MESHGROUP_HPP
