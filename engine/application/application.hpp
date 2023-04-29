#ifndef DEEPLOM_APPLICATION_HPP
#define DEEPLOM_APPLICATION_HPP

#include <fstream>
#include <unordered_map>

#include <engine.hpp>
#include <window/window.hpp>
#include "camera/camera.h"
#include "misc/imguiobject.hpp"

#include "mesh/mesh.hpp"
#include "img/image.h"


#include <engine/application/scene.h>

#include <vk_utils/environment.hpp>

namespace gorilla {

class Application {

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct view_projection
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 camPos;
	};

	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	engine::scene scene;
	view_projection view_proj;

	const int maxFrames = 2;
	bool frameBufferResized = false;

public:
    Application(std::string name, int w, int h, std::vector<uint32_t> vert, std::vector<uint32_t> frag, const engine::scene& scene);

    Application(const Application &other) = delete;
    Application& operator =(const Application &other) = delete;

    Application(Application &&other) = delete;
    Application& operator =(Application &&other) = delete;

    void run();


	std::vector<uint32_t> vertCode;
	std::vector<uint32_t> fragCode;

private:

	camera cam = camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, -6.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	float lastx = 0.0f;
	float lasty = 0.0f;
	bool first_mouse = true;

	void initVulkan();

	// Basic environment: graphics + present queues
	gorilla::glfw::window window;
	vk_utils::environment environment;

