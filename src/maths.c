#include "common.h"
#include "ident.h"
#include "maths.h"
#include "actor.h"
#include "debug.h"
#include "libc.h"

extern u16 acostab[];
extern Quat vertQ;

extern u32 rseed;
extern u16 atantab[];
extern u16 atanStep;
extern u16 maxAtanVal;

s32 Random(s32 lim) {
    if (lim == 0) {
        return 0;
    }
    rseed = rseed ^ (rseed << 1);
    if (!(rseed & 0x80000000)) {
        rseed++;
    } else if (rseed == 0x80000000) {
        rseed = 0;
    }
    return rseed % lim;
}

void SeedRandom(void) {
    rseed = 0x6151F;
}

void CrossProduct(Vec3 *result, Vec3 *operand1, Vec3 *operand2) {
    result->x = operand1->y * operand2->z - operand1->z * operand2->y;
    result->y = operand1->z * operand2->x - operand1->x * operand2->z;
    result->z = operand1->x * operand2->y - operand1->y * operand2->x;
}

void CrossProductVertical(Vec3 *result, Vec3 *operand2) {
    result->x = operand2->z;
    result->y = 0.0f;
    result->z = -operand2->x;
}

inline f32 acos(f32 x) {
    u16 *tab = acostab;

    if (x > 1.000001) {
        PRINTF("
%f
", x);
        Crash("ACOS");
    }
    if (x < -1.000001) {
        PRINTF("
%f
", x);
        Crash("ACOS");
    }
    if (x == 1.0) {
        return 0.0f;
    }
    tab += (s32)(x * 10000.0f) + 10000;
    return *tab / 20000.0f;
}

inline f32 acosBetter(f32 x) {
    u16 *tab = acostab;
    s32 i;
    f32 f;
    f32 a;
    f32 d;

    if (x > 1.0f || x < -1.0f) {
        Crash("ACOSBETTER");
    }
    i = x * 10000.0;
    if (i == 10000) {
        return 0.0f;
    }
    f = x * 10000.0f;
    // unknown construct; two loops around the table lookup give the ROM's code here and in
    // the copies inlined into QuatSlerp and GetRotationFromQuaternion
    do {
        do {
            tab += i + 10000;
            a = *tab;
            a /= 20000.0f;
            if (i < f) {
                d = f - i;
                i++;
                if (i == 10000) {
                    a *= 1.0f - d;
                } else {
                    a = a * (1.0f - d) + acostab[i + 10000] / 20000.0f * d;
                }
            }
        } while (0);
        return a;
    } while (0);
}

void InitAtan(void) {
    atanStep = atantab[1];
    maxAtanVal = atantab[0] / atanStep;
}

inline f32 atan(f32 x) {
    u16 *tab = atantab;

    if (x > maxAtanVal) {
        x = maxAtanVal;
    }
    tab += (s32)(x * atanStep) + 2;
    return *tab / 40000.0f;
}

f32 atan2(f32 y, f32 x) {
    if (x == 0.0f && y == 0.0f) {
        return 0.0f;
    }
    if (x > 0.0f) {
        if (y > 0.0f) {
            return atan(y / x);
        }
        return -atan(-y / x);
    }
    if (x < 0.0f) {
        if (y > 0.0f) {
            return atan(-x / y) + 1.570796327;
        }
        if (y < 0.0f) {
            return -1.570796327 - atan(x / y);
        }
        return 3.1415927f;
    }
    if (y > 0.0f) {
        return 1.5707964f;
    }
    return -1.5707964f;
}

void QuatSlerp(Quat *src1, Quat *sp2, f32 t, Quat *dquat) {
    Quat negs2;
    Quat *src2;
    f32 x, y, z, w;
    f32 pdist;
    f32 ndist;
    f32 cosom;
    f32 omega;
    f32 sinom;
    f32 sclp;
    f32 sclq;

    x = src1->x - sp2->x;
    y = src1->y - sp2->y;
    z = src1->z - sp2->z;
    w = src1->w - sp2->w;
    pdist = x * x + y * y + z * z + w * w;
    x = src1->x + sp2->x;
    y = src1->y + sp2->y;
    z = src1->z + sp2->z;
    w = src1->w + sp2->w;
    ndist = x * x + y * y + z * z + w * w;
    if (pdist < ndist) {
        src2 = sp2;
    } else {
        negs2.x = -sp2->x;
        negs2.y = -sp2->y;
        negs2.z = -sp2->z;
        negs2.w = -sp2->w;
        src2 = &negs2;
    }
    cosom = src1->x * src2->x + src1->y * src2->y + src1->z * src2->z + src1->w * src2->w;
    if (cosom + 1.0f > 0.00001f) {
        if (1.0f - cosom > 0.00001f) {
            omega = acosBetter(cosom);
            sinom = __sinf(omega);
            sclp = __sinf((1.0f - t) * omega) / sinom;
            sclq = __sinf(t * omega) / sinom;
        } else {
            sclp = 1.0f - t;
            sclq = t;
        }
        dquat->x = sclp * src1->x + sclq * src2->x;
        dquat->y = sclp * src1->y + sclq * src2->y;
        dquat->z = sclp * src1->z + sclq * src2->z;
        dquat->w = sclp * src1->w + sclq * src2->w;
    } else {
        dquat->x = -src1->y;
        dquat->y = src1->x;
        dquat->z = -src1->w;
        dquat->w = src1->z;
        sclp = __sinf((1.0f - t) * 1.5707964f);
        sclq = __sinf(t * 1.5707964f);
        dquat->x = sclp * src1->x + sclq * dquat->x;
        dquat->y = sclp * src1->y + sclq * dquat->y;
        dquat->z = sclp * src1->z + sclq * dquat->z;
    }
}

s32 QuatSlerpAtFixedSpeed(Quat *src1, Quat *src2, f32 speed, Quat *dquat) {
    f32 tempFloat;
    f32 tempFloat2;

    tempFloat = (src1->x - src2->x) * (src1->x - src2->x) + (src1->y - src2->y) * (src1->y - src2->y)
                + (src1->z - src2->z) * (src1->z - src2->z) + (src1->w - src2->w) * (src1->w - src2->w);
    tempFloat2 = (src1->x + src2->x) * (src1->x + src2->x) + (src1->y + src2->y) * (src1->y + src2->y)
                 + (src1->z + src2->z) * (src1->z + src2->z) + (src1->w + src2->w) * (src1->w + src2->w);
    if (tempFloat < tempFloat2) {
        tempFloat = src1->x * src2->x + src1->y * src2->y + src1->z * src2->z + src1->w * src2->w;
    } else {
        tempFloat = -(src1->x * src2->x + src1->y * src2->y + src1->z * src2->z + src1->w * src2->w);
    }
    tempFloat = ABS(acos(tempFloat));
    if (speed < tempFloat) {
        QuatSlerp(src1, src2, speed / tempFloat, dquat);
        return FALSE;
    }
    *dquat = *src2;
    return TRUE;
}

s32 QuatSlerpAtFixedSpeedOld(Quat *src1, Quat *src2, f32 speed, Quat *dquat) {
    f32 tempFloat;
    f32 tempFloat2;

    tempFloat = (src1->x - src2->x) * (src1->x - src2->x) + (src1->y - src2->y) * (src1->y - src2->y)
                + (src1->z - src2->z) * (src1->z - src2->z) + (src1->w - src2->w) * (src1->w - src2->w);
    tempFloat2 = (src1->x + src2->x) * (src1->x + src2->x) + (src1->y + src2->y) * (src1->y + src2->y)
                 + (src1->z + src2->z) * (src1->z + src2->z) + (src1->w + src2->w) * (src1->w + src2->w);
    if (tempFloat2 < tempFloat) {
        tempFloat = src1->x * src2->x + src1->y * src2->y + src1->z * src2->z + src1->w * src2->w;
    } else {
        tempFloat = -(src1->x * src2->x + src1->y * src2->y + src1->z * src2->z + src1->w * src2->w);
    }
    tempFloat = ABS(acos(tempFloat));
    if (speed < tempFloat) {
        QuatSlerp(src1, src2, speed / tempFloat, dquat);
        return FALSE;
    }
    *dquat = *src2;
    return TRUE;
}

void QuaternionToMatrix(Quat *squat, Matrix *dmatrix) {
    f32 s;
    f32 xs, ys, zs;
    f32 wx, wy, wz;
    f32 xx, xy, xz;
    f32 yy, yz, zz;

    s = 2.0f / (DOT_PRODUCT(squat, squat) + squat->w * squat->w);
    xs = squat->x * s;
    ys = squat->y * s;
    zs = squat->z * s;
    wx = squat->w * xs;
    wy = squat->w * ys;
    wz = squat->w * zs;
    xx = squat->x * xs;
    xy = squat->x * ys;
    xz = squat->x * zs;
    yy = squat->y * ys;
    yz = squat->y * zs;
    zz = squat->z * zs;
    dmatrix->m[0][0] = 1.0f - (yy + zz);
    dmatrix->m[0][1] = xy + wz;
    dmatrix->m[0][2] = xz - wy;
    dmatrix->m[0][3] = 0.0f;
    dmatrix->m[1][0] = xy - wz;
    dmatrix->m[1][1] = 1.0f - (xx + zz);
    dmatrix->m[1][2] = yz + wx;
    dmatrix->m[1][3] = 0.0f;
    dmatrix->m[2][0] = xz + wy;
    dmatrix->m[2][1] = yz - wx;
    dmatrix->m[2][2] = 1.0f - (xx + yy);
    dmatrix->m[2][3] = 0.0f;
    dmatrix->m[3][0] = 0.0f;
    dmatrix->m[3][1] = 0.0f;
    dmatrix->m[3][2] = 0.0f;
    dmatrix->m[3][3] = 1.0f;
}

void MatrixToQuaternion(Matrix *smatrix, Quat *dquat) {
    f32 tr;
    f32 s;
    s32 i;
    s32 j;
    s32 k;
    f32 *fi;
    f32 *fj;
    f32 *fk;

    tr = smatrix->m[0][0] + smatrix->m[1][1] + smatrix->m[2][2];
    if (tr > 0.0f) {
        s = sqrtf(tr + 1.0f);
        dquat->w = s * 0.5f;
        s = 0.5f / s;
        dquat->x = (smatrix->m[1][2] - smatrix->m[2][1]) * s;
        dquat->y = (smatrix->m[2][0] - smatrix->m[0][2]) * s;
        dquat->z = (smatrix->m[0][1] - smatrix->m[1][0]) * s;
        return;
    }
    if (smatrix->m[0][0] < smatrix->m[1][1]) {
        if (smatrix->m[1][1] < smatrix->m[2][2]) {
            fi = &dquat->z;
            i = 2;
            fj = &dquat->x;
            j = 0;
            fk = &dquat->y;
            k = 1;
        } else {
            fi = &dquat->y;
            i = 1;
            fj = &dquat->z;
            j = 2;
            fk = &dquat->x;
            k = 0;
        }
    } else if (smatrix->m[0][0] < smatrix->m[2][2]) {
        fi = &dquat->z;
        i = 2;
        fj = &dquat->x;
        j = 0;
        fk = &dquat->y;
        k = 1;
    } else {
        fi = &dquat->x;
        i = 0;
        fj = &dquat->y;
        j = 1;
        fk = &dquat->z;
        k = 2;
    }
    s = sqrtf(smatrix->m[i][i] - (smatrix->m[j][j] + smatrix->m[k][k]) + 1.0f);
    *fi = s * 0.5f;
    s = 0.5f / s;
    dquat->w = (smatrix->m[j][k] - smatrix->m[k][j]) * s;
    *fj = (smatrix->m[i][j] + smatrix->m[j][i]) * s;
    *fk = (smatrix->m[i][k] + smatrix->m[k][i]) * s;
}

inline void QuaternionMultiply(Quat *dest, Quat *src1, Quat *src2) {
    Quat temp;

    temp.w = src1->w * src2->w - (src1->x * src2->x + src1->y * src2->y + src1->z * src2->z);
    temp.x = src1->w * src2->x + src2->w * src1->x + src1->y * src2->z - src1->z * src2->y;
    temp.y = src1->w * src2->y + src2->w * src1->y + src1->z * src2->x - src1->x * src2->z;
    temp.z = src1->w * src2->z + src2->w * src1->z + src1->x * src2->y - src1->y * src2->x;
    *dest = temp;
}

void GetRotationFromQuaternion(Quat *destQ, Quat *srcQ) {
    f32 sinThetaOver2;
    f32 len;
    f32 theta;

    theta = 2.0f * acosBetter(srcQ->w);
    sinThetaOver2 = __sinf(theta / 2.0f);
    destQ->w = theta;
    if (sinThetaOver2 != 0.0f) {
        destQ->x = srcQ->x / sinThetaOver2;
        destQ->y = srcQ->y / sinThetaOver2;
        destQ->z = srcQ->z / sinThetaOver2;
        len = LENGTH(destQ);
        if (len != 0.0f) {
            destQ->x /= len;
            destQ->y /= len;
            destQ->z /= len;
        }
    } else {
        destQ->x = 0.0f;
        destQ->y = 1.0f;
        destQ->z = 0.0f;
    }
}

inline void GetQuaternionFromRotation(Quat *destQ, Quat *srcQ) {
    f32 thetaOver2 = srcQ->w / 2.0f;
    f32 sinThetaOver2 = __sinf(thetaOver2);

    destQ->w = __cosf(thetaOver2);
    destQ->x = sinThetaOver2 * srcQ->x;
    destQ->y = sinThetaOver2 * srcQ->y;
    destQ->z = sinThetaOver2 * srcQ->z;
}

void AddOneScaledVector(Vec3 *result, Vec3 *operand1, Vec3 *operand2, f32 scale2) {
    result->x = operand1->x + operand2->x * scale2;
    result->y = operand1->y + operand2->y * scale2;
    result->z = operand1->z + operand2->z * scale2;
}

void AddToOneScaledVector(Vec3 *result, Vec3 *operand2, f32 scale2) {
    result->x = result->x + operand2->x * scale2;
    result->y = result->y + operand2->y * scale2;
    result->z = result->z + operand2->z * scale2;
}

void AddToOneScaledVector2D(Vec3 *result, Vec3 *operand2, f32 scale2) {
    result->x = result->x + operand2->x * scale2;
    result->z = result->z + operand2->z * scale2;
}

void AddOneScaledVector2D(Vec3 *result, Vec3 *operand1, Vec3 *operand2, f32 scale2) {
    result->x = operand1->x + operand2->x * scale2;
    result->z = operand1->z + operand2->z * scale2;
}

f32 DistanceBetweenPoints2D(Vec3 *v1, Vec3 *v2) {
    Vec3 d;

    SubVector2D(&d, v1, v2);
    return LENGTH_2D(&d);
}

f32 DistanceBetweenPoints2DSquared(Vec3 *a, Vec3 *b) {
    Vec3 d;

    SubVector2D(&d, a, b);
    return d.x * d.x + d.z * d.z;
}

f32 DistanceBetweenPoints(Vec3 *v1, Vec3 *v2) {
    Vec3 d;

    d.x = v1->x - v2->x;
    d.y = v1->y - v2->y;
    d.z = v1->z - v2->z;
    return LENGTH(&d);
}

f32 DistanceBetweenPointsSquared(Vec3 *v1, Vec3 *v2) {
    Vec3 d;

    d.x = v1->x - v2->x;
    d.y = v1->y - v2->y;
    d.z = v1->z - v2->z;
    return DOT_PRODUCT(&d, &d);
}

s32 IsPointWithinBox(Box *box, Vec3 *pos) {
    s32 ret = FALSE;

    if (pos->x >= box->pos.x && pos->x <= box->pos.x + box->size.x &&
        pos->y >= box->pos.y && pos->y <= box->pos.y + box->size.y &&
        pos->z >= box->pos.z && pos->z <= box->pos.z + box->size.z) {
        ret = TRUE;
    }
    return ret;
}

s32 IsPointWithinBoxPlusHeight(Box *box, Vec3 *pos, f32 height) {
    s32 ret = FALSE;

    if (pos->x >= box->pos.x && pos->x <= box->pos.x + box->size.x &&
        pos->y >= box->pos.y && pos->y <= box->pos.y + box->size.y + height &&
        pos->z >= box->pos.z && pos->z <= box->pos.z + box->size.z) {
        ret = TRUE;
    }
    return ret;
}

s32 IsPointWithinBox2D(Box *box, Vec3 *pos) {
    s32 ret = FALSE;

    if (pos->x >= box->pos.x && pos->x <= box->pos.x + box->size.x &&
        pos->z >= box->pos.z && pos->z <= box->pos.z + box->size.z) {
        ret = TRUE;
    }
    return ret;
}

void RotateVector2D(Vec3 *dest, Vec3 *source, f32 angle) {
    Vec3 temp;
    f32 cosangle = __cosf(-angle);
    f32 sinangle = __sinf(-angle);

    temp.y = source->y;
    temp.x = cosangle * source->x + -sinangle * source->z;
    temp.z = cosangle * source->z + sinangle * source->x;
    *dest = temp;
}

void RotateVector2DXYZ(Vec3 *dest, Vec3 *source, f32 angle, s8 axis) {
    Vec3 temp;
    s8 a;
    s8 b;

    switch (axis) {
        case 0:
            a = 1;
            b = 2;
            break;
        case 1:
            a = 2;
            b = 0;
            break;
        case 2:
            a = 0;
            b = 1;
            break;
    }
    (&temp.x)[axis] = (&source->x)[axis];
    (&temp.x)[a] = cosf(angle) * (&source->x)[a] + -sinf(angle) * (&source->x)[b];
    (&temp.x)[b] = cosf(angle) * (&source->x)[b] + sinf(angle) * (&source->x)[a];
    *dest = temp;
}

void FlipVector(Vec3 *vector) {
    vector->x = -vector->x;
    vector->y = -vector->y;
    vector->z = -vector->z;
}

void FlipVector2D(Vec3 *vector) {
    vector->x = -vector->x;
    vector->z = -vector->z;
}

void SetVector2D(Vec3 *v1, Vec3 *v2) {
    v1->x = v2->x;
    v1->z = v2->z;
}

void AddToVector2D(Vec3 *result, Vec3 *operand) {
    result->x = result->x + operand->x;
    result->z = result->z + operand->z;
}

void SubFromVector2D(Vec3 *result, Vec3 *operand) {
    result->x = result->x - operand->x;
    result->z = result->z - operand->z;
}

void AddVector2D(Vec3 *result, Vec3 *operand1, Vec3 *operand2) {
    result->x = operand1->x + operand2->x;
    result->z = operand1->z + operand2->z;
}

void AddScaledVector(Vec3 *result, Vec3 *operand1, f32 scale1, Vec3 *operand2, f32 scale2) {
    result->x = operand1->x * scale1 + operand2->x * scale2;
    result->y = operand1->y * scale1 + operand2->y * scale2;
    result->z = operand1->z * scale1 + operand2->z * scale2;
}

void AddScaledVector2D(Vec3 *result, Vec3 *operand1, f32 scale1, Vec3 *operand2, f32 scale2) {
    result->x = operand1->x * scale1 + operand2->x * scale2;
    result->z = operand1->z * scale1 + operand2->z * scale2;
}

void SubVector2D(Vec3 *result, Vec3 *operand1, Vec3 *operand2) {
    result->x = operand1->x - operand2->x;
    result->z = operand1->z - operand2->z;
}

f32 FindShortestAngle(f32 val1, f32 val2) {
    f32 temp = val1 - val2;
    f32 ret;

    if (ABS(temp) > PI) {
        ret = temp;
        if (val1 > val2) {
            ret = val2 - val1;
        }
        ret += TWOPI;
    } else {
        ret = ABS(temp);
    }
    return ret;
}

f32 FindShortestAngleSigned(f32 ang1, f32 ang2) {
    f32 temp = ang1 - ang2;

    if (temp > -PI && temp <= PI) {
        return temp;
    }
    if (temp > PI) {
        return (ang1 - ang2) - TWOPI;
    }
    return (ang1 + TWOPI) - ang2;
}

void MakeUnit(Vec3 *vect) {
    f32 len = LENGTH(vect);

    if (len != 0.0f) {
        vect->x /= len;
        vect->y /= len;
        vect->z /= len;
    }
}

inline void MakeUnit2D(Vec3 *vect) {
    f32 len = LENGTH_2D(vect);

    if (len != 0.0f) {
        vect->x /= len;
        vect->z /= len;
    }
}

void RoundToZero(f32 *f) {
    f32 absVal = *f;

    if (!(absVal > 0.0f)) {
        absVal = -absVal;
    }
    if (absVal < 0.0001) {
        *f = 0.0f;
    }
}

inline f32 Aabs(f32 angle) {
    angle = (angle > TWOPI) ? angle - TWOPI : angle;
    return (angle < 0.0f) ? angle + TWOPI : angle;
}

f32 SineWave(f32 freq, f32 phase, s32 base) {
    return __sinf((base + phase) / freq);
}

void InitMatrixStack(void) {
    s32 i, j, k;

    matrixStack.sp = 0;
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                matrixStack.stack[i].m[j][k] = 0.0f;
            }
        }
    }
    matrixStack.stack[0].m[0][0] = 1.0f;
    matrixStack.stack[0].m[1][1] = 1.0f;
    matrixStack.stack[0].m[2][2] = 1.0f;
    matrixStack.stack[0].m[3][3] = 1.0f;
}

