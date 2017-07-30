#pragma once

#include "shared.h"
#include <math.h>
#include <assert.h>

#define SinF sinf
#define CosF cosf
#define SqrtF sqrtf
#define Fabs fabs
#define PI32 3.14159265359f
#define EPSILON 0.00000011920928955078125f // Copy of the one defined in float.h

union v2
{
    struct
    {
        r32 x, y;
    };

    r32 E[2];
};

union v3
{
    struct
    {
        r32 x, y, z;
    };

    struct
    {
        r32 r, g, b;
    };

    r32 E[3];
};

union v4
{
    struct
    {
        r32 x, y, z, w;
    };

    struct
    {
        r32 r, g, b, a;
    };

    r32 E[4];
};

union m4
{
    r32 E[4][4];
    r32 Array[16];
};

//
// General Math Functions
//

b32 EqFloats(r32 A, r32 B)
{
    b32 Result;

    Fabs(A - B) <= EPSILON ?  Result = true : Result = false;

    return (Result);
}

r32 ToRadians(r32 Degrees)
{
    r32 Result;

    Result = (Degrees * PI32) / 180.0f;

    return (Result);
}


//
// v2 Functions/Overloads
//

inline v2 operator+(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return (Result);
}

inline v2& operator+=(v2 &A, v2 B)
{
    A.x += B.x;
    A.y += B.y;
    return A;
}

inline v2 operator-(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;

    return (Result);
}

inline v2& operator-=(v2 &A, v2 B)
{
    A.x -= B.x;
    A.y -= B.y;
    return A;
}

inline v2& operator*=(v2 &A, r32 Scalar)
{
    A.x *= Scalar;
    A.y *= Scalar;

    return A;
}

inline v2 operator/(v2 Vec, r32 Scalar)
{
    v2 Result;

    Result.x = Vec.x / Scalar;
    Result.y = Vec.y / Scalar;

    return(Result);
}

inline v2 operator*(r32 A, v2 B)
{
    v2 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;

    return(Result);
}

inline v2 operator*(v2 B, r32 A)
{
    v2 Result = A * B;

    return(Result);
}

inline v2 V2(r32 x, r32 y)
{
    v2 Result;

    Result.x = x;
    Result.y = y;

    return (Result);
}

inline r32 DotProduct(v2 A, v2 B)
{
    r32 Result;

    Result = (A.x * B.x) + (A.y * B.y);

    return (Result);
}

inline r32 Magnitude(v2 A)
{
    r32 Result;

    Result = SqrtF(DotProduct(A, A));

    return (Result);
}

inline v2 Normalize(v2 A)
{
    v2 Result;

    r32 Length = Magnitude(A);

    Assert(!EqFloats(Length, 0));

    Result = A / Length;

    return (Result);
}

v2 Project(v2 A, v2 B)
{
    // TODO: Check if A is not normalized and the result is alswo the same
    v2 Result = {};

    Result.x = DotProduct(A, B) * B.x;
    Result.y = DotProduct(A, B) * B.y;

#if _DEBUG
    if(Magnitude(B) > 1.0f)
    {
        printf("ERROR in ProjectV2, Second argument must be normalized\n");
    }
#endif
    return (Result);
}

v2 RightHandNormal(v2 A)
{
    v2 Result;

    Result.x = -A.y;
    Result.y = A.x;

    return (Result);
}

b32 NullVec(v2 A)
{
    if(A.x != 0 || A.y != 0)
        return false;

    return true;
}


//
// v3 Functions/Overloads
//

inline v3 V3(r32 x, r32 y, r32 z)
{
    v3 Result;

    Result.x = x;
    Result.y = y;
    Result.z = z;

    return (Result);
}

inline v3 operator+(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return (Result);
}

inline v3& operator+=(v3 &A, v3 B)
{
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
}

inline v3 operator*(r32 A, v3 B)
{
    v3 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;

    return(Result);
}

inline v3 operator*(v3 B, r32 A)
{
    v3 Result = A * B;

    return(Result);
}

inline v3 operator/(v3 Vec, r32 Scalar)
{
    v3 Result;

    Result.x = Vec.x / Scalar;
    Result.y = Vec.y / Scalar;
    Result.z = Vec.z / Scalar;

    return(Result);
}

inline r32 DotProduct(v3 A, v3 B)
{
    r32 Result;

    Result = (A.x * B.x) + (A.y * B.y) + (A.z * B.z);

    return (Result);
}

inline r32 Magnitude(v3 A)
{
    r32 Result;

    Result = SqrtF(DotProduct(A, A));

    return (Result);
}

