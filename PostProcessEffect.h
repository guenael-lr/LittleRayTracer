#pragma once
#include "AABB.h"
#include "AABB.h"
#include "Utils.h"

class PostProcessEffect
{
public:
    PostProcessEffect() = default;
    virtual ~PostProcessEffect() = default;
    virtual void applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int& p_resolutionX, const int& p_resolutionY, const
                                  int& p_numFrame) = 0;
};

class GlowEffect final : public PostProcessEffect
{
public:
    explicit GlowEffect(const float p_luminosityThreshold = 0.7f, const unsigned int p_nbBlurPass = 5.f, const float p_exposure = 2.f) :
        m_luminosityThreshold(p_luminosityThreshold), m_nbBlurPass(p_nbBlurPass), m_exposure(p_exposure) { }
    void applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int& p_resolutionX, const int& p_resolutionY, const
                          int& p_numFrame) override;
private:
    void applyBilinearFiltering(glm::vec3* p_pixels, const int& p_resolutionX, const int& p_resolutionY);
    float m_luminosityThreshold;
    unsigned int m_nbBlurPass;
    float m_exposure;
};

class GammaCorrectionEffect final : public PostProcessEffect
{
public:
	explicit GammaCorrectionEffect(const float p_gamma = 2.2f) : m_gamma(p_gamma) { }
	void applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int& p_resolutionX, const int& p_resolutionY, const
                          int& p_numFrame) override;

private:
    float m_gamma; //Correspond to the gamma correction value
};

class ToneMappingEffect final : public PostProcessEffect
{
public:
    explicit ToneMappingEffect(const float p_key = 1.0f) : m_key(p_key) { }
	void applyPostProcess(glm::vec3* p_scenePixels, glm::vec3* p_postProcessedScenePixels, const int& p_resolutionX, const int& p_resolutionY, const
                          int& p_numFrame) override;

    private:
	float m_key; //Correspond to the chroma key value
};