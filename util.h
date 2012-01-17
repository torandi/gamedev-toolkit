#ifndef UTIL_H
#define UTIL_H

#include <cmath>

inline float deg_to_rad(const float &fAngDeg) {
	const float fDegToRad = M_PI * 2.0f / 360.0f;
	return fAngDeg * fDegToRad;
}

inline float rad_to_deg(const float &fAngRad) {
	const float fDegToRad = M_PI * 2.0f / 360.0f;
	return fAngRad / fDegToRad;
}
#endif