void LoadMatrix(Matrix *m) {
    if (matrixStack.sp < 0) {
        matrixStack.sp = 0;
    }
    matrixStack.stack[matrixStack.sp] = *m;
}

void PushMatrix(Matrix *m) {
    u8 sp = ++matrixStack.sp;

    matrixStack.stack[sp] = *m;
    guMtxCatF(matrixStack.stack[sp].m, matrixStack.stack[sp - 1].m, matrixStack.stack[sp].m);
}

void PopMatrix(void) {
    if (matrixStack.sp == -1) {
        PRINTF("MatrixStack underflow\n");
        Crash("POP MATRIX");
    }
    matrixStack.sp--;
}

void ComputeResultMatrix(void) {
    s32 pad[4]; // forces the original 16-byte stack frame

    matrixStack.result = matrixStack.stack[matrixStack.sp];
}

void RotateVectorByQuaternion(Vec3 *result, Vec3 *vect, Quat *q) {
    Quat rot;

    GetRotationFromQuaternion(&rot, q);
    RotateVectorByRotation(result, vect, &rot);
}

void RotateVectorByRotation(Vec3 *result, Vec3 *vect, Quat *rot) {
    Vec3 par;
    Vec3 perp;
    Vec3 cross;
    f32 t;
    f32 len;
    f32 sinTheta;

    t = vect->x * rot->x;
    t += vect->y * rot->y;
    t += vect->z * rot->z;
    par.x = t * rot->x;
    par.y = t * rot->y;
    par.z = t * rot->z;
    SUB_VECTORS(&perp, vect, &par);
    cross.x = rot->y * perp.z - rot->z * perp.y;
    cross.y = rot->z * perp.x - rot->x * perp.z;
    cross.z = rot->x * perp.y - rot->y * perp.x;
    t = LENGTH(&perp);
    if (t == 0.0f) {
        *result = *vect;
        return;
    }
    len = LENGTH(&cross);
    if (len != 0.0f) {
        t /= len;
        SCALE_VECTOR(&cross, t);
    }
    t = __cosf(rot->w);
    sinTheta = __sinf(rot->w);
    result->x = par.x + t * perp.x + sinTheta * cross.x;
    result->y = par.y + t * perp.y + sinTheta * cross.y;
    t *= perp.z; // scaling t in place matches the original registers
    result->z = par.z + t + sinTheta * cross.z;
}

