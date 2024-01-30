#include "PostProcessEffect.h"
#include "CLittleRaytracer.h"

void GlowEffect::applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int p_resolutionX, const int p_resolutionY, int p_numFrame)
{
	// Cree un tableau temporaire pour stocker l'image apr�s le filtre bilateral
	const auto tempImage = new glm::vec3[p_resolutionX * p_resolutionY];
	memcpy(tempImage, p_scenePixels, p_resolutionX * p_resolutionY * sizeof(glm::vec3));

	// Applique le filtre bilat�ral en deux passes (horizontal et vertical)
	bilateralFilter(tempImage, p_postProcessedScenePixels, p_resolutionX, p_resolutionY);

	// Libere la memoire du tableau temporaire
	delete[] tempImage;
}

void GlowEffect::bilateralFilter(glm::vec3* p_input, glm::vec3* p_output, const int p_width, const int p_height)
{
	// Calculez le rayon du filtre en fonction de l'�cart-type spatial
	const int radius = static_cast<int>(ceil(m_sigmaSpace * 3.0f));

	// Cr�ez un tableau temporaire pour stocker l'image apr�s le filtre spatial
	const auto tempImage = new glm::vec3[p_width * p_height];

	// Applique le filtre spatial en deux passes (horizontal et vertical)
	spatialFilter(p_input, tempImage, p_width, p_height, radius);

	// Applique le filtre de couleur
	colorFilter(tempImage, p_output, p_width, p_height, radius);

	// Libere la memoire du tableau temporaire
	delete[] tempImage;
}

void GlowEffect::spatialFilter(glm::vec3* p_input, glm::vec3* p_output, int p_width, int p_height, int p_radius)
{
	// Calcule les poids du filtre spatial
	auto weights = new float[2 * p_radius + 1];
	float sum = 0.0f;
	for (int i = -p_radius; i <= p_radius; i++)
	{
		weights[i + p_radius] = expf(-static_cast<float>(i * i) / (2.0f * m_sigmaSpace * m_sigmaSpace));
		sum += weights[i + p_radius];
	}

	// Normalise les poids
	for (int i = 0; i < 2 * p_radius + 1; i++)
		weights[i] /= sum;

	// Applique le filtre spatial en deux passes (horizontal et vertical)
	for (int pass = 0; pass < 2; pass++)
	{
		// Parcoure chaque ligne de l'image
		for (int y = 0; y < p_height; y++)
		{
			// Parcoure chaque colonne de l'image
			for (int x = 0; x < p_width; x++)
			{
				// Calcule la couleur filtree pour le pixel (x, y)
				auto color = glm::vec3(0.0f);
				for (int i = -p_radius; i <= p_radius; i++)
				{
					// Calcule la position du pixel voisin
					int xNeighbor = x;
					int yNeighbor = y;
					if (pass == 0)
						xNeighbor += i;
					else
						yNeighbor += i;

					// Check si le pixel voisin est dans l'image
					if (xNeighbor >= 0 && xNeighbor < p_width && yNeighbor >= 0 && yNeighbor < p_height)
					{
						// Calcule le poids du pixel voisin
						float weight = weights[i + p_radius];

						// Ajoute la couleur du pixel voisin � la couleur filtree
						color += weight * p_input[yNeighbor * p_width + xNeighbor];
					}
				}

				// Stocke la couleur filtree dans l'image de sortie
				p_output[y * p_width + x] = color;
			}
		}
	}

	// Libere la m�moire des poids
	delete[] weights;
}

void GlowEffect::colorFilter(glm::vec3* p_input, glm::vec3* p_output, int p_width, int p_height, int p_radius)
{
	// Calcule les poids du filtre de couleur
	auto weights = new float[2 * p_radius + 1];
	float sum = 0.0f;
	for (int i = -p_radius; i <= p_radius; i++)
	{
		weights[i + p_radius] = expf(-static_cast<float>(i * i) / (2.0f * m_sigmaColor * m_sigmaColor));
		sum += weights[i + p_radius];
	}

	// Normalise les poids
	for (int i = 0; i < 2 * p_radius + 1; i++)
		weights[i] /= sum;

	// Applique le filtre de couleur en deux passes (horizontal et vertical)
	for (int pass = 0; pass < 2; pass++)
	{
		// Parcoure chaque ligne de l'image
		for (int y = 0; y < p_height; y++)
		{
			// Parcourez chaque colonne de l'image
			for (int x = 0; x < p_width; x++)
			{
				// Calcule la couleur filtr�e pour le pixel (x, y)
				auto color = glm::vec3(0.0f);
				for (int i = -p_radius; i <= p_radius; i++)
				{
					// Calcule la position du pixel voisin
					int xNeighbor = x;
					int yNeighbor = y;
					if (pass == 0)
						xNeighbor += i;
					else
						yNeighbor += i;

					// Assure-vous que le pixel voisin est dans l'image
					if (xNeighbor >= 0 && xNeighbor < p_width && yNeighbor >= 0 && yNeighbor < p_height)
					{
						// Calcule le poids du pixel voisin
						float weight = weights[i + p_radius];

						// Ajoute la couleur du pixel voisin a la couleur filtree
						color += weight * p_input[yNeighbor * p_width + xNeighbor];
					}
				}

				// Stocke la couleur filtr�e dans l'image
				p_output[y * p_width + x] = color;
			}
		}
	}

	// Libere la memoire des poids
	delete[] weights;
}

void GammaCorrectionEffect::applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int p_resolutionX, const int p_resolutionY, int p_numFrame)
{
	// Cree un tableau temporaire pour stocker l'image apr�s le filtre bilateral
	const auto tempImage = new glm::vec3[p_resolutionX * p_resolutionY];
	memcpy(tempImage, p_scenePixels, p_resolutionX * p_resolutionY * sizeof(glm::vec3));

	// apply gamma correction on accumulated pixels with r g b
	for (int i = 0; i < p_resolutionX * p_resolutionY; i++)
	{
		tempImage[i].r = powf(tempImage[i].r / p_numFrame, 1.0f / m_gamma);
		tempImage[i].g = powf(tempImage[i].g / p_numFrame, 1.0f / m_gamma);
		tempImage[i].b = powf(tempImage[i].b / p_numFrame, 1.0f / m_gamma);
	}

	// Libere la memoire du tableau temporaire
	delete[] tempImage;
}

void ToneMappingEffect::applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int p_resolutionX, const int p_resolutionY, int p_numFrame)
{
	// Cree un tableau temporaire pour stocker l'image apr�s le filtre bilateral
	const auto tempImage = new glm::vec3[p_resolutionX * p_resolutionY];
	memcpy(tempImage, p_scenePixels, p_resolutionX * p_resolutionY * sizeof(glm::vec3));

	// Calculate average luminance
	float averageLuminance = 0.0f;
	for (int i = 0; i < p_resolutionX * p_resolutionY; ++i)
	{
		float luminance = 0.2126f * tempImage[i].r + 0.7152f * tempImage[i].g + 0.0722f * tempImage[i].b;
		averageLuminance += logf(0.00001f + luminance);
	}
	averageLuminance = expf(averageLuminance / (p_resolutionX * p_resolutionY));

	// Apply Reinhard tone mapping
	for (int i = 0; i < p_resolutionX * p_resolutionY; ++i)
	{
		p_postProcessedScenePixels[i] = tempImage[i] * (m_key / (1.0f + averageLuminance));
	}

	// Libere la memoire du tableau temporaire
	delete[] tempImage;
}
