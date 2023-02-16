#ifndef DEEPLOM_APPLICATION_HPP
#define DEEPLOM_APPLICATION_HPP

#include <Engine/engine.hpp>
#include <Window/window.hpp>
#include <fstream>

#include <unordered_map>

namespace gorilla {

class Application {

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct vertex
	{
		glm::vec3 pos;
		glm::vec3 col;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription bindingDescription()
		{
			vk::VertexInputBindingDescription res(0, sizeof(vertex), vk::VertexInputRate::eVertex);

			return res;
		}

		static std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions()
		{
			std::array<vk::VertexInputAttributeDescription, 3> res{};

			res[0] = vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, pos));
			res[1] = vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, col));
			res[2] = vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(vertex, texCoord));

			return res;
		}
	};

	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	//        const std::string modelPath = "../Geometry/room.obj";
	const std::string modelPath = "C:/Users/Kiril/Desktop/Всё/прочее/r3ds/JacketTransferExample/Jacket_01_Decimated_750k2.obj";
	const std::string texturePath = "../Geometry/room.png";

	std::vector<vertex> verts;
	std::vector<uint32_t> indices;

	const int maxFrames = 2;
	bool frameBufferResized = false;

public:
    Application(std::string name, int w, int h);

    Application(const Application &other) = delete;
    Application& operator =(const Application &other) = delete;

    Application(Application &&other) = delete;
    Application& operator =(Application &&other) = delete;

    void run();

private:

	uint32_t instanceVersion;
    std::vector<vk::LayerProperties> layerProperties;
    std::vector<vk::ExtensionProperties> extensionProperties;

    std::vector<vk::QueueFamilyProperties> queueFamilies;
	std::vector<vk::LayerProperties> deviceLayerProperties;
	std::vector<vk::ExtensionProperties> deviceExtensionProperties;

	// Basic environment: graphics + present queues
	vk::raii::Context context;
	gorilla::window window;
	vk::raii::Instance instance = nullptr;
	vk::raii::SurfaceKHR surface = nullptr;
	vk::raii::PhysicalDevice physDevice = nullptr;
	vk::raii::Device device = nullptr;

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
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::PipelineLayout layout = nullptr;
    std::vector<vk::raii::Pipeline> pipelines;
    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::raii::CommandPool commandPool = nullptr;
    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
    vk::raii::Buffer indexBuffer = nullptr;
    vk::raii::DeviceMemory indexBufferMemory = nullptr;
    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    uint32_t mipLevels;
    vk::raii::Image texture = nullptr;
    vk::raii::DeviceMemory textureImageMemory = nullptr;
    vk::raii::ImageView textureView = nullptr;
    vk::raii::Sampler textureSampler = nullptr;

    vk::raii::Image depthImage = nullptr;
    vk::raii::DeviceMemory depthImageMemory = nullptr;
    vk::raii::ImageView depthImageView = nullptr;

    uint32_t currentFrame = 0;

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> isFlightFences;

    SwapChainSupportDetails querySupport(const vk::raii::PhysicalDevice& physicalDevice) const;
    vk::raii::PhysicalDevice pickDevice(const std::vector<vk::raii::PhysicalDevice> &devices) const;
    void pickBestFormat(const SwapChainSupportDetails& details, vk::Format& outFormat, vk::ColorSpaceKHR& outColorSpace) const;
    vk::PresentModeKHR pickPresentMode(const SwapChainSupportDetails& details) const;

    void recordCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t imgIndex) const;

    uint32_t findMemoryType(uint32_t filter, vk::MemoryPropertyFlags properties) const;

    static std::vector<char> readFile(const std::string& filename);

    vk::raii::ShaderModule createShader(const std::vector<char>& code);

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
    void createTextureImage(bool verbose = false);
    void createTextureImageView(bool verbose = false);
    void createTextureSampler(bool verbose = false);
    void loadModel(bool verbose = false);
    void createVertexBuffers(bool verbose = false);
    void createIndexBuffers(bool verbose = false);
    void createUniformBuffers(bool verbose = false);
    void createCommandBuffers(bool verbose = false);
    void createDescriptorPool(bool verbose = false);
    void createDescriptorSets(bool verbose = false);
    void createSyncObjects(bool verbose = false);

    void recreateSwapChain(bool verbose = false);

    void drawFrame();
};

} // gorilla

#endif //DEEPLOM_APPLICATION_HPP
