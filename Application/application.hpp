#ifndef DEEPLOM_APPLICATION_HPP
#define DEEPLOM_APPLICATION_HPP

#include <Engine/engine.hpp>
#include <Window/window.hpp>
#include <fstream>

namespace gorilla {

    class Application {
    public:
        Application(std::string name, int w, int h);

        Application(const Application &other) = delete;
        Application& operator =(const Application &other) = delete;

        Application(Application &&other) = delete;
        Application& operator =(Application &&other) = delete;

        void run();

    private:
        const std::string modelPath = "../Geometry/room.obj";
        const std::string texturePath = "../Geometry/room.png";

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

        std::vector<vertex> verts;
        std::vector<uint32_t> indices;

        const int maxFrames = 2;
        bool frameBufferResized = false;

        std::vector<vk::LayerProperties> layerProperties;
        std::vector<vk::ExtensionProperties> extensionProperties;

        std::vector<vk::QueueFamilyProperties> queueFamilies;



        gorilla::Window window;
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::SurfaceKHR surface = nullptr;
        vk::raii::PhysicalDevice physDevice = nullptr;
        uint32_t graphicsQueueFamilyIndex = 0;
        uint32_t presentQueueFamilyIndex = 0;
        vk::raii::Device device = nullptr;
        vk::raii::Queue graphicsQueue = nullptr;
        vk::raii::Queue presentQueue = nullptr;
        vk::raii::SwapchainKHR swapchain = nullptr;
        std::vector<vk::Image> scImages;
        vk::Format scImageFormat;
        vk::Extent2D scImageExtent;
        std::vector<vk::raii::ImageView> imageViews;
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

        uint32_t findMemoryType(uint32_t filter, vk::MemoryPropertyFlags properties) const
        {
            vk::PhysicalDeviceMemoryProperties mp = physDevice.getMemoryProperties();
            for (uint32_t i = 0; i < mp.memoryTypeCount; i++)
            {
                if (filter & (1 << i) && (mp.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;
            }

            throw std::runtime_error("Nu suitable memory type found!");
        }

        static std::vector<char> readFile(const std::string& filename)
        {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open())
            {
                throw std::exception("Cannot read file");
            }

            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            file.close();

            return buffer;
        }

        vk::raii::ShaderModule createShader(const std::vector<char>& code)
        {
            vk::ShaderModuleCreateInfo smci({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));

            return device.createShaderModule(smci);
        }

        std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
        {
            vk::raii::Buffer resBuf = nullptr;
            vk::raii::DeviceMemory resBufMem = nullptr;
            vk::BufferCreateInfo bci({}, size, usage, vk::SharingMode::eExclusive);
            resBuf = vk::raii::Buffer(device, bci);

            vk::MemoryRequirements mr = resBuf.getMemoryRequirements();

            vk::MemoryAllocateInfo mai(mr.size, findMemoryType(mr.memoryTypeBits, properties));
            resBufMem = device.allocateMemory(mai);

            resBuf.bindMemory(*resBufMem, 0);

            return { std::move(resBuf), std::move(resBufMem) };
        }

        void copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
        {
            vk::raii::CommandBuffer cb = beginSingleTimeCommands();

            vk::BufferCopy bc(0, 0, size);
            cb.copyBuffer(*srcBuffer, *dstBuffer, bc);

            endSingleTimeCommands(*cb);
        }

        void updateUniformBuffer(uint32_t currentImage)
        {
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();

            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            UniformBufferObject ubo{};
            ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), scImageExtent.width / float(scImageExtent.height), 0.1f, 10.0f);
            ubo.proj[1][1] *= -1;

            void* data = uniformBuffersMemory[currentImage].mapMemory(0, sizeof(ubo));
            std::memcpy(data, &ubo, sizeof(ubo));
            uniformBuffersMemory[currentImage].unmapMemory();
        }

        std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, uint32_t height, uint32_t  mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
        {
            vk::raii::Image img = nullptr;
            vk::raii::DeviceMemory mem = nullptr;

            vk::ImageCreateInfo ici(
                    {},
                    vk::ImageType::e2D,
                    format,
                    { width, height, 1 },
                    mipLevels,
                    1,
                    vk::SampleCountFlagBits::e1,
                    tiling,
                    usage,
                    vk::SharingMode::eExclusive,
                    {},
                    vk::ImageLayout::eUndefined);
            img = device.createImage(ici);

            vk::DeviceImageMemoryRequirements dimr(&ici);
            vk::MemoryRequirements2 memRequirements = device.getImageMemoryRequirements(dimr);

            vk::MemoryAllocateInfo allocInfo(memRequirements.memoryRequirements.size, findMemoryType(memRequirements.memoryRequirements.memoryTypeBits, properties));
            mem = device.allocateMemory(allocInfo);

            vk::BindImageMemoryInfo bimi(*img, *mem, 0);
            device.bindImageMemory2(bimi);

            return { std::move(img), std::move(mem) };
        }