	static vk_utils::environment initEnvironment(const glfw::window& window);
		static vk::raii::Instance createInstance(const vk::raii::Context& context, const glfw::window& window);
		static vk::raii::SurfaceKHR createSurface(const glfw::window& window, const vk::raii::Instance& instance);
		static vk::raii::PhysicalDevice createPhysDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, int& graphicsFamily, int& presentFamily);
		static vk::raii::Device createLogicalDevice(const vk::raii::PhysicalDevice& phys_device, int graphics_family, int present_family);
		static vk::raii::Queue createQueue(const vk::raii::Device& device, int index);
	//--

	// Swapchain and images
    vk::raii::SwapchainKHR swapchain = nullptr;
    std::vector<vk::Image> scImages;
    vk::Format scImageFormat;
    vk::Extent2D scImageExtent;
    std::vector<vk::raii::ImageView> imageViews;

	void initSwapchain(bool verbose = false);
		void createSwapChain(bool verbose = false);
		void createImageViews(bool verbose = false);

	void recreateSwapChain(bool verbose = false);
	//--

	// Render State Declaration
    vk::raii::RenderPass renderPass = nullptr;
	vk::raii::DescriptorSetLayout scenewise_descriptor_set_layout = nullptr;
	vk::raii::DescriptorSetLayout objectwise_descriptor_set_layout = nullptr;
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    std::vector<vk::raii::Pipeline> pipelines;
    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::raii::CommandPool commandPool = nullptr;

	void createRenderPass(bool verbose = false);
	void createDescriptorSetLayout(bool verbose = false);
	void createGraphicsPipeline(bool verbose = false);
	void createFramebuffers(bool verbose = false);
	void createCommandPool(bool verbose = false);
	//--

	//-- Render State proper
	vk::raii::Buffer view_projection_buffer = nullptr;
	vk::raii::DeviceMemory view_projection_buffer_memory = nullptr;

	vk::raii::Buffer light_sources_buffer = nullptr;
	vk::raii::DeviceMemory light_sources_buffer_memory = nullptr;

	vk::raii::Buffer models_buffer = nullptr;
	vk::raii::DeviceMemory models_buffer_memory = nullptr;

    vk::raii::DescriptorPool descriptorPool = nullptr;
	std::vector<vk::raii::DescriptorSet> scenewise_descriptor_sets;
    std::vector<vk::raii::DescriptorSet> objectwise_descriptor_sets;
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    vk::raii::Image depthImage = nullptr;
    vk::raii::DeviceMemory depthImageMemory = nullptr;
    vk::raii::ImageView depthImageView = nullptr;

	void createUniformBuffers(bool verbose = false);
	void createDepthResources(bool verbose = false);
	void createTextureImage(const gorilla::asset::image& image, engine::texture& res, vk::Format format);
	void createTextureImageView(engine::texture& res, vk::Format format);
	void createTextureSampler(engine::texture& res);

	void createCommandBuffers(bool verbose = false);
	void createDescriptorPool(bool verbose = false);
	void createDescriptorSets(bool verbose = false);
	//--

	std::vector<engine::scene::prop> props;

	void loadModel(const std::string& name, engine::scene::prop& res, bool verbose = false);
	void createVertexBuffers(engine::scene::prop& res, bool verbose = false);
	void createIndexBuffers(engine::scene::prop& res, bool verbose = false);

    uint32_t currentFrame = 0;

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> isFlightFences;

	void createSyncObjects(bool verbose = false);


	// imgui specifics
	vk::raii::DescriptorPool ImguiDescriptorPool = nullptr;
	std::unique_ptr<engine::misc::imgui_object> imgui;
	bool isMenuShowing = false;
	bool lockChange = false;

	void createImguiDescriptorPool(bool verbose = false);
	//--

	void drawFrame();


	static vk::raii::PhysicalDevice pickDevice(const std::vector<vk::raii::PhysicalDevice> &devices);

	static SwapChainSupportDetails querySupport(const vk_utils::environment& environment);
    static std::pair<vk::Format, vk::ColorSpaceKHR> pickBestFormat(const SwapChainSupportDetails& details);
    static vk::PresentModeKHR pickPresentMode(const SwapChainSupportDetails& details);

	static uint32_t findMemoryType(const vk_utils::environment& environment, uint32_t filter, vk::MemoryPropertyFlags properties);
	static vk::Format findSupportedFormat(
			const vk_utils::environment& environment,
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling,
			vk::FormatFeatureFlags features);
	static vk::Format findDepthFormat(const vk_utils::environment& environment);

	static vk::raii::ShaderModule createShader(const vk_utils::environment& environment, const std::vector<uint32_t>& code);
	static std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(const vk_utils::environment& environment, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	static std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
			const vk_utils::environment& environment,
			uint32_t width,
			uint32_t height,
			uint32_t  mipLevels,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties);

	static std::vector<char> readFile(const std::string& filename);

	static vk::raii::CommandBuffer beginSingleTimeCommands(const vk_utils::environment& environment, const vk::raii::CommandPool& cmd_pool);
	static void endSingleTimeCommands(const vk_utils::environment& environment, const vk::raii::CommandBuffer& buffer);

	static void copyBuffer(
			const vk_utils::environment& environment,
			const vk::raii::CommandPool& cmd_pool,
			const vk::raii::Buffer& srcBuffer,
			const vk::raii::Buffer& dstBuffer,
			vk::DeviceSize size);
	static void copyBufferToImage(
			const vk_utils::environment& environment,
			const vk::raii::CommandPool& cmd_pool,
			vk::Buffer buffer,
			vk::Image image,
			uint32_t width,
			uint32_t height);

	static void transitionImageLayout(
			const vk_utils::environment& environment,
			const vk::raii::CommandPool& cmd_pool,
			vk::Image image,
			vk::Format format,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			uint32_t mipLevels);
	static void generateMipmaps(
			const vk_utils::environment& environment,
			const vk::raii::CommandPool& cmd_pool,
			vk::Image img,
			vk::Format imageFormat,
			int32_t texW,
			int32_t texH,
			uint32_t mipLevels);

	static bool hasStencilComponent(vk::Format format);

    void recordCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t imgIndex) const;

    void updateUniformBuffer(uint32_t currentImage);
};

} // gorilla

#endif //DEEPLOM_APPLICATION_HPP
