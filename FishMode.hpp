//based on https://github.com/15-466/15-466-f20-base2/blob/master/PlayMode.hpp

#include "Mode.hpp"

#include "Scene.hpp"
#include "GL.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct FishMode : Mode {
	FishMode();
	virtual ~FishMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	
	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	GLenum fish_vertex_type = GL_TRIANGLES; 
	GLuint fish_vertex_start = 0; 
	GLuint fish_vertex_count = 0; 

	struct Fish {
		float hang_time = 2.0f; 
		int add_score = 0; 
		bool throwing = false; 
		bool retreating = false; 
		Scene::Transform *transform; 
		float rotation_speed = 3.0f; 
		glm::vec3 destination =  glm::vec3(0.0f, 0.0f, 0.0f); 
		glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f); 
	};

	float rise_speed = 10.0f; 
	float throw_speed = 70.0f; 

	float sunk_depth = -2.0f; 

	float collide_length_throw = .2f; 
	float collide_length_fish = 1.0f; 

	int score = 5; 
	int cumulative_num_fish = 0; 

	std::vector<Fish> fishes;
	size_t max_num_fish = 4; 
	size_t num_fish = 0; 
	float reappear_time = 1.0f; 
	float float_time = 1.0f; 

	float add_elapsed = 0.0f; 


	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *fish = nullptr;
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
