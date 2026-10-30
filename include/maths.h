#ifndef MATHS_H
#define MATHS_H

#include "types.h"

typedef struct Matrix {
    f32 m[4][4];
} Matrix;

typedef struct MatrixStack {
    Matrix stack[20];
    s16 sp;
    Matrix result;
} MatrixStack;

extern MatrixStack matrixStack;

s32 Random(s32 lim);
void SeedRandom(void);
void CrossProduct(Vec3 *result, Vec3 *operand1, Vec3 *operand2);
void CrossProductVertical(Vec3 *result, Vec3 *operand2);
void AddOneScaledVector(Vec3 *result, Vec3 *operand1, Vec3 *operand2, f32 scale2);
void AddToOneScaledVector(Vec3 *result, Vec3 *operand2, f32 scale2);
void AddToOneScaledVector2D(Vec3 *result, Vec3 *operand2, f32 scale2);
void AddOneScaledVector2D(Vec3 *result, Vec3 *operand1, Vec3 *operand2, f32 scale2);
f32 DistanceBetweenPoints2D(Vec3 *v1, Vec3 *v2);
f32 DistanceBetweenPoints2DSquared(Vec3 *a, Vec3 *b);
f32 DistanceBetweenPoints(Vec3 *v1, Vec3 *v2);
f32 DistanceBetweenPointsSquared(Vec3 *v1, Vec3 *v2);
s32 IsPointWithinBox(Box *box, Vec3 *pos);
s32 IsPointWithinBoxPlusHeight(Box *box, Vec3 *pos, f32 height);
s32 IsPointWithinBox2D(Box *box, Vec3 *pos);
void FlipVector(Vec3 *vector);
void FlipVector2D(Vec3 *vector);
void SetVector2D(Vec3 *v1, Vec3 *v2);
void AddToVector2D(Vec3 *result, Vec3 *operand);
void SubFromVector2D(Vec3 *result, Vec3 *operand);
void AddVector2D(Vec3 *result, Vec3 *operand1, Vec3 *operand2);
void AddScaledVector(Vec3 *result, Vec3 *operand1, f32 scale1, Vec3 *operand2, f32 scale2);
void AddScaledVector2D(Vec3 *result, Vec3 *operand1, f32 scale1, Vec3 *operand2, f32 scale2);
void SubVector2D(Vec3 *result, Vec3 *operand1, Vec3 *operand2);
f32 FindShortestAngle(f32 val1, f32 val2);
f32 FindShortestAngleSigned(f32 ang1, f32 ang2);
void MakeUnit(Vec3 *vect);
void RotateVector2D(Vec3 *dest, Vec3 *source, f32 angle);
void RotateVector2DXYZ(Vec3 *dest, Vec3 *source, f32 angle, s8 axis);
void MakeUnit2D(Vec3 *vect);
void RoundToZero(f32 *f);
void RotateVectorByQuaternion(Vec3 *result, Vec3 *vect, Quat *q);
f32 Aabs(f32 angle);
f32 atan2(f32 y, f32 x);
f32 acos(f32 x);
void QuatSlerp(Quat *src1, Quat *sp2, f32 t, Quat *dquat);
void QuaternionMultiply(Quat *dest, Quat *src1, Quat *src2);
void GetQuaternionFromRotation(Quat *destQ, Quat *srcQ);
f32 SineWave(f32 freq, f32 phase, s32 base);
void InitMatrixStack(void);
void LoadMatrix(Matrix *m);
void PushMatrix(Matrix *m);
void PopMatrix(void);
void ComputeResultMatrix(void);

void InitAtan(void);
s32 QuatSlerpAtFixedSpeed(Quat *src1, Quat *src2, f32 speed, Quat *dquat);
s32 QuatSlerpAtFixedSpeedOld(Quat *src1, Quat *src2, f32 speed, Quat *dquat);
void GetRotationFromQuaternion(Quat *destQ, Quat *srcQ);
void RotateVectorByRotation(Vec3 *result, Vec3 *vect, Quat *rot);
void NormalToQuaternion(Quat *q, Vec3 *normal);

#endif
