#include "generators.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace waves
{
    double sine(double time, double amp, double freq)
    {
        return amp*::sin(time*freq*M_PI*2);
    }

    double square(double time, double amp, double freq)
    {
        return sine(time, amp, freq) > 0. ? 1. : 0.;
    }

}
