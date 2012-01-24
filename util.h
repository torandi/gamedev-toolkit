#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <string>
#include <cstdlib>

float uniformRandomInRange(float min, float max);
std::string format(const std::string &fmt, ...);

inline float deg_to_rad(const float &fAngDeg) {
	const float fDegToRad = M_PI * 2.0f / 360.0f;
	return fAngDeg * fDegToRad;
}

inline float rad_to_deg(const float &fAngRad) {
	const float fDegToRad = M_PI * 2.0f / 360.0f;
	return fAngRad / fDegToRad;
}

inline float frand() {
	return (float)rand()/RAND_MAX;
}

void seed_random();
#endif
