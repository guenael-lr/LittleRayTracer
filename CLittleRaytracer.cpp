#include "Utils.h"
#include "CLittleRaytracer.h"
#include "Sphere.h"
#include <thread>

LittleRaytracer::LittleRaytracer(glm::ivec2 p_outputRes) :
	m_running(false),
	m_window(NULL),
	m_renderer(NULL),
	m_resolution(p_outputRes),
	m_camera(NULL),
	m_pixelsAcc(NULL),
	m_numFrame(1),
	m_postProcessEffects({})
{
}


LittleRaytracer::~LittleRaytracer()
{
	if (m_pixelsAcc)
		delete[] m_pixelsAcc;

	delete m_camera;

	for (int i = 0; i < m_colliders.size(); i++)
		delete m_colliders[i];
	
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
	std::srand((unsigned int)time(NULL));


	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

	SDL_CreateWindowAndRenderer(m_resolution.x, m_resolution.y, 0, &m_window, &m_renderer);
	SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 0);
	SDL_RenderClear(m_renderer);
	SDL_RenderPresent(m_renderer);


	m_camera = new Camera(1.0f, glm::vec2( m_resolution.x/(float)m_resolution.y, 1.0f) * 2.0f, 0.1);


	Object * sphere0 = new Sphere(glm::vec3(1, 1, -2), 1.0f);
	sphere0->material.emissive= glm::vec3(2.0, 1.5, 1.5);
	//sphere0->material.color = glm::vec3(1.0, 0.0, 0.0);
	//sphere0->material.roughness = 0.1f;
	m_colliders.push_back(sphere0);
	
	Object* sphere1 = new Sphere(glm::vec3(0, -100.5f, -1), 100.0f);
	sphere1->material.color = glm::vec3(1.0, 1.0, 1.0);
	sphere1->material.roughness = 0.5f;
	m_colliders.push_back(sphere1);

	Object* sphere2 = new Sphere(glm::vec3(-1, 0, -1.0), 0.5f);
	sphere2->material.color = glm::vec3(0.0, 0.0, 1.0);
	sphere2->material.roughness = 0.5f;
	m_colliders.push_back(sphere2);

	Object* sphere4 = new Sphere(glm::vec3(0, 0, -1.0), 0.5f);
	sphere4->material.color = glm::vec3(0.0, 1.0, 0.0);
	sphere4->material.roughness = 0.5f;
	m_colliders.push_back(sphere4);

	Object* sphere3 = new Sphere(glm::vec3(0.7, 0, -0.2), 0.5f);
	sphere3->material.color = glm::vec3(0.0, 0.0, 1.0);
	sphere3->material.roughness = 0.5f;
	m_colliders.push_back(sphere3);

	/*for (int i = 0; i < 6; i++)
		m_colliders.push_back(new Sphere(glm::ballRand(0.5f) + glm::vec3(0, 0, -1), glm::linearRand(0.1f, 0.2f) ));*/


	m_pixelsAcc = new glm::vec3[m_resolution.x * m_resolution.y];
	memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));

	m_postProcessEffects.emplace_back(new GlowEffect(0.5f, 0.1f));
	return 0;
}


void LittleRaytracer::run()
{
	if (init())
	{
		std::cout << "Init error" << std::endl;
		return;
	}

	//init camera 

	glm::ivec2 currentPixelCoordinates = glm::vec2(0, 0);

	m_running = true;
	m_numFrame = 1;
	SDL_Event event;
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
			//glm::vec3 color = glm::ivec3(glm::linearRand(0, 1), glm::linearRand(0, 1), glm::linearRand(0, 1));
			// multithread this
			glm::vec3 color = getPixelColor(currentPixelCoordinates);
			
			m_pixelsAcc[currentPixelCoordinates.y * m_resolution.x + currentPixelCoordinates.x] += color;
			// end of multithread

			updatePixelOnScreen(currentPixelCoordinates.x, currentPixelCoordinates.y, m_pixelsAcc[currentPixelCoordinates.y * m_resolution.x + currentPixelCoordinates.x] / (float)m_numFrame);

			currentPixelCoordinates.x++;
			if (currentPixelCoordinates.x == m_resolution.x)
			{
				currentPixelCoordinates.x = 0;
				currentPixelCoordinates.y++;

				SDL_RenderPresent(m_renderer);
				SDL_Delay(0);
			}
		}
		else
		{
			m_numFrame++;
			currentPixelCoordinates.x = 0;
			currentPixelCoordinates.y = 0;
			
			for(PostProcessEffect* effect : m_postProcessEffects)
				effect->applyPostProcess(m_pixelsAcc, m_resolution.x, m_resolution.y);
		}
	}
}


void LittleRaytracer::updatePixelOnScreen(int p_x, int p_y, glm::vec3 p_rgb)
{
	glm::ivec4 RGBA(0, 0, 0, 255);
	for (int i = 0; i < 3; i++)
		RGBA[i] = glm::clamp((int)(p_rgb[i]*256), 0, 255);

	SDL_SetRenderDrawColor(m_renderer, RGBA.r, RGBA.g, RGBA.b, RGBA.a);
	SDL_RenderDrawPoint(m_renderer, p_x, p_y);
}

glm::vec3 LittleRaytracer::getPixelColor(glm::ivec2 p_pixel)
{
	glm::vec3 origin = m_camera->getPosInWorld(glm::vec3(0, 0, 0));

	glm::vec3 dir = m_camera->getDirFromPixel(m_resolution, p_pixel);

	glm::vec3 pointAimed = origin + dir * m_camera->m_focalPlaneDistance;

	// creating a jittered camera position based on the aperture radius
	glm::vec3 newCameraPos = m_camera->getPosInWorld(glm::vec3(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f), 0.0f) * m_camera->m_radiusPlaneAperture);
	
	glm::vec3 newCameraDir = glm::normalize(pointAimed - newCameraPos);

	return raytrace(newCameraPos, newCameraDir, MAX_DEPTH);
}

glm::vec3 LittleRaytracer::raytrace(glm::vec3 p_origin, glm::vec3 p_dir, int p_depth)
{
	if (p_depth == 0)
		return glm::vec3(0, 0, 0); // IN THE SHADOW !
	// nearest collider detection
	RaycastHit rch;
	glm::vec2 interval(0.001, INFINITY);
	bool collided = false;
	for (int i = 0; i < m_colliders.size(); i++)
	{
		RaycastHit rchIt;
		if (m_colliders[i]->raycast(p_origin, p_dir, interval, rchIt))
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


float LittleRaytracer::applyDirectLighting(glm::vec3 p_posLight, glm::vec3 p_pointPosition, glm::vec3 p_normal, glm::vec3 p_eyeDir)
{
	// Phong 
	// I = Ka + Kd.(L.N) + Ks.(E.R)^s

	glm::vec3 lightDir = glm::normalize(p_posLight - p_pointPosition);

	RaycastHit rch;
	bool collided = false;
	glm::vec2 interval = glm::vec2(0.001f, INFINITY);
	for (int i = 0; i < m_colliders.size(); i++)
	{
		RaycastHit rchIt;
		if (m_colliders[i]->raycast(p_pointPosition, lightDir, interval, rchIt) && rchIt.t < interval.y)
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