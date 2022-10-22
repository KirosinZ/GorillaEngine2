#include <iostream>

#include <Application/application.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <3rdparty/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <3rdparty/tiny_obj_loader.h>

namespace gorilla {

    Application::Application(std::string name, int w, int h)
    : window(std::move(name), w, h)
    {
        initVulkan();
    }

    vk::raii::PhysicalDevice Application::pickDevice(const std::vector<vk::raii::PhysicalDevice>& devices) const
    {
        if (devices.empty())
            throw std::runtime_error("No GPUs present!");

        return devices[0];

        for (const auto& dev : devices)
        {

        }
    }

    Application::SwapChainSupportDetails Application::querySupport(const vk::raii::PhysicalDevice& physicalDevice) const
    {
        Application::SwapChainSupportDetails details{};

        details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        details.formats = physicalDevice.getSurfaceFormatsKHR(*surface);
        details.presentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

        return details;
    }

    void Application::pickBestFormat(const Application::SwapChainSupportDetails& details, vk::Format& outFormat, vk::ColorSpaceKHR& outColorSpace) const
    {
        outFormat = details.formats[1].format;
        outColorSpace = details.formats[1].colorSpace;
    }

    vk::PresentModeKHR Application::pickPresentMode(const Application::SwapChainSupportDetails& details) const
    {
        if (std::find(details.presentModes.begin(), details.presentModes.end(), vk::PresentModeKHR::eMailbox) != details.presentModes.end()) return vk::PresentModeKHR::eMailbox;

        return vk::PresentModeKHR::eFifo;
    }