        vk::raii::CommandBuffer beginSingleTimeCommands()
        {
            vk::CommandBufferAllocateInfo cbai(*commandPool, vk::CommandBufferLevel::ePrimary, 1);

            vk::raii::CommandBuffer cb = std::move(device.allocateCommandBuffers(cbai)[0]);

            vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            cb.begin(cbbi);

            return cb;
        }

        void endSingleTimeCommands(vk::CommandBuffer buffer)
        {
            buffer.end();

            vk::SubmitInfo si({}, {}, buffer);
            graphicsQueue.submit(si);
            graphicsQueue.waitIdle();
        }

        void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
        {
            vk::raii::CommandBuffer cb = beginSingleTimeCommands();

            vk::ImageMemoryBarrier barrier({}, {}, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED,
                                           VK_QUEUE_FAMILY_IGNORED, image,
                                           {vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1});

            if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
            {
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

                if (hasStencilComponent(format))
                    barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
            } else
            {
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            }

            vk::PipelineStageFlags srcStage;
            vk::PipelineStageFlags dstStage;
            if (oldLayout == vk::ImageLayout::eUndefined
             && newLayout == vk::ImageLayout::eTransferDstOptimal)
            {
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

                srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
                dstStage = vk::PipelineStageFlagBits::eTransfer;
            } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
                    && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
            {
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                srcStage = vk::PipelineStageFlagBits::eTransfer;
                dstStage = vk::PipelineStageFlagBits::eFragmentShader;
            } else if (oldLayout == vk::ImageLayout::eUndefined
                    && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
            {
                barrier.srcAccessMask = vk::AccessFlagBits::eNone;
                barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

                srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
                dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
            } else {
                throw std::invalid_argument("unsupported layout transition!");
            }

            cb.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);

            endSingleTimeCommands(*cb);
        }

        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
        {
            vk::raii::CommandBuffer cb = beginSingleTimeCommands();

            vk::BufferImageCopy bic(0, 0, 0, { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, { 0, 0, 0 }, { width, height, 1});

            cb.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, bic);

            endSingleTimeCommands(*cb);
        }

        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling  tiling, vk::FormatFeatureFlags features)
        {
            for (vk::Format format : candidates)
            {
                vk::FormatProperties props = physDevice.getFormatProperties(format);

                if(tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features
                || tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
                    return format;
            }

            throw std::runtime_error("failed to find supported format!");
        }

        vk::Format findDepthFormat()
        {
            return findSupportedFormat(
                    { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
                    vk::ImageTiling::eOptimal,
                    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }

        bool hasStencilComponent(vk::Format format)
        {
            return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
        }

        void generateMipmaps(vk::Image img, vk::Format imageFormat, int32_t texW, int32_t texH, uint32_t mipLevels)
        {
            vk::FormatProperties props = physDevice.getFormatProperties(imageFormat);
            if (!(props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
                throw std::runtime_error("texture image format does not support linear blitting!");

            vk::raii::CommandBuffer buffer = beginSingleTimeCommands();

            vk::ImageMemoryBarrier barrier({}, {}, {}, {}, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, img, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            int32_t mipW = texW;
            int32_t mipH = texH;

            for (uint32_t i = 1; i < mipLevels; i++)
            {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

                buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

                std::array<vk::Offset3D, 2> srcOffsets = {
                        vk::Offset3D(0, 0, 0),
                        vk::Offset3D(mipW, mipH, 1),
                };
                std::array<vk::Offset3D, 2> dstOffsets = {
                        vk::Offset3D(0, 0, 0),
                        vk::Offset3D(mipW > 1 ? mipW / 2 : 1, mipH > 1 ? mipH / 2 : 1, 1),
                };
                vk::ImageBlit blit(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1), srcOffsets, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1), dstOffsets);
                buffer.blitImage(img, vk::ImageLayout::eTransferSrcOptimal, img, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

                barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

                if (mipW > 1) mipW /= 2;
                if (mipH > 1) mipH /= 2;

            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

            endSingleTimeCommands(*buffer);
        }

        void initVulkan();

        void establishContext(bool verbose = false);
        void createInstance(bool verbose = false);
        void createSurface(bool verbose = false);
        void createPhysDevice(bool verbose = false);
        void createLogicalDevice(bool verbose = false);
        void createQueue(bool verbose = false);
        void createSwapChain(bool verbose = false);
        void createImageViews(bool verbose = false);
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
