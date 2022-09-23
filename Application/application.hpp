#ifndef DEEPLOM_APPLICATION_HPP
#define DEEPLOM_APPLICATION_HPP

#include <Window/window.hpp>
#include <Engine/engine.hpp>

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
        gorilla::Window window;
        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        std::vector<vk::raii::PhysicalDevice> physDevices;
        vk::raii::Device device = nullptr;

        void initVulkan();

        void establishContext(bool verbose = false);
        void createInstance(bool verbose = false);
        void pickPhysicalDevice(bool verbose = false);
        void createLogicalDevice(bool verbose = false);
    };

} // gorilla

#endif //DEEPLOM_APPLICATION_HPP
