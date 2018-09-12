#include "generators.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace waves
{
    float sine(float time, float amp, float freq)
    {
        return amp*::sin(time*freq*M_PI*2);
    }

    float square(float time, float amp, float freq)
    {
        return sine(time, amp, freq) > 0. ? 1. : 0.;
    }

}
