//
// Created by Kiril on 14.03.2023.
//

#ifndef DEEPLOM_INSTANCE_HPP
#define DEEPLOM_INSTANCE_HPP

#include <vulkan/vulkan_raii.hpp>
#include <set>


namespace gorilla::engine
{

class instance
{
public:

	instance();
	inline instance(std::nullptr_t) {}

	inline vk::raii::Context& vk_context()	{ return _context; };
	inline const vk::raii::Context& vk_context() const { return _context; }

	inline vk::raii::Instance& vk_instance() { return _instance; }
	inline const vk::raii::Instance& vk_instance() const { return _instance; }

	inline vk::raii::PhysicalDevice& vk_phys_device() { return _physical_device; }
	inline const vk::raii::PhysicalDevice& vk_phys_device() const { return _physical_device; }

	inline vk::raii::Device& vk_device() { return _device; }
	inline const vk::raii::Device& vk_device() const { return _device; }

	inline const std::set<std::string>& available_extensions() { return _available_extensions; }
private:
	vk::raii::Context _context;
	vk::raii::Instance _instance = nullptr;
	vk::raii::PhysicalDevice _physical_device = nullptr;
	vk::raii::Device _device = nullptr;

	std::set<std::string> _available_extensions;

	void establish_context(bool verbose = false);
	void create_instance(const std::string& app_name, uint32_t app_version, bool verbose = false);
	void create_physical_device(bool verbose);
};

} // gorilla::engine

#endif //DEEPLOM_INSTANCE_HPP
