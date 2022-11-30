#include <iostream>
#include <stdexcept>

#include <Application/application.hpp>
#include <Geometry/raw_obj.h>


int main()
{
    gorilla::Application app("Gorilla Application", 1920, 1080);

    gorilla::geometry::raw_obj ro("../base.obj", true, true, true, false, false, false, false, false);

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "What the fuck" << std::endl;
    }

    return 0;
}
