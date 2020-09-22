//based on https://github.com/15-466/15-466-f20-base2/blob/master/PlayMode.cpp

#include "FishMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <ctime>

GLuint pond_meshes_for_lit_color_texture_program = 0;

Load< MeshBuffer > pond_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("pond.pnct"));
	pond_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});


Load< Scene > pond_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("pond.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = pond_meshes->lookup(mesh_name);
		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = pond_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

FishMode::FishMode() : scene(*pond_scene) {
	// code from https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run Robert Mason
    srand((unsigned int)time(NULL));

	for (auto &drawable : scene.drawables) {
		if (drawable.transform->name == "Fish") {
			fish = (drawable.transform);
			fish_vertex_type = drawable.pipeline.type; 
			fish_vertex_start = drawable.pipeline.start; 
			fish_vertex_count = drawable.pipeline.count; 
		}
	}
	if (fish == nullptr) throw std::runtime_error("No fish transform found.");
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	std::cout << "\nhit some fish!"; 

}

FishMode::~FishMode() {
}

bool FishMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_MOUSEBUTTONDOWN) {
		//throw the fish 
		if(score > 0) {
			score --; 
			Fish f; 
			f.transform = new Scene::Transform; 
			f.throwing = true; 
			f.rotation_speed = 20.0f; 
			f.axis = glm::normalize(glm::vec3((rand() % 10), (rand() % 10), (rand() % 10))); 

			f.transform->position = camera->transform->position; 
			f.transform->name = "Fish" + std::to_string(cumulative_num_fish); 

			//raycast to plane code + guide from this website https://antongerdelan.net/opengl/raycasting.html

			//mouse to clip space taken from game0 code https://github.com/15-466/15-466-f20-base0/blob/master/PongMode.cpp  
			//convert mouse from window pixels (top-left origin, +y is down) to clip space ([-1,1]x[-1,1], +y is up):
			glm::vec2 clip_mouse = glm::vec2(
				(evt.motion.x + 0.5f) / window_size.x * 2.0f - 1.0f,
				(evt.motion.y + 0.5f) / window_size.y *-2.0f + 1.0f
			);
			glm::vec4 ray_clip = glm::vec4(clip_mouse.x,clip_mouse.y, 1.0,  1.0);
			glm::vec4 ray_eye = glm::inverse(camera->make_projection()) * ray_clip;
			ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0,  0.0); 
			glm::vec3 ray_wor = glm::vec3(glm::mat4(camera->transform->make_local_to_world()) * ray_eye);
			ray_wor = glm::normalize(ray_wor);

			//find intersection with the z=sunk_depth plane
			float t = (sunk_depth - f.transform->position.z) / ray_wor.z; 
 			f.destination.z = sunk_depth; 
			f.destination.x = t * ray_wor.x + f.transform->position.x; 
			f.destination.y = t * ray_wor.y + f.transform->position.y; 

			cumulative_num_fish++; 
			scene.drawables.emplace_back(f.transform);

			Scene::Drawable &drawable = scene.drawables.back();
			drawable.pipeline = lit_color_texture_program_pipeline;
			drawable.pipeline.vao = pond_meshes_for_lit_color_texture_program;
			drawable.pipeline.type = fish_vertex_type;
			drawable.pipeline.start = fish_vertex_start;
			drawable.pipeline.count = fish_vertex_count;
			fishes.push_back(f); 			
		}
		else {
			std::cout << "\nyou are out of fish to throw :("; 
		}
		return true; 
	} 

	return false;
}

