#pragma once
#include "Utils.h"
class Texture
{
	protected:
		int m_width;
		int m_height;

	public:
		std::vector<std::vector<glm::vec3>> m_pixels;
		Texture();
		~Texture();
		void loadFromFile(const char* p_path);

		glm::vec3 getColor(glm::vec3& point, glm::vec3 vertices[3], glm::vec3 uv[3]);

		glm::vec3 getBarycentric(glm::vec3& point, glm::vec3 vertices[3]);

		Uint32 getpixel(SDL_Surface* surface, int x, int y);

};

