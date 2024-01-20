#include <iostream>

#include <application/application.hpp>


#include <asset_loader/loader.h>

#include <asset_loader/obj/obj.h>

#include <vk_utils/environment.hpp>


#include <obj/obj.h>
#include <img/image.h>

#include <application/applicationsimple.hpp>

std::vector<uint32_t> check_shader(const gorilla::shader_compiler::compiler::result_spirv_code& result)
{
	if (result.n_errors() > 0)
		std::cerr << result.error_message() << std::endl;

	return result.data();
}

int main()
{
	gorilla::pbr_pipeline::compile_shaders();

	gorilla::shader_compiler::compilation_options options;
	options.set_target_env(gorilla::shader_compiler::environment::vulkan);
	options.set_target_spirv(gorilla::shader_compiler::compiler::spv_version());
	gorilla::shader_compiler::compiler compiler;

	std::vector<uint32_t> vert, frag;
	// std::vector<uint32_t> vert = gorilla::asset_loader::load_shader("../Resources/Compiled Shaders/vert.spv");
	// std::vector<uint32_t> frag = gorilla::asset_loader::load_shader("../Resources/Compiled Shaders/frag.spv");
	// std::vector<uint32_t> vert = gorilla::asset_loader::compile_shader("../Resources/Shaders/shader.vert", compiler, options, gorilla::shader_compiler::shader_kind::vertex);
	// std::vector<uint32_t> frag = gorilla::asset_loader::compile_shader("../Resources/Shaders/shader.frag", compiler, options, gorilla::shader_compiler::shader_kind::fragment);
	// if (vert.empty() || frag.empty())
	// 	return -1;

	gorilla::engine::scene s;
	gorilla::engine::scene::prop_description prop;
	prop.obj_filename = "../Resources/Models/melon.obj";
	prop.texture = *gorilla::asset::image::load_image("../Resources/Textures/melon.png");
	prop.normal = *gorilla::asset::image::load_image("../Resources/Textures/melon_normal.png");
	prop.roughness = *gorilla::asset::image::load_image("../Resources/Textures/melon_roughness.png");
	prop.metallic = *gorilla::asset::image::load_image("../Resources/Textures/black.png");
	prop.ambient_occlusion = *gorilla::asset::image::load_image("../Resources/Textures/white.png");
	s.props.push_back(prop);

	s.lights.point_lights.n_lights = 1;
	auto& point_light = s.lights.point_lights.lights[0];
	point_light.position = glm::vec3(-3.0f, 0.0f, 5.0f);
	point_light.intensity = 10.0f;
	point_light.color = glm::vec3(1.0f, 1.0f, 1.0f);
	point_light.constant = 1.0f;
	point_light.linear = 0.0f;
	point_light.quadratic = 0.0f;

	auto& point_light1 = s.lights.point_lights.lights[1];
	point_light1.position = glm::vec3(-3.0f, 0.0f, 5.0f);
	point_light1.intensity = 10.0f;
	point_light1.color = glm::vec3(1.0f, 1.0f, 1.0f);
	point_light1.constant = 1.0f;
	point_light1.linear = 0.3f;
	point_light1.quadratic = 0.0f;

	s.lights.spot_lights.n_lights = 2;
	auto& spot_light = s.lights.spot_lights.lights[0];
	spot_light.position = glm::vec3(0.0f, 0.0f, -5.0f);
	spot_light.direction = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));
	spot_light.intensity = 10.0f;
	spot_light.color = glm::vec3(1.0f, 1.0f, 1.0f);
	spot_light.cone_size = 0.95f;
	spot_light.constant = 1.0f;
	spot_light.linear = 0.2f;
	spot_light.quadratic = 0.0f;

	auto& spot_light1 = s.lights.spot_lights.lights[1];
	spot_light1.position = glm::vec3(5.0f, 5.0f, -5.0f);
	spot_light1.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, 2.0f));
	spot_light1.intensity = 10.0f;
	spot_light1.color = glm::vec3(1.0f, 1.0f, 1.0f);
	spot_light1.cone_size = 0.95f;
	spot_light1.constant = 1.0f;
	spot_light1.linear = 0.2f;
	spot_light1.quadratic = 0.0f;

	std::unique_ptr<gorilla::Application> app;

    try {
	    app = std::make_unique<gorilla::Application>("Gorilla Application", 1920, 1080, vert, frag, s);

        app->run();
    } catch (const std::exception &e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "What the fuck" << std::endl;
    }

    return 0;
}
