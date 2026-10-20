#include "math/vector_utils.h"

float ClampFloat(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}
