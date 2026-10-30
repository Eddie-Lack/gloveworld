#ifndef MACROS_H
#define MACROS_H

#define ABS(x) ((x) > 0.0f ? (x) : -(x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define SCALE_VECTOR(v, s) { (v)->x *= (s); (v)->y *= (s); (v)->z *= (s); }
#define SCALE_VECTOR_2D(v, s) { (v)->x *= (s); (v)->z *= (s); }
#define ADD_VECTORS(out, a, b) { (out)->x = (a)->x + (b)->x; (out)->y = (a)->y + (b)->y; (out)->z = (a)->z + (b)->z; }
#define SUB_VECTORS(out, a, b) { (out)->x = (a)->x - (b)->x; (out)->y = (a)->y - (b)->y; (out)->z = (a)->z - (b)->z; }
#define COPY_VECTOR(out, a) (*(out) = *(a))
#define SUB_FROM_VECTOR(out, a) { (out)->x -= (a)->x; (out)->y -= (a)->y; (out)->z -= (a)->z; }
#define ADD_TO_VECTOR(out, a) { (out)->x += (a)->x; (out)->y += (a)->y; (out)->z += (a)->z; }
#define ZERO_VECTOR(v) ((v)->x = (v)->y = (v)->z = 0.0f)
#define DOT_PRODUCT(a, b) ((a)->x * (b)->x + (a)->y * (b)->y + (a)->z * (b)->z)
#define LENGTH(v) sqrtf((v)->x * (v)->x + (v)->y * (v)->y + (v)->z * (v)->z)
#define LENGTH_2D(v) sqrtf((v)->x * (v)->x + (v)->z * (v)->z)

#define PI 3.141592654
#define TWOPI 6.283185308

#endif
