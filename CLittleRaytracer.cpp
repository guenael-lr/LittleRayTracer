#include "Utils.h"
#include "CLittleRaytracer.h"
#include "Sphere.h"
#include "ObjMesh.h"

LittleRaytracer::LittleRaytracer(glm::ivec2 p_outputRes) :
	m_running(false),
	m_withEffect(false),
	m_window(nullptr),
	m_renderer(nullptr),
	m_resolution(p_outputRes),
	m_camera(nullptr),
	m_pixelsAcc(nullptr),
	m_postProcessedPixels(nullptr),
	m_numFrame(1),
	m_nbThreads(NB_THREADS),
	m_threads({}),
	m_postProcessEffects({})
{
}


LittleRaytracer::~LittleRaytracer()
{
	delete[] m_pixelsAcc;

	delete[] m_postProcessedPixels;

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


	m_camera = new Camera(1.0f, glm::vec2( m_resolution.x/static_cast<float>(m_resolution.y), 1.0f) * 2.0f, 0.0f);


	Object * sphere0 = new Sphere(glm::vec3(1.f, 1.f, -2.f), 1.0f);
	sphere0->material.emissive= glm::vec3(3.f, 2.5f, 1.f);
	//sphere0->material.color = glm::vec3(1.0, 0.0, 0.0);
	//sphere0->material.roughness = 0.1f;
	m_colliders.push_back(sphere0);
	
	//Ground sphere
	Object* sphere1 = new Sphere(glm::vec3(0.f, -100.5f, -1.f), 100.0f);
	sphere1->material.color = glm::vec3(1.0f, 1.0f, 1.0f);
	sphere1->material.roughness = 0.3f;
	m_colliders.push_back(sphere1);
	
#if RENDER_OBJ
	Object* fox = new ObjMesh("../Resources/Models/FOKS/FOKS.obj", glm::vec3(0.f, -0.5f, -0.5f), glm::vec3(0.f, 0.f, 0.f));

	fox->material.setTexture("../Resources/Models/FOKS/diffuse.bmp");
	fox->material.color = glm::vec3(1.0f, 0.0f, 0.0f);
	fox->material.roughness = 0.6f;
	fox->material.metallic = 0.4f;
	m_colliders.push_back(fox);
#else
	Object* sphere2 = new Sphere(glm::vec3(-1.f, 0.f, -1.f), 0.5f);
	sphere2->material.color = glm::vec3(0.0f, 0.0f, 1.0f);
	sphere2->material.roughness = 0.5f;
	m_colliders.push_back(sphere2);

	Object* sphere4 = new Sphere(glm::vec3(0.f, 0.f, -1.0f), 0.5f);
	sphere4->material.color = glm::vec3(0.0f, 1.0f, 0.0f);
	sphere4->material.roughness = 0.5f;
	m_colliders.push_back(sphere4);

	Object* sphere3 = new Sphere(glm::vec3(0.7f, 0.f, -0.2f), 0.5f);
	sphere3->material.color = glm::vec3(0.0f, 0.0f, 1.0f);
	sphere3->material.roughness = 0.5f;
	m_colliders.push_back(sphere3);
#endif

	m_pixelsAcc = new glm::vec3[m_resolution.x * m_resolution.y];
	m_postProcessedPixels = new glm::vec3[m_resolution.x * m_resolution.y];
	memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
	memset(m_postProcessedPixels, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
	
#if USE_TONEMAPPING
	m_postProcessEffects.emplace_back(new ToneMappingEffect(TONEMAPPING_KEY));
#endif
#if USE_GLOW
	m_postProcessEffects.emplace_back(new GlowEffect(GLOW_LUMINOSITY_THRESHOLD, GLOW_NB_BLUR_PASS, GLOW_EXPOSURE));
#endif
#if USE_GAMMA
	m_postProcessEffects.emplace_back(new GammaCorrectionEffect(GAMMA_VALUE));
#endif
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
	m_withEffect = false;
	SDL_Event event;
	//init time
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

	//vector for time 
	float SPF = 0.0f;


	while (m_running)
	{
		if (m_numFrame > 1) {
			std::string title = "Little Raytracer - " + std::to_string(SPF / (m_numFrame - 1)).substr(0, std::to_string(SPF / (m_numFrame - 1)).find(".") + 3) + "s per frame";
			SDL_SetWindowTitle(m_window, title.c_str());
		}
		

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
					case SDLK_f:
						m_withEffect = !m_withEffect;
						break;
					case SDLK_p:
						// add 0.01f to the distance of the focal plane
						m_camera->m_focalPlaneDistance += 0.01f;
						// reset the pixel accumulator
						memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						memset(m_postProcessedPixels, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						// reset the number of frame
						m_numFrame = 1;
						currentPixelCoordinates.x = 0;
						currentPixelCoordinates.y = 0;
						break;
					case SDLK_m:
						// remove 0.01f to the distance of the focal plane
						m_camera->m_focalPlaneDistance -= 0.01f;
						// reset the pixel accumulator
						memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						memset(m_postProcessedPixels, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						// reset the number of frame
						m_numFrame = 1;
						currentPixelCoordinates.x = 0;
						currentPixelCoordinates.y = 0;
						break;
					case SDLK_l:
						// remove 0.01f to the radius of the aperture
						m_camera->m_radiusPlaneAperture -= 0.01f;
						// reset the pixel accumulator
						memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						memset(m_postProcessedPixels, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						// reset the number of frame
						m_numFrame = 1;
						currentPixelCoordinates.x = 0;
						currentPixelCoordinates.y = 0;
						break;
					case SDLK_o:
						// add 0.01f to the radius of the aperture
						m_camera->m_radiusPlaneAperture += 0.01f;
						// reset the pixel accumulator
						memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						memset(m_postProcessedPixels, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						// reset the number of frame
						m_numFrame = 1;
						currentPixelCoordinates.x = 0;
						currentPixelCoordinates.y = 0;
						break;
					case SDLK_d:
						//Reset focus plane distance and aperture radius
						m_camera->m_radiusPlaneAperture = 0.f;
						m_camera->m_focalPlaneDistance = 1.f;
						memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						memset(m_postProcessedPixels, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));
						// reset the number of frame
						m_numFrame = 1;
						currentPixelCoordinates.x = 0;
						currentPixelCoordinates.y = 0;
						break;
					}
				}
			}
		}

		if (currentPixelCoordinates.y < m_resolution.y)
		{
			//Start of multithread
			for (unsigned int i = 0; i < m_nbThreads; ++i)
			{
				m_threads.emplace_back(new std::thread([this](glm::ivec2 p_currentPixelCoordinates)
					{
						std::srand(static_cast<unsigned int>(time(nullptr)));
						for (int currX = 0; currX < m_resolution.x; ++currX) {
							glm::vec3 color = getPixelColor(glm::ivec2(currX, p_currentPixelCoordinates.y));
							std::lock_guard<std::mutex> lck(m_mutex);
							m_pixelsAcc[p_currentPixelCoordinates.y * m_resolution.x + currX] += color;
							if (m_withEffect)
								updatePixelOnScreen(currX, p_currentPixelCoordinates.y, m_postProcessedPixels[p_currentPixelCoordinates.y * m_resolution.x + currX]);
							else
								updatePixelOnScreen(currX, p_currentPixelCoordinates.y, m_pixelsAcc[p_currentPixelCoordinates.y * m_resolution.x + currX] / static_cast<float>(m_numFrame));
						}
					


					}, currentPixelCoordinates));


				currentPixelCoordinates.y++;
				if (currentPixelCoordinates.y == m_resolution.y)
					i = m_nbThreads;
				
			}
			for (std::thread* thread : m_threads)
			{
				thread->join();
				delete thread;
			}
			m_threads.clear();
			SDL_RenderPresent(m_renderer);
			SDL_Delay(0);
		}
		else
		{
			//Normally post process should be before but it makes them softer to be late by one frame
			m_numFrame++;

			currentPixelCoordinates.y = 0;
			
			if (m_postProcessEffects.empty())
				continue;
			
			m_postProcessEffects[0]->applyPostProcess(m_pixelsAcc, m_postProcessedPixels, m_resolution.x, m_resolution.y, m_numFrame);

			//apply Post effects over the first one applied
			for(size_t i = 1; i < m_postProcessEffects.size(); ++i)
				m_postProcessEffects[i]->applyPostProcess(m_postProcessedPixels, m_postProcessedPixels, m_resolution.x, m_resolution.y, 1);
			
			//get time
			std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
			
			float delta = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.f;
			SPF += delta;
			start = end;
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
		return {0, 0, 0}; // IN THE SHADOW !
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

	glm::vec3 color = rch.hitMaterial.color;
	if(rch.hitMaterial.texture)
		color = rch.hitMaterial.texture->getColor( rch.hitPosition,rch.hitVertices,rch.hitUV);

	return emissive + (direct + rch.hitMaterial.metallic * reflected) * color;
}


float LittleRaytracer::applyDirectLighting(glm::vec3 p_posLight, glm::vec3 p_pointPosition, glm::vec3 p_normal, glm::vec3 p_eyeDir) const
{
	// Phong 
	// I = Ka + Kd.(L.N) + Ks.(E.R)^s

	glm::vec3 lightDir = glm::normalize(p_posLight - p_pointPosition);

	RaycastHit rch;
	auto interval = glm::vec2(0.001f, INFINITY);
	for (auto* collider : m_colliders)
	{
		RaycastHit rchIt;
		if (collider->raycast(p_pointPosition, lightDir, interval, rchIt) && rchIt.t < interval.y)
		{
			interval.y = rchIt.t;
			rch = rchIt;
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
