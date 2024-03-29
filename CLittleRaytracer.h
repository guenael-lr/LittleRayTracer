#pragma once
#include "Utils.h"
#include "Camera.h"
#include "Object.h"
#include "PostProcessEffect.h"


class LittleRaytracer
{
public:
	LittleRaytracer(glm::ivec2 p_outputRes);
	~LittleRaytracer();

	void run();

protected:
	int init();
	void updatePixelOnScreen(int p_x, int p_y, glm::vec3 p_rgb) const;
	glm::vec3 getPixelColor(glm::ivec2 p_pixel);
	glm::vec3 raytrace(glm::vec3 p_origin, glm::vec3 p_dir, int p_depth);
	float applyDirectLighting(glm::vec3 p_posLight, glm::vec3 p_pointPosition, glm::vec3 p_normal, glm::vec3 p_eyeDir) const;
	
	bool m_running;

	bool m_withEffect;

	SDL_Window* m_window;
	SDL_Renderer* m_renderer;

	glm::ivec2 m_resolution;


	Camera* m_camera;
	std::vector<Object*> m_colliders;

	glm::vec3 * m_pixelsAcc;
	glm::vec3 * m_postProcessedPixels;
	int m_numFrame;

	unsigned int m_nbThreads;
	std::vector<std::thread*> m_threads;
	std::mutex m_mutex;
	std::vector<PostProcessEffect*> m_postProcessEffects;
};

