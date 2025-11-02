//
// Created by neo on 31.10.25.
//

#ifndef RAYTRACINGTOOLS_RANDOM_H
#define RAYTRACINGTOOLS_RANDOM_H

#endif //RAYTRACINGTOOLS_RANDOM_H
#pragma once

#include <random>

static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_real_distribution<double> dist(0.0, 1.0);

inline double easy_uniform_random () {
    return dist(mt);
}