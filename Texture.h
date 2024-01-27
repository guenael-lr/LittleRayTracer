#pragma once
#include "Utils.h"
class Texture
{
	protected:
		int m_width;
		int m_height;
		glm::vec3* m_pixels;
	public:
		Texture(){}
		~Texture(){}
		void loadFromFile(const char* p_path) {
			//load image from bmp so we need to inverse image in y
			//so we need to inverse y in uv

			SDL_Surface* image = SDL_LoadBMP(p_path);
			if (image == NULL)
			{
				printf("SDL_LoadBMP Error: %s\n", SDL_GetError());
				return;
			}

			m_width = image->w;
			m_height = image->h;

			//get the pixel data
			m_pixels = new glm::vec3[m_width * m_height];

			//load image in reverse Y
			for (int y = 0; y < m_height; y++) {
				for (int x = 0; x < m_width; x++) {
					Uint32 pixel = ((Uint32*)image->pixels)[(m_height - 1 - y) * m_width + x];
					Uint8 r, g, b;
					SDL_GetRGB(pixel, image->format, &r, &g, &b);
					m_pixels[y * m_width + x] = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
				}
			}

			SDL_FreeSurface(image);
		}

		glm::vec3 getColor(glm::vec3 uv[3]) {
			//get color from uv
			glm::vec3 color = glm::vec3(0.0f);
			for (int i = 0; i < 3; i++) {
				color += m_pixels[(int)(uv[i].y * m_height) * m_width + (int)(uv[i].x * m_width)];
			}
			return color / 3.0f;

		}

};

