#include "util.h"
#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

std::string format(const std::string &fmt, ...) {
    va_list args;

    va_start(args, fmt);
    char *buffer = NULL;
    int size = vsnprintf(buffer, 0, fmt.c_str(), args);
    va_end(args);
    buffer = new char[size + 1];
    
    va_start(args, fmt);
    vsnprintf(buffer, size + 1, fmt.c_str(), args);
    va_end(args);
    std::string str(buffer);
    delete [] buffer;

    va_end(args);
    return str;
}

float uniformRandomInRange(float min, float max) {
    assert(min < max);
    double n = (double) rand() / (double) RAND_MAX;
    double v = min + n * (max - min);
    return v;
}

void seed_random() {
	struct timeval ts;
	gettimeofday(&ts, NULL);

    /* calculate dt */
    double t = ts.tv_sec * 1000000.0;
    t += ts.tv_usec;
	 srand(t);
}
