#include "Utils.h"
#include "CLittleRaytracer.h"
#include "Sphere.h"
#include "ObjMesh.h"


LittleRaytracer::LittleRaytracer(glm::ivec2 p_outputRes) :
	m_window(NULL),
	m_renderer(NULL),
	m_resolution(p_outputRes),
	m_running(false),
	m_camera(NULL),
	m_pixelsAcc(NULL),
	m_numFrame(1)
{
}


LittleRaytracer::~LittleRaytracer()
{
	if (m_pixelsAcc)
		delete[] m_pixelsAcc;

	delete m_camera;

	for (int i = 0; i < m_colliders.size(); i++)
		delete m_colliders[i];
	
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


	m_camera = new Camera(1.0f, glm::vec2( m_resolution.x/(float)m_resolution.y, 1.0f) * 2.0f);


	Object * light = new Sphere(glm::vec3(1, 1, -0.5), 0.5f);
	light->material = new Material();
	light->material->emissive= glm::vec3(2.0, 1.5, 1.5);
	//light->material->color = glm::vec3(1.0, 0.0, 0.0);
	//light->material->roughness = 0.1f;
	m_colliders.push_back(light);
	
	Object* sphere1 = new Sphere(glm::vec3(0, -100.5f, -1), 100.0f);
	sphere1->material = new Material();
	sphere1->material->color = glm::vec3(1.0, 1.0, 1.0);
	sphere1->material->roughness = 0.5f;
	m_colliders.push_back(sphere1);

	Object* fox = new ObjMesh("Resources/Models/FOKS/FOKS.obj", glm::vec3(0, -0.5f, -0.5f), glm::vec3(0, 0, 0));
	//set scale to 0.1
	//fox->setScale(glm::vec3(0.01, 0.01, 0.01));
	fox->material = new Material();
	fox->material->setTexture("Resources/Models/FOKS/diffuse.bmp");
	fox->material->color = glm::vec3(1.0f, 0.0f, 0.0f);
	fox->material->roughness = 1.f;
	m_colliders.push_back(fox);


	m_pixelsAcc = new glm::vec3[m_resolution.x * m_resolution.y];
	memset(m_pixelsAcc, 0, m_resolution.x * m_resolution.y * sizeof(glm::vec3));

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

	//init threads to render each lines of the screen
	std::vector<std::thread> threads;
	//get max number of threads - 1
	int maxThreads = std::thread::hardware_concurrency() / 2 ;

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

		//divide screen by line, and render each line in a thread
		for (int i = 0; i < maxThreads; i++) {
			threads.push_back(std::thread(&LittleRaytracer::renderLine, this, currentPixelCoordinates.y));
			if(currentPixelCoordinates.y + 1 < m_resolution.y)
				currentPixelCoordinates.y++;
			else {
				m_numFrame++;
				currentPixelCoordinates.y = 0;
			}
		}

		//wait for all threads to finish
		for (int i = 0; i < threads.size(); i++) {
			threads[i].join();
		}

		//reset threads
		threads.clear();

		//update pixels on screen
		for (int y = 0; y < m_resolution.y; y++)
		{
			for (int x = 0; x < m_resolution.x; x++)
			{
				glm::vec3 color = m_pixelsAcc[y * m_resolution.x + x] / (float)m_numFrame;
				updatePixelOnScreen(x, y, color);
			}
		}

		SDL_RenderPresent(m_renderer);
		SDL_Delay(0);

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

	return raytrace(origin, dir, MAX_DEPTH);
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
	//calcul ambiant direct indirect light

	//color start at ambiant light
	glm::vec3 color = glm::vec3(0, 0, 0);
	if (collided)
	{
		//check if material if emissive
		if (rch.hitMaterial->emissive != glm::vec3(0, 0, 0))
			return rch.hitMaterial->emissive;
		
		if (rch.hitMaterial->texture != NULL)
			color += rch.hitMaterial->getColorFromTexture(rch.hitPosition,rch.hitVertices,rch.hitUV);
		else
			color += rch.hitMaterial->color;
		
		//apply ligh is using the last rchhit

		
	}
	else
	{
		//no collider hit, return sky color
		color = glm::vec3(0.f, 0.2f, 0.5f);
	}

	return color;

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
    if (collided) {
        // Check if the collided object is the light source
        if (rch.hitCollider == m_colliders[0])
        {
            // Skip the shadow calculation for the light source
            iLight = glm::max(0.0f, glm::dot(lightDir, p_normal));
        }
        else
        {
            sLight = glm::pow(glm::max(0.0f, glm::dot(glm::reflect(-lightDir, p_normal), p_eyeDir)), 100.0f);
			sLight = glm::pow(glm::max(0.0f, glm::dot(glm::reflect(-lightDir, p_normal), p_eyeDir)), 100.0f);
        }
    }

    return iLight + sLight;
}

//create function that will be used by thread to render a line of the screen
void LittleRaytracer::renderLine(int p_line){
	//for each pixel of the line
	for (int x = 0; x < m_resolution.x; x++)
	{
		glm::vec3 color = getPixelColor(glm::ivec2(x, p_line));
		m_pixelsAcc[p_line * m_resolution.x + x] += color; 

	}
}