    void Application::recordCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t imgIndex) const
    {
        vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
        buffer.begin(cbbi);

        std::array<float, 4> clr = { 0.0f, 0.0f, 0.0f, 1.0f };
        std::vector<vk::ClearValue> clearColors(2);
        clearColors[0].color = clr;
        clearColors[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
        vk::RenderPassBeginInfo rpbi(*renderPass, *framebuffers[imgIndex], {{0, 0}, scImageExtent }, clearColors);
        buffer.beginRenderPass(rpbi, vk::SubpassContents::eInline);
        {
            buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipelines[0]);
            vk::Viewport vp(0.0f, 0.0f, static_cast<float>(scImageExtent.width), static_cast<float>(scImageExtent.height), 0.0f, 1.0f);
            buffer.setViewport(0, vp);

            vk::Rect2D sc({ 0, 0 }, scImageExtent);
            buffer.setScissor(0, sc);

            std::vector<vk::DeviceSize> ds = { 0 };
            buffer.bindVertexBuffers(0, *vertexBuffer, ds);
            buffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint32);

            buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *layout, 0, *descriptorSets[currentFrame], {});

            buffer.drawIndexed(indices.size(), 1, 0, 0, 0);
        }
        buffer.endRenderPass();
        buffer.end();
    }

    void Application::establishContext(bool verbose) {
        layerProperties = context.enumerateInstanceLayerProperties();
        extensionProperties = context.enumerateInstanceExtensionProperties();
    }

    void Application::createInstance(bool verbose)
    {
        vk::ApplicationInfo ai(
                window.Name().c_str(),
                VK_MAKE_API_VERSION(1, 0, 1, 0),
                Engine::Name.c_str(),
                Engine::Version,
                Engine::VulkanVersion
        );

        const std::vector<const char*> layers {

        };

        std::vector<const char *> extensions = Window::requiredExtensions();

        vk::InstanceCreateInfo ici({}, &ai, layers, extensions);
        instance = vk::raii::Instance(context, ici);
    }

    void Application::createSurface(bool verbose)
    {
        surface = window.surface(instance);
    }

    void Application::createPhysDevice(bool verbose)
    {
        physDevice = pickDevice(instance.enumeratePhysicalDevices());
        queueFamilies = physDevice.getQueueFamilyProperties();

        for (int i = 0; i < queueFamilies.size(); ++i)
        {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                graphicsQueueFamilyIndex = i;

            if (physDevice.getSurfaceSupportKHR(i, *surface))
                presentQueueFamilyIndex = i;
        }
    }

    void Application::createLogicalDevice(bool verbose)
    {
        const std::array<float, 1>  priorities = { 1.0f };

        vk::DeviceQueueCreateInfo dqci({}, graphicsQueueFamilyIndex, priorities);
        vk::DeviceQueueCreateInfo dqci2({}, presentQueueFamilyIndex, priorities);

        std::vector<vk::DeviceQueueCreateInfo> dqcis = {
                dqci,
                dqci2
        };

        std::vector<const char *> deviceLayers {

        };

        std::vector<const char *> deviceExtensions {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        vk::PhysicalDeviceFeatures features{};
        features.samplerAnisotropy = true;

        vk::DeviceCreateInfo dci({}, dqcis, deviceLayers, deviceExtensions, &features);

        device = vk::raii::Device(physDevice, dci);
    }

    void Application::createQueue(bool verbose)
    {
        graphicsQueue = device.getQueue(graphicsQueueFamilyIndex, 0);
        presentQueue = device.getQueue(presentQueueFamilyIndex, 0);
    }

    void Application::createSwapChain(bool verbose)
    {
        Application::SwapChainSupportDetails details = querySupport(physDevice);
        vk::Format format;
        vk::ColorSpaceKHR colorSpace;
        pickBestFormat(details, format, colorSpace);
        vk::PresentModeKHR presentMode = pickPresentMode(details);

        vk::Extent2D extent = window.framebufferSize();
        vk::Extent2D actualExtent = extent;
        actualExtent.width = std::clamp(actualExtent.width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);

        std::array<uint32_t, 2> bl = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
        vk::SwapchainCreateInfoKHR scci(
                {},
                *surface,
                3,
                format,
                colorSpace,
                actualExtent,
                1,
                vk::ImageUsageFlagBits::eColorAttachment,
                vk::SharingMode::eConcurrent,
                bl,
                details.capabilities.currentTransform,
                //vk::SurfaceTransformFlagBitsKHR::eIdentity,
                vk::CompositeAlphaFlagBitsKHR::eOpaque,
                presentMode,
                true);
        swapchain = vk::raii::SwapchainKHR(device, scci);
        auto imgs = swapchain.getImages();
        scImages.clear();
        for (auto img : imgs)
        {
            scImages.emplace_back(img);
        }
        scImageExtent = actualExtent;
        scImageFormat = format;
    }

    void Application::createImageViews(bool verbose)
    {
        imageViews.clear();
        imageViews.reserve(scImages.size());
        for (auto & scImage : scImages)
        {
            vk::ImageViewCreateInfo ivci({}, scImage, vk::ImageViewType::e2D, scImageFormat, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            imageViews.emplace_back(device, ivci);
        }
    }

    void Application::createRenderPass(bool verbose)
    {
        vk::SubpassDependency sd(
                VK_SUBPASS_EXTERNAL,
                0,
                vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                {},
                vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        vk::AttachmentDescription colorAttachment(
                {},
                scImageFormat,
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentDescription depthAttachment(
                {},
                findDepthFormat(),
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eDontCare,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentReference ref(
                0,
                vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentReference dref(
                1,
                vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpass(
                {},
                vk::PipelineBindPoint::eGraphics,
                {},
                ref,
                {},
                &dref);

        std::vector<vk::AttachmentDescription> attachments = {colorAttachment, depthAttachment};

        vk::RenderPassCreateInfo rpci(
                {},
                attachments,
                subpass,
                sd);

        renderPass = device.createRenderPass(rpci);
    }

    void Application::createDescriptorSetLayout(bool verbose)
    {
        std::vector<vk::DescriptorSetLayoutBinding> dslbs = {
                {0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                {1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}
        };

        vk::DescriptorSetLayoutCreateInfo dslci({}, dslbs);
        descriptorSetLayout = device.createDescriptorSetLayout(dslci);
    }

    void Application::createGraphicsPipeline(bool verbose)
    {
        auto vertCode = readFile("../Shaders/vert.spv");
        auto fragCode = readFile("../Shaders/frag.spv");

        vk::raii::ShaderModule vert = createShader(vertCode);
        vk::raii::ShaderModule frag = createShader(fragCode);

        vk::PipelineShaderStageCreateInfo pssci_v({}, vk::ShaderStageFlagBits::eVertex, *vert, "main");
        vk::PipelineShaderStageCreateInfo pssci_f({}, vk::ShaderStageFlagBits::eFragment, *frag, "main");

        std::vector<vk::PipelineShaderStageCreateInfo> stages = { pssci_v, pssci_f };

        std::vector<vk::DynamicState> dynStates = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor,
        };

        vk::PipelineDynamicStateCreateInfo pdsci({}, dynStates);

        auto binding = vertex::bindingDescription();
        auto attributes = vertex::attributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo pvisci({}, binding, attributes);

        vk::PipelineInputAssemblyStateCreateInfo piaci({}, vk::PrimitiveTopology::eTriangleList, false);

        vk::PipelineViewportStateCreateInfo viewportState({}, 1, {}, 1, {});

        vk::PipelineRasterizationStateCreateInfo resterizer(
                {},
                false,
                false,
                vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eNone,
                vk::FrontFace::eCounterClockwise,
                false, 0.0f, 0.0f, 0.0f, 1.0f);

        vk::PipelineDepthStencilStateCreateInfo pdssci(
                {},
                true, true, vk::CompareOp::eLess, false,
                false);

        vk::PipelineMultisampleStateCreateInfo pmci({}, vk::SampleCountFlagBits::e1, false);

        vk::PipelineColorBlendAttachmentState colorBlendAttachment(false);
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eR;

        vk::PipelineColorBlendStateCreateInfo cbsci(
                {},
                false,
                vk::LogicOp::eCopy,
                colorBlendAttachment,
                { 0.0f, 0.0f, 0.0f, 0.0f });

        vk::PipelineLayoutCreateInfo plci({}, *descriptorSetLayout, {});

        layout = device.createPipelineLayout(plci);

        vk::GraphicsPipelineCreateInfo gpci(
                {},
                stages,
                &pvisci,
                &piaci,
                {},
                &viewportState,
                &resterizer,
                &pmci,
                &pdssci,
                &cbsci,
                &pdsci,
                *layout,
                *renderPass,
                0);

        pipelines = device.createGraphicsPipelines(nullptr, gpci);
    }

    void Application::createFramebuffers(bool verbose)
    {
        framebuffers.clear();
        framebuffers.reserve(imageViews.size());
        for (auto & imageView : imageViews)
        {
            vk::ImageView attachments[] = {
                    *imageView,
                    *depthImageView,
            };

            vk::FramebufferCreateInfo fci({}, *renderPass, attachments, scImageExtent.width, scImageExtent.height, 1);
            framebuffers.push_back(device.createFramebuffer(fci));
        }
    }

    void Application::createCommandPool(bool verbose)
    {
        vk::CommandPoolCreateInfo cpci(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsQueueFamilyIndex);

        commandPool = device.createCommandPool(cpci);
    }

    void Application::createDepthResources(bool verbose)
    {
        vk::Format format = findDepthFormat();
        auto&& images = createImage(scImageExtent.width, scImageExtent.height, 1, format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
        depthImage = std::move(images.first);
        depthImageMemory = std::move(images.second);

        vk::ImageViewCreateInfo ivci(
                {},
                *depthImage,
                vk::ImageViewType::e2D,
                format,
                {},
                {
                    vk::ImageAspectFlagBits::eDepth,
                    0,
                    1,
                    0,
                    1});
        depthImageView = device.createImageView(ivci);

        transitionImageLayout(*depthImage, format, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
    }

    void Application::createTextureImage(bool verbose)
    {
        int texW, texH, texChannels;
        stbi_uc* pixels = stbi_load(texturePath.c_str(), &texW, &texH, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = texW * texH * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texW, texH)))) + 1;

        if (!pixels)
            throw std::runtime_error("failed to load image");

        auto&&[stagingBuffer, stagingBufferMemory] = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        std::memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        stbi_image_free(pixels);

        auto &&tmp = createImage(texW, texH, mipLevels, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);
        texture = std::move(tmp.first);
        textureImageMemory = std::move(tmp.second);

        transitionImageLayout(*texture, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(*stagingBuffer, *texture, texW, texH);
        //transitionImageLayout(*texture, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, mipLevels);
        generateMipmaps(*texture, vk::Format::eR8G8B8A8Srgb, texW, texH, mipLevels);
    }

    void Application::createTextureImageView(bool verbose)
    {
        vk::ImageViewCreateInfo ivci(
                {},
                *texture,
                vk::ImageViewType::e2D,
                vk::Format::eR8G8B8A8Srgb,
                vk::ComponentMapping(),
                vk::ImageSubresourceRange(
                        vk::ImageAspectFlagBits::eColor,
                        0,
                        mipLevels,
                        0,
                        1));
        textureView  = { device, ivci };
    }

    void Application::createTextureSampler(bool verbose)
    {
        vk::PhysicalDeviceProperties properties = physDevice.getProperties();

        vk::SamplerCreateInfo sci(
                {},
                vk::Filter::eLinear, vk::Filter::eLinear,
                vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat,
                vk::SamplerAddressMode::eRepeat,
                vk::SamplerAddressMode::eRepeat,
                0.0f,
                true,
                properties.limits.maxSamplerAnisotropy,
                false, vk::CompareOp::eAlways, 0.0f, mipLevels,
                vk::BorderColor::eIntOpaqueBlack,
                false);

        textureSampler = device.createSampler(sci);
    }

    void Application::loadModel(bool verbose)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
            throw std::runtime_error(warn + err);

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                vertex vertex{};

                vertex.pos = {
                        attrib.vertices[3 * index.vertex_index],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                };


                vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };

                vertex.col = {1.0f, 1.0f, 1.0f};

                verts.push_back(vertex);
                indices.push_back(indices.size());
            }
        }
    }

    void Application::createVertexBuffers(bool verbose)
    {
        const vk::DeviceSize size = sizeof(verts[0]) * verts.size();

        auto&&[stagingBuffer, stagingBufferMemory] = createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        void* data = stagingBufferMemory.mapMemory(0, size);
        std::memcpy(data, verts.data(), size);
        stagingBufferMemory.unmapMemory();

        auto &&buffers = createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
        vertexBuffer = std::move(buffers.first);
        vertexBufferMemory = std::move(buffers.second);

        copyBuffer(stagingBuffer, vertexBuffer, size);
    }

    void Application::createIndexBuffers(bool verbose)
    {
        const vk::DeviceSize size = sizeof(indices[0]) * indices.size();

        auto&&[stagingBuffer, stagingBufferMemory] = createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        void* data = stagingBufferMemory.mapMemory(0, size);
        std::memcpy(data, indices.data(), size);
        stagingBufferMemory.unmapMemory();

        auto &&buffers = createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
        indexBuffer = std::move(buffers.first);
        indexBufferMemory = std::move(buffers.second);

        copyBuffer(stagingBuffer, indexBuffer, size);
    }

    void Application::createUniformBuffers(bool verbose)
    {
        const vk::DeviceSize size = sizeof(UniformBufferObject);

        uniformBuffers.reserve(maxFrames);
        uniformBuffersMemory.reserve(maxFrames);

        for (int i = 0; i < maxFrames; i++)
        {
            auto&& buffers = createBuffer(size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            uniformBuffers.push_back(std::move(buffers.first));
            uniformBuffersMemory.push_back(std::move(buffers.second));
        }
    }

    void Application::createDescriptorPool(bool verbose)
    {
        std::vector<vk::DescriptorPoolSize> poolSizes = {
                {vk::DescriptorType::eUniformBuffer, (uint32_t)maxFrames},
                {vk::DescriptorType::eCombinedImageSampler, (uint32_t)maxFrames}
        };

        vk::DescriptorPoolCreateInfo dpci(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxFrames, poolSizes);

        descriptorPool = device.createDescriptorPool(dpci);
    }

    void Application::createDescriptorSets(bool verbose)
    {
        std::vector<vk::DescriptorSetLayout> layouts(maxFrames, *descriptorSetLayout);
        vk::DescriptorSetAllocateInfo dsai(*descriptorPool, layouts);

        descriptorSets = device.allocateDescriptorSets(dsai);
        for (int i = 0; i < maxFrames; i++)
        {
            vk::DescriptorBufferInfo dbi(*uniformBuffers[i], 0, sizeof(UniformBufferObject));
            vk::DescriptorImageInfo dii(*textureSampler, *textureView, vk::ImageLayout::eShaderReadOnlyOptimal);
            std::vector<vk::WriteDescriptorSet> descriptorWrites = {
                    {*descriptorSets[i], 0, 0, vk::DescriptorType::eUniformBuffer, {}, dbi},
                    {*descriptorSets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, dii}
            };

            device.updateDescriptorSets(descriptorWrites, {});
        }
    }

    void Application::createCommandBuffers(bool verbose)
    {
        vk::CommandBufferAllocateInfo cbai(*commandPool, vk::CommandBufferLevel::ePrimary, maxFrames);
        commandBuffers = device.allocateCommandBuffers(cbai);
    }

    void Application::createSyncObjects(bool verbose)
    {
        vk::SemaphoreCreateInfo sci;

        vk::FenceCreateInfo fci(vk::FenceCreateFlagBits::eSignaled);

        for (int i = 0; i < maxFrames; i++)
        {
            imageAvailableSemaphores.push_back(device.createSemaphore(sci));
            renderFinishedSemaphores.push_back(device.createSemaphore(sci));
            isFlightFences.push_back(device.createFence(fci));

        }
    }

    void Application::recreateSwapChain(bool verbose)
    {
        vk::Extent2D ext = window.framebufferSize();
        while (ext.height == 0 || ext.width == 0)
        {
            ext = window.framebufferSize();
            Window::waitEvents();
        }

        device.waitIdle();

        framebuffers.clear();
        imageViews.clear();
        swapchain = nullptr;

        createSwapChain();
        createImageViews();
        createDepthResources();
        createFramebuffers();
    }

    void Application::initVulkan() {
        establishContext();
        createInstance();
        createSurface();
        createPhysDevice();
        createLogicalDevice();
        createQueue();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();
        createDepthResources();
        createFramebuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        loadModel();
        createVertexBuffers();
        createIndexBuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    void Application::drawFrame()
    {
        vk::Result result = device.waitForFences(*isFlightFences[currentFrame], true, UINT64_MAX);
        resultCheck(result, "Fence checker");

        uint32_t imgIndex;
        vk::AcquireNextImageInfoKHR anii(*swapchain, UINT64_MAX, *imageAvailableSemaphores[currentFrame], {}, 1);
        auto tmp = device.acquireNextImage2KHR(anii);
        result = tmp.first;
        if (result == vk::Result::eErrorOutOfDateKHR || frameBufferResized)
        {
            frameBufferResized = false;
            recreateSwapChain();
            return;
        }
        device.resetFences(*isFlightFences[currentFrame]);
        imgIndex = tmp.second;

        commandBuffers[currentFrame].reset();
        recordCommandBuffer(commandBuffers[currentFrame], imgIndex);

        std::array<vk::PipelineStageFlags, 1> waitStages{
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
        };
        vk::SubmitInfo si(*imageAvailableSemaphores[currentFrame], waitStages, *commandBuffers[currentFrame], *renderFinishedSemaphores[currentFrame]);

        updateUniformBuffer(currentFrame);

        graphicsQueue.submit(si, *isFlightFences[currentFrame]);

        vk::PresentInfoKHR pi(*renderFinishedSemaphores[currentFrame], *swapchain, imgIndex);

        try
        {
            presentQueue.presentKHR(pi);
            currentFrame = (currentFrame + 1) % maxFrames;
        }
        catch (...)
        {
            frameBufferResized = false;
            recreateSwapChain();
            currentFrame = (currentFrame + 1) % maxFrames;
        }

        //currentFrame = (currentFrame + 1) % maxFrames;
    }

    void Application::run() {
        //window.setShouldClose(true);
        while (!window.shouldClose()) {
            Window::pollEvents();
            drawFrame();
        }

        device.waitIdle();
    }
} // gorilla