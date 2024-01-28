#pragma once
#include "Utils.h"
class Texture
{
	protected:
		int m_width;
		int m_height;

	public:
		std::vector<std::vector<glm::vec3>> m_pixels;
		Texture(){
			m_width = 0;
			m_height = 0;
		}
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

			for(int i = 0; i < m_width; i++)
			{
				m_pixels.push_back(std::vector<glm::vec3>(m_height));
				for(int j = 0; j < m_height; j++)
				{
					m_pixels[i].push_back(glm::vec3(0.0f));
					Uint8 r, g, b;
					Uint32 pixel = getpixel(image, i, j);
					SDL_GetRGB(pixel, image->format, &r, &g, &b);
					m_pixels[i][j] = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
				}
			}

			SDL_FreeSurface(image);
		}

		glm::vec3 getColor(glm::vec3 &point, glm::vec3 vertices[3], glm::vec3 uv[3]) {
			glm::vec3 barycentric = getBarycentric(point, vertices);
			
			//use the barycentric to get UV position if the point
			glm::vec3 uvPoint = uv[0] * barycentric.x + uv[1] * barycentric.y + uv[2] * barycentric.z;

			//convert the UV position to pixel position
			int x = (int)(uvPoint.x * m_width);
			int y = (int)( (1 -uvPoint.y ) * m_height);

			return m_pixels[x][y];
		}

		glm::vec3 getBarycentric(glm::vec3 &point, glm::vec3 vertices[3]) {
			
			//convert the distances between the point and the vertices to a distance between 0 and 1
			float area = glm::length(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
			float alpha = glm::length(glm::cross(vertices[1] - point, vertices[2] - point)) / area; 
			float beta = glm::length(glm::cross(vertices[2] - point, vertices[0] - point)) / area; 
			float gamma = glm::length(glm::cross(vertices[0] - point, vertices[1] - point)) / area;

			return glm::vec3(alpha, beta, gamma);
		}

		Uint32 getpixel(SDL_Surface* surface, int x, int y)
		{
			int bpp = surface->format->BytesPerPixel;
			/* Here p is the address to the pixel we want to retrieve */
			Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

			switch (bpp) {
			case 1:
				return *p;
				break;

			case 2:
				return *(Uint16*)p;
				break;

			case 3:
				if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
					return p[0] << 16 | p[1] << 8 | p[2];
				else
					return p[0] | p[1] << 8 | p[2] << 16;
				break;

			case 4:
				return *(Uint32*)p;
				break;

			default:
				return 0;       /* shouldn't happen, but avoids warnings */
			}
		}

};

