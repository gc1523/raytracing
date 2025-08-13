#ifndef RT_H
#define RT_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>


// C++ Std Usings

using std::make_shared;
using std::shared_ptr;


// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // Returns a random real in [0,1).
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(std::mt19937& rng) {
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

inline double random_double(double min, double max, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

inline int random_int(int min, int max, std::mt19937& rng) {
    // Returns a random integer in [min,max].
    return int(random_double((double)min, (double)(max+1), rng));
}

// Common Headers

#include "colour.h"
#include "ray.h"
#include "vec3.h"
#include "interval.h"
#endif