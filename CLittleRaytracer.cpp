#include "Utils.h"
#include "CLittleRaytracer.h"

#include <algorithm>

#include "Sphere.h"
#include "ObjMesh.h"


LittleRaytracer::LittleRaytracer(glm::ivec2 p_outputRes) :
	m_running(false),
	m_window(NULL),
	m_renderer(NULL),
	m_resolution(p_outputRes),
	m_camera(NULL),
	m_pixelsAcc(NULL),
	m_numFrame(1),
	m_postProcessEffects({}),
	m_threads({})
{
}


LittleRaytracer::~LittleRaytracer()
{
	delete[] m_pixelsAcc;

	delete m_camera;

	for (const auto& collider : m_colliders)
		delete collider;
	
	for(const PostProcessEffect* effect : m_postProcessEffects)
		delete effect;
	m_postProcessEffects.clear();
	
	if(m_renderer)
		SDL_DestroyRenderer(m_renderer);
	if(m_window)
		SDL_DestroyWindow(m_window);
	SDL_Quit();
}


int LittleRaytracer::init()
{
	std::srand(static_cast<unsigned int>(time(nullptr)));


	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

	SDL_CreateWindowAndRenderer(m_resolution.x, m_resolution.y, 0, &m_window, &m_renderer);
	SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 0);
	SDL_RenderClear(m_renderer);
	SDL_RenderPresent(m_renderer);


	m_camera = new Camera(1.0f, glm::vec2( m_resolution.x/static_cast<float>(m_resolution.y), 1.0f) * 2.0f, 0.1f);

	Object* light = new Sphere(glm::vec3(1, 1, -0.5), 0.5f);
	light->material.emissive= glm::vec3(2.0, 1.5, 1.5);
	//light->material.color = glm::vec3(1.0, 0.0, 0.0);
	//light->material.roughness = 0.1f;
	m_colliders.push_back(light);
	
	Object* sphere1 = new Sphere(glm::vec3(0, -100.5f, -1), 100.0f);
	sphere1->material.color = glm::vec3(1.0, 1.0, 1.0);
	sphere1->material.roughness = 0.5f;
	m_colliders.push_back(sphere1);

	Object* fox = new ObjMesh("Resources/Models/FOKS/FOKS.obj", glm::vec3(0, -0.5f, -0.5f), glm::vec3(0, 0, 0));
	//set scale to 0.1
	//fox->setScale(glm::vec3(0.01, 0.01, 0.01));
	fox->material.color = glm::vec3(1.0f, 0.0f, 0.0f);
	fox->material.roughness = 0.5f;
	m_colliders.push_back(fox);

	/*for (int i = 0; i < 6; i++)
		m_colliders.push_back(new Sphere(glm::ballRand(0.5f) + glm::vec3(0, 0, -1), glm::linearRand(0.1f, 0.2f) ));*/


	m_pixelsAcc = new glm::vec3[m_resolution.x * m_resolution.y];
	memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));

	m_postProcessEffects.emplace_back(new GlowEffect(0.5f, 0.1f));

	m_nbThreads = std::thread::hardware_concurrency();
	return 0;
}


void LittleRaytracer::run()
{
	if (init())
	{
		std::cout << "Init error" << '\n';
		return;
	}

	//init camera 

	glm::ivec2 currentPixelCoordinates = glm::vec2(0, 0);

	m_running = true;
	m_numFrame = 1;
	SDL_Event event;

	//FOR DEBUGGING 
			m_nbThreads = 1;

	unsigned int pixelsPerThread = m_resolution.x / m_nbThreads;
	unsigned int remainingPixelsPerThread = m_resolution.x % m_nbThreads;
	while (m_running)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					m_running = false;
					break;
				case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)
					{
					case SDLK_ESCAPE:
						m_running = false;
						break;
					case SDLK_SPACE:
						{
							SDL_Surface* sshot = SDL_CreateRGBSurface(0, m_resolution.x, m_resolution.y, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
							SDL_RenderReadPixels(m_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
							SDL_SaveBMP(sshot, "screenshot.bmp");
							SDL_FreeSurface(sshot);
						}
						break;
					}
					break;
				}
			}
		}
		if (currentPixelCoordinates.y < m_resolution.y)
		{
			currentPixelCoordinates.x = 0;
			
			//Start of multithread
			unsigned int startX = 0;
			for (unsigned int i = 0; i < m_nbThreads; ++i)
			{
				unsigned int endX = startX + pixelsPerThread + (i < remainingPixelsPerThread ? 1 : 0);

				m_threads.emplace_back(new std::thread([this](int p_currY, int p_startX, int p_endX)
				{
					for (int currX = p_startX; currX < p_endX; ++currX)
					{
						glm::vec3 color = getPixelColor(glm::ivec2(currX, p_currY));
						
						std::lock_guard<std::mutex> lck(m_mutex);
						m_pixelsAcc[p_currY * m_resolution.x + currX] += color;
						updatePixelOnScreen(currX, p_currY, m_pixelsAcc[p_currY * m_resolution.x + currX] / static_cast<float>(m_numFrame));
					}
				}, currentPixelCoordinates.y, startX, endX));

				startX = endX;
			}

			// Join threads
			for (std::thread* thread : m_threads)
			{
				thread->join();
				delete thread;
			}

			m_threads.clear();

			SDL_RenderPresent(m_renderer);
			SDL_Delay(0);
			currentPixelCoordinates.y++;
		}
		else
		{
			m_numFrame++;
			currentPixelCoordinates.x = 0;
			currentPixelCoordinates.y = 0;
			
			//for(PostProcessEffect* effect : m_postProcessEffects)
			//	effect->applyPostProcess(m_pixelsAcc, m_resolution.x, m_resolution.y);
		}
	}
}


