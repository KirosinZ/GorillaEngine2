#include <iostream>
#include <stdexcept>

#include <Application/application.hpp>


int main()
{
//    gorilla::Application* app = new MyApplication("Gorilla Application", 1920, 1080);
	gorilla::Application* app = new gorilla::Application("Gorilla Application", 1920, 1080);

    try {
        app->run();
    } catch (const std::exception &e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "What the fuck" << std::endl;
    }

	delete app;

    return 0;
}
