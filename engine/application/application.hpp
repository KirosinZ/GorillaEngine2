#ifndef DEEPLOM_APPLICATION_HPP
#define DEEPLOM_APPLICATION_HPP

#include <fstream>
#include <unordered_map>

#include <engine.hpp>
#include <window/window.hpp>
#include "camera/camera.h"
#include "misc/imguiobject.hpp"

#include "mesh/mesh.hpp"


#include <engine/application/scene.h>

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


	uint32_t instanceVersion;
    std::vector<vk::LayerProperties> layerProperties;
    std::vector<vk::ExtensionProperties> extensionProperties;

    std::vector<vk::QueueFamilyProperties> queueFamilies;
	std::vector<vk::LayerProperties> deviceLayerProperties;
	std::vector<vk::ExtensionProperties> deviceExtensionProperties;

	// Basic environment: graphics + present queues
	vk::raii::Context context;
	gorilla::glfw::window window;
	vk::raii::Instance instance = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::PhysicalDevice physDevice = nullptr;
	vk::raii::Device device = nullptr;


	std::vector<engine::scene::prop> props;

	uint32_t graphicsQueueFamilyIndex = 0;
	vk::raii::Queue graphicsQueue = nullptr;
	uint32_t presentQueueFamilyIndex = 0;
	vk::raii::Queue presentQueue = nullptr;

	void initEnvironment(bool verbose = false);
		void establishContext(bool verbose = false);
		void createInstance(bool verbose = false);
		void createSurface(bool verbose = false);
		void createPhysDevice(bool verbose = false);
		void createLogicalDevice(bool verbose = false);
		void createQueue(bool verbose = false);
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
	//--

    vk::raii::RenderPass renderPass = nullptr;
	vk::raii::DescriptorSetLayout scenewise_descriptor_set_layout = nullptr;
	vk::raii::DescriptorSetLayout objectwise_descriptor_set_layout = nullptr;
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    std::vector<vk::raii::Pipeline> pipelines;
    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::raii::CommandPool commandPool = nullptr;
//    vk::raii::Buffer vertexBuffer = nullptr;
//    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
//    vk::raii::Buffer indexBuffer = nullptr;
//    vk::raii::DeviceMemory indexBufferMemory = nullptr;

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

//    uint32_t mipLevels;
//    vk::raii::Image texture = nullptr;
//    vk::raii::DeviceMemory textureImageMemory = nullptr;
//    vk::raii::ImageView textureView = nullptr;
//    vk::raii::Sampler textureSampler = nullptr;

    vk::raii::Image depthImage = nullptr;
    vk::raii::DeviceMemory depthImageMemory = nullptr;
    vk::raii::ImageView depthImageView = nullptr;

//	vk::raii::Image normalMap = nullptr;
//	vk::raii::DeviceMemory normalMapMemory = nullptr;
//	vk::raii::ImageView normalMapView = nullptr;
//	vk::raii::Sampler normalMapSampler = nullptr;

    uint32_t currentFrame = 0;

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> isFlightFences;


	vk::raii::DescriptorPool ImguiDescriptorPool = nullptr;
	std::unique_ptr<engine::misc::imgui_object> imgui;
	bool isMenuShowing = false;
	bool lockChange = false;

    SwapChainSupportDetails querySupport(const vk::raii::PhysicalDevice& physicalDevice) const;
    vk::raii::PhysicalDevice pickDevice(const std::vector<vk::raii::PhysicalDevice> &devices) const;
    void pickBestFormat(const SwapChainSupportDetails& details, vk::Format& outFormat, vk::ColorSpaceKHR& outColorSpace) const;
    vk::PresentModeKHR pickPresentMode(const SwapChainSupportDetails& details) const;

    void recordCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t imgIndex) const;

    uint32_t findMemoryType(uint32_t filter, vk::MemoryPropertyFlags properties) const;

    static std::vector<char> readFile(const std::string& filename);

    vk::raii::ShaderModule createShader(const std::vector<uint32_t>& code);

    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    void copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size);

    void updateUniformBuffer(uint32_t currentImage);

    std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, uint32_t height, uint32_t  mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

    vk::raii::CommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(vk::CommandBuffer buffer);

    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling  tiling, vk::FormatFeatureFlags features);

    vk::Format findDepthFormat();

    bool hasStencilComponent(vk::Format format);

    void generateMipmaps(vk::Image img, vk::Format imageFormat, int32_t texW, int32_t texH, uint32_t mipLevels);

    void initVulkan();


    void createRenderPass(bool verbose = false);
    void createDescriptorSetLayout(bool verbose = false);
    void createGraphicsPipeline(bool verbose = false);
    void createFramebuffers(bool verbose = false);
    void createCommandPool(bool verbose = false);
    void createDepthResources(bool verbose = false);
    void createTextureImage(const std::string& name, engine::scene::prop& res, bool verbose = false);
    void createTextureImageView(engine::scene::prop& res, bool verbose = false);
    void createTextureSampler(engine::scene::prop& res, bool verbose = false);
	void createNormalImage(const std::string& name, engine::scene::prop& res, bool verbose = false);
	void createNormalImageView(engine::scene::prop& res, bool verbose = false);
	void createNormalSampler(engine::scene::prop& res, bool verbose = false);
	void createRoughnessImage(const std::string& name, engine::scene::prop& res, bool verbose = false);
	void createRoughnessImageView(engine::scene::prop& res, bool verbose = false);
	void createRoughnessSampler(engine::scene::prop& res, bool verbose = false);
    void loadModel(const std::string& name, engine::scene::prop& res, bool verbose = false);
    void createVertexBuffers(engine::scene::prop& res, bool verbose = false);
    void createIndexBuffers(engine::scene::prop& res, bool verbose = false);
    void createUniformBuffers(bool verbose = false);
    void createCommandBuffers(bool verbose = false);
    void createDescriptorPool(bool verbose = false);
    void createDescriptorSets(bool verbose = false);
    void createSyncObjects(bool verbose = false);

    void recreateSwapChain(bool verbose = false);

    void drawFrame();


	void createImguiDescriptorPool(bool verbose = false);
};

} // gorilla

#endif //DEEPLOM_APPLICATION_HPP