void LittleRaytracer::updatePixelOnScreen(int p_x, int p_y, glm::vec3 p_rgb) const
{
	glm::ivec4 RGBA(0, 0, 0, 255);
	for (int i = 0; i < 3; i++)
		RGBA[i] = glm::clamp(static_cast<int>(p_rgb[i] * 256), 0, 255);

	SDL_SetRenderDrawColor(m_renderer, RGBA.r, RGBA.g, RGBA.b, RGBA.a);
	SDL_RenderDrawPoint(m_renderer, p_x, p_y);
}

glm::vec3 LittleRaytracer::getPixelColor(glm::ivec2 p_pixel)
{
	std::unique_lock<std::mutex> lck(m_mutex);
	
	glm::vec3 origin = m_camera->getPosInWorld(glm::vec3(0, 0, 0));

	glm::vec3 dir = m_camera->getDirFromPixel(m_resolution, p_pixel);

	glm::vec3 pointAimed = origin + dir * m_camera->m_focalPlaneDistance;

	// creating a jittered camera position based on the aperture radius
	glm::vec3 newCameraPos = m_camera->getPosInWorld(glm::vec3(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f), 0.0f) * m_camera->m_radiusPlaneAperture);

	lck.unlock();
	
	glm::vec3 newCameraDir = glm::normalize(pointAimed - newCameraPos);

	return raytrace(newCameraPos, newCameraDir, MAX_DEPTH);
}

glm::vec3 LittleRaytracer::raytrace(glm::vec3 p_origin, glm::vec3 p_dir, int p_depth)
{
	if (p_depth == 0)

		return {0.f, 0.f, 0.f}; // IN THE SHADOW !
	// nearest collider detection
	RaycastHit rch;
	glm::vec2 interval(0.001, INFINITY);
	bool collided = false;
	for (auto& collider : m_colliders)
	{
		RaycastHit rchIt;
		if (collider->raycast(p_origin, p_dir, interval, rchIt))
		{
			collided = true;
			interval.y = rchIt.t;
			rch = rchIt;
		}
	}
	// GLOBAL ILLUMINATION / AMBIENT
	if (!collided)
		return glm::mix(glm::vec3(0.5, 0.5, 0.5)*0.1f, glm::vec3(0.7, 0.5, 0.9)*0.1f, (p_dir.y + 1.0f) / 2.0f); 


	// EMISSIVE
	glm::vec3 emissive(0, 0, 0);
	if (rch.hitMaterial.emissive != glm::vec3(0, 0, 0))
		emissive = rch.hitMaterial.emissive;			// LIGHT
	// NORMAL REGARDING ROUGHNESS (MATERIAL)
	glm::vec3 normal = glm::normalize(rch.hitNormal + rch.hitMaterial.roughness*glm::ballRand(1.0f)); // BE CAREFUL : normal can be (0,0,0) !
	// DIRECT ILLUMINATION
	glm::vec3 direct = raytrace(rch.hitPosition, normal, p_depth - 1); // ACCUMULATION
	// INDIRECT ILLUMINATION
	glm::vec3 reflected = raytrace(rch.hitPosition, glm::reflect(p_dir, normal), p_depth - 1);
	// RESULT
	return emissive + (direct + rch.hitMaterial.metallic * reflected) * rch.hitMaterial.color;
}


float LittleRaytracer::applyDirectLighting(glm::vec3 p_posLight, glm::vec3 p_pointPosition, glm::vec3 p_normal, glm::vec3 p_eyeDir) const
{
	// Phong 
	// I = Ka + Kd.(L.N) + Ks.(E.R)^s

	glm::vec3 lightDir = glm::normalize(p_posLight - p_pointPosition);

	RaycastHit rch;
	bool collided = false;
	auto interval = glm::vec2(0.001f, INFINITY);
	for (auto collider : m_colliders)
	{
		RaycastHit rchIt;
		if (collider->raycast(p_pointPosition, lightDir, interval, rchIt) && rchIt.t < interval.y)
		{
			interval.y = rchIt.t;
			rch = rchIt;
			collided = true;
		}
	}

	float iLight = 0;
	float sLight = 0;

	if (rch.hitCollider == m_colliders[0])
	{
		iLight = glm::max(0.0f, glm::dot(lightDir, p_normal));
		sLight = glm::pow(glm::max(0.0f, glm::dot(glm::reflect(-lightDir, p_normal), p_eyeDir)), 100.0f);
	}

	return iLight + sLight;
}