void FishMode::update(float elapsed) {
	//spawn new fish 
	if(num_fish < max_num_fish) {
		add_elapsed += elapsed; 

		//only do it it after reappear_time seconds
		if(add_elapsed > reappear_time) {
			add_elapsed = 0.0f; 
			Fish f; 
			f.transform = new Scene::Transform; 
			float radius = float(rand() % 7); 
			float angle = .628f * (rand() % 10);
			f.throwing = false; 
			//make fish hang around less if your score is high 
			if(score > 10) f.hang_time = 1.5f; 
			if(score > 20) f.hang_time = 1.0f; 
			if(score > 30) f.hang_time = .7f; 

			//to do: make sure that it does not collide with other fish in the pond 
			f.transform->position.x = radius * cos(angle); 
			f.transform->position.y = radius * sin(angle); 
			f.transform->position.z = sunk_depth; 
			f.transform->name = "Fish" + std::to_string(cumulative_num_fish); 
			cumulative_num_fish++; 
			scene.drawables.emplace_back(f.transform);

			Scene::Drawable &drawable = scene.drawables.back();
			drawable.pipeline = lit_color_texture_program_pipeline;
			drawable.pipeline.vao = pond_meshes_for_lit_color_texture_program;
			drawable.pipeline.type = fish_vertex_type;
			drawable.pipeline.start = fish_vertex_start;
			drawable.pipeline.count = fish_vertex_count;

			num_fish ++; 
			fishes.push_back(f); 
			
			}
	}

	for (std::vector<Fish>::iterator f = std::begin(fishes); f != std::end(fishes); ++f) {
		//everyone is always rotating :D 
		f->transform->rotation *=  glm::angleAxis(f->rotation_speed * elapsed, f->axis);

		//throw your throwing fish 
		if(f->throwing) {
			glm::vec3 direction = f->destination - f->transform->position; 
			//if the fish has reached its destination, delete it
			if(glm::length(direction) < collide_length_throw) {
				std::list<Scene::Drawable>::iterator drawable = scene.drawables.begin(); 
				while(drawable != scene.drawables.end()) {
					if(drawable->transform == f->transform) {
						scene.drawables.erase(drawable); 
						break;
					}
					else ++drawable; 
				}
				score += f->add_score; 
				fishes.erase(f); 
				f--; 
				continue; 
			}
			//otherwise move towards destination
			direction =  glm::normalize(direction); 
			f->transform->position += direction * throw_speed * elapsed; 
			//check if you hit any fish 
			for (std::vector<Fish>::iterator f2 = std::begin(fishes); f2 != std::end(fishes); ++f2) {
				if(!(f2->throwing)) {
					if(glm::length(f->transform->position - f2->transform->position) < collide_length_fish) {
						f2->add_score = 1; 
						f->add_score = 1; 
						f2->throwing = true;
						f2->rotation_speed = 20.0f; 
						f2->axis = glm::normalize(glm::vec3((rand() % 10), (rand() % 10), (rand() % 10))); 
						//they bounce off the screen
						float radius = 20.0f; 
						float angle = .628f * (rand() % 10);
						f->destination = glm::vec3(radius * cos(angle), radius * sin(angle), 2.0f); 
						angle = .628f * (rand() % 10);
						f2->destination = glm::vec3(radius * cos(angle), radius * sin(angle), 2.0f); 
						num_fish--; 
					}
				}
			}
		}

		//animate the non-thrown fish
		else {
			if(f->retreating) {
				f->transform->position.z -= elapsed * rise_speed; 
				if(f->transform->position.z < sunk_depth){
					std::list<Scene::Drawable>::iterator drawable = scene.drawables.begin(); 
					while(drawable != scene.drawables.end()) {
						if(drawable->transform == f->transform) {
							scene.drawables.erase(drawable); 
							break;
						}
						else ++drawable; 
					}
					score += f->add_score; 
					fishes.erase(f); 
					f--; 
					num_fish--; 
				}

			}
			else {
				if(f->transform->position.z < 0.0f) f->transform->position.z += elapsed * rise_speed; 
				f->hang_time -= elapsed; 
				//fish will retreat 
				if(f->hang_time <= 0.0) {
					f->retreating = true; 
				}
			}
		}
	}
	
}

void FishMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.2f, 1.2f, 1.2f)));
	GL_ERRORS();
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("fish: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("fish: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