inline v3 Normalize(v3 A)
{
    v3 Result;

    r32 Length = Magnitude(A);

    Assert(!EqFloats(Length, 0));

    Result = A / Length;

    return (Result);
}

//
// m4 Functions/Overloads
//

inline m4 M4()
{
    m4 Result = {};

    Result.E[0][0] = 1.0f;
    Result.E[1][1] = 1.0f;
    Result.E[2][2] = 1.0f;
    Result.E[3][3] = 1.0f;

#if _DEBUG
    // TODO: Identity Test
#endif

    return (Result);
}

inline m4 Transpose(m4 A)
{
    m4 Result = {};

    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            Result.E[j][i] = A.E[i][j];
        }
    }

#if _DEBUG
    // TODO: Test
#endif

    return (Result);
}

inline m4 Scale(v3 A)
{
    m4 Result = {};

    Result.E[0][0] = A.x;
    Result.E[1][1] = A.y;
    Result.E[2][2] = A.z;
    Result.E[3][3] = 1.0f;

#if _DEBUG
    // TODO: Test
#endif

    return (Result);
}

m4 Translate(m4 A, v3 V)
{
    m4 Result = A;

    Result.E[3][0] += V.x;
    Result.E[3][1] += V.y;
    Result.E[3][2] += V.z;

    return (Result);
}

m4 operator*(m4 Left, m4 Right)
{
    m4 Result = M4();

    for(i32 Columns = 0; Columns < 4; Columns++)
    {
        for(i32 Rows = 0; Rows < 4; Rows++)
        {
            r32 Sum = 0;
            for(i32 CurrentMatrice = 0; CurrentMatrice < 4; ++CurrentMatrice)
            {
                Sum += Left.E[CurrentMatrice][Rows] * Right.E[Columns][CurrentMatrice];
            }

            Result.E[Columns][Rows] = Sum;
        }
    }

    return (Result);
}

m4 Ortho(r32 Left, r32 Right, r32 Bottom, r32 Top, r32 Near, r32 Far)
{
    m4 Result = M4();

    Result.E[0][0] = 2.0f / (Right - Left);
    Result.E[1][1] = 2.0f / (Top - Bottom);
    Result.E[2][2] = 2.0f / (Near - Far);

    Result.E[3][0] = (Left + Right) / (Left - Right);
    Result.E[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.E[3][2] = (Far + Near) / (Near - Far);

    return (Result);
}

m4 Rotate(r32 Angle, v3 Axis)
{
    m4 Result = M4();

    Axis = Normalize(Axis);

    r32 SinTheta = SinF(ToRadians(Angle));
    r32 CosTheta = CosF(ToRadians(Angle));
    r32 CosValue = 1.0f - CosTheta;

    Result.E[0][0] = (Axis.x * Axis.x * CosValue) + CosTheta;
    Result.E[0][1] = (Axis.x * Axis.y * CosValue) + (Axis.z * SinTheta);
    Result.E[0][2] = (Axis.x * Axis.z * CosValue) - (Axis.y * SinTheta);

    Result.E[1][0] = (Axis.y * Axis.x * CosValue) - (Axis.z * SinTheta);
    Result.E[1][1] = (Axis.y * Axis.y * CosValue) + CosTheta;
    Result.E[1][2] = (Axis.y * Axis.z * CosValue) + (Axis.x * SinTheta);

    Result.E[2][0] = (Axis.z * Axis.x * CosValue) + (Axis.y * SinTheta);
    Result.E[2][1] = (Axis.z * Axis.y * CosValue) - (Axis.x * SinTheta);
    Result.E[2][2] = (Axis.z * Axis.z * CosValue) + CosTheta;

    #ifdef _DEBUG
    // TODO: Test
    #endif

    return (Result);
}

//
// Debug Functions
//

void DebugPrintVec(v2 A)
{
    printf("x: %f  y: %f\n", A.x, A.y);
}

void DebugPrintVec(v3 A)
{
    printf("x: %f  y: %f  z: %f\n", A.x, A.y, A.z);
}

void DebugPrintMatrix(m4 M)
{
    printf("Row 0: %f  %f  %f  %f\n", M.E[0][0], M.E[0][1], M.E[0][2], M.E[0][3]);
    printf("Row 1: %f  %f  %f  %f\n", M.E[1][0], M.E[1][1], M.E[1][2], M.E[1][3]);
    printf("Row 2: %f  %f  %f  %f\n", M.E[2][0], M.E[2][1], M.E[2][2], M.E[2][3]);
    printf("Row 3: %f  %f  %f  %f\n", M.E[3][0], M.E[3][1], M.E[3][2], M.E[3][3]);
}