void NormalToQuaternion(Quat *q, Vec3 *normal) {
    f32 t;

    if (normal->x != 0.0f) {
        t = sqrtf(1.0f / (normal->z * normal->z / (normal->x * normal->x) + 1.0f));
        q->z = t;
        if (normal->x > 0.0f) {
            q->z = -t;
        }
        q->x = -(normal->z * q->z) / normal->x;
        q->w = acos(normal->y) / 2.0f;
        SCALE_VECTOR_2D(q, __sinf(q->w));
        q->w = __cosf(q->w);
    } else if (normal->z != 0.0f) {
        q->z = 0.0f;
        q->w = acos(normal->y) / 2.0f;
        if (normal->z > 0.0f) {
            q->x = __sinf(q->w);
        } else {
            q->x = -__sinf(q->w);
        }
        q->w = __cosf(q->w);
    } else if (normal->y > 0.0f) {
        q->x = 0.0f;
        q->z = 0.0f;
        q->w = 1.0f;
    } else {
        q->x = 0.0f;
        q->w = 0.0f;
        q->z = -1.0f;
    }
    q->y = 0;
}

void CalculateQuatForPlane(Actor *actor, Quat *qAim, Vec3 *plane) {
    Quat tempRot;
    Quat tempQ;
    Vec3 axis;

    if (plane->y == -1.0f) {
        vertQ.w = Aabs(TWOPI - actor->rot.y);
        tempRot.y = 0.0f;
        tempRot.z = 0.0f;
        tempRot.w = 3.1415927f;
        tempRot.x = 1.0f;
    } else {
        vertQ.w = actor->rot.y;
        axis.x = plane->x;
        axis.z = plane->z;
        axis.y = 0;
        MakeUnit2D(&axis);
        tempRot.y = 0.0f;
        tempRot.x = axis.z;
        tempRot.z = -axis.x;
        tempRot.w = acos(plane->y);
    }
    GetQuaternionFromRotation(qAim, &vertQ);
    GetQuaternionFromRotation(&tempQ, &tempRot);
    QuaternionMultiply(qAim, &tempQ, qAim);
}

INCLUDE_RODATA("asm/nonmatchings/maths", pad_80106434);
