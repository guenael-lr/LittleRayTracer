#pragma once

#include "./ThirdPartyLibs/SDL2-2.0.18/include/SDL.h"

#define GLM_FORCE_SWIZZLE
#include "./ThirdPartyLibs/glm/glm/ext.hpp"
#include "./ThirdPartyLibs/glm/glm/glm.hpp"

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

//Number of bounce per sent ray
#define MAX_DEPTH 10

#define NB_THREADS (.75f * std::thread::hardware_concurrency())

#define USE_GLOW 1
#define USE_TONEMAPPING 1
#define USE_GAMMA 1


//Prevail a value between 1 and 10, number of times we divide each side of the AABB box collider
#define NB_AABB_DIVISIONS 7

//post process
#define GLOW_EXPOSURE 1.2f
#define GLOW_NB_BLUR_PASS 5
#define GLOW_LUMINOSITY_THRESHOLD 1.f
#define TONEMAPPING_KEY 1.2f
#define GAMMA_VALUE 1.33f

//If enabled, will render the foks model, otherwise some spheres
#define RENDER_OBJ 0