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
#include <Resources/Shaders/PBR_pipeline/pbrpipeline.hpp>

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
	glfw::window _window;

	vk::raii::Instance _instance = nullptr;
	vk::raii::PhysicalDevice _phys_device = nullptr;
	vk::raii::Device _device = nullptr;

	vk_utils::queue _graphics_queue;
	vk_utils::queue _present_queue;
	vk_utils::queue _transfer_queue;

	void initEnvironment();
	//--

	// Swapchain and images
	vk::raii::SurfaceKHR _surface = nullptr;
    vk::raii::SwapchainKHR _swapchain = nullptr;
    std::vector<vk::Image> _sc_images;
    vk::Format _sc_format;
	vk::ColorSpaceKHR _sc_cspace;
	vk::PresentModeKHR _sc_present_mode;
    vk::Extent2D _sc_extent;
    std::vector<vk::raii::ImageView> _sc_image_views;

	void initSwapchain(bool verbose = false);
		void createImageViews(bool verbose = false);

	void recreateSwapChain(bool verbose = false);
	//--

	// Render State Declaration
    vk::raii::RenderPass _render_pass = nullptr;
	vk::raii::DescriptorSetLayout _scenewise_descriptor_set_layout = nullptr;
	vk::raii::DescriptorSetLayout _objectwise_descriptor_set_layout = nullptr;
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    std::vector<vk::raii::Pipeline> _pipelines;
	std::unique_ptr<pbr_pipeline> _pbr_pipeline = nullptr;
    std::vector<vk::raii::Framebuffer> _framebuffer;
    vk::raii::CommandPool _cmd_pool = nullptr;

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
	vk::raii::DescriptorSet _scenewise_descriptor_set = nullptr;
    std::vector<vk::raii::DescriptorSet> objectwise_descriptor_sets;
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    vk::raii::Image _depth_image = nullptr;
    vk::raii::DeviceMemory _depth_image_memory = nullptr;
    vk::raii::ImageView _depth_image_view = nullptr;

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
	void createSkybox();

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

	void show_meshes();
	void show_point_lights();
	void show_spot_lights();
	void show_control_window();
	void drawFrame();


    static std::pair<vk::Format, vk::ColorSpaceKHR> pickBestFormat(const SwapChainSupportDetails& details);
    static vk::PresentModeKHR pickPresentMode(const SwapChainSupportDetails& details);

	uint32_t findMemoryType(uint32_t filter, vk::MemoryPropertyFlags properties);
	vk::Format findSupportedFormat(
			const std::vector<vk::Format>& candidates,
			vk::ImageTiling tiling,
			vk::FormatFeatureFlags features);
	vk::Format findDepthFormat();

	vk::raii::ShaderModule createShader(const std::vector<uint32_t>& code);
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(
			vk::DeviceSize size,
			vk::BufferUsageFlags usage,
			vk::MemoryPropertyFlags properties);
	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
			uint32_t width,
			uint32_t height,
			uint32_t mipLevels,
			vk::Format format,
			vk::ImageTiling tiling,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties);

	static std::vector<char> readFile(const std::string& filename);

	vk::raii::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(const vk::raii::CommandBuffer& buffer);

	void copyBuffer(
			const vk::raii::Buffer& srcBuffer,
			const vk::raii::Buffer& dstBuffer,
			vk::DeviceSize size);
	void copyBufferToImage(
			const vk::raii::Buffer& buffer,
			const vk::raii::Image& image,
			uint32_t width,
			uint32_t height);

	void transitionImageLayout(
			vk::Image image,
			vk::Format format,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			uint32_t mipLevels);
	void generateMipmaps(
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
