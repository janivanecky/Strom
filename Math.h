#pragma once
#include <stdint.h>

#define PI 3.14159265359f
#define PI2 6.28318530718f
#define PIHALF 1.57079632679f

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) \
			((sizeof(a) / sizeof(*(a))) / \
			static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

struct Vector2
{
	float x;
	float y;

	Vector2()
	{
		this->x = 0;
		this->y = 0;
	}

	Vector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	const Vector2 operator-(const Vector2 v2)
	{
		return Vector2(this->x - v2.x, this->y - v2.y);
	}

	const Vector2 operator-(const Vector2 v2) const
	{
		return Vector2(this->x - v2.x, this->y - v2.y);
	}

	const Vector2 operator-()
	{
		return Vector2(-this->x, -this->y);
	}

	const Vector2 operator*(const float v)
	{
		return Vector2(this->x * v, this->y * v);
	}

	const Vector2 operator*(const float v) const
	{
		return Vector2(this->x * v, this->y * v);
	}

	const Vector2 operator/(const float v)
	{
		return Vector2(this->x / v, this->y / v);
	}

	const Vector2 operator+(const Vector2 v2)
	{
		return Vector2(this->x + v2.x, this->y + v2.y);
	}

	const Vector2& operator+=(const Vector2 v2)
	{
		this->x += v2.x;
		this->y += v2.y;
		return *this;
	}

	const Vector2& operator*=(const float x)
	{
		this->x *= x;
		this->y *= x;
		return *this;
	}

	const Vector2 operator+(const Vector2 v2) const
	{
		return Vector2(this->x + v2.x, this->y + v2.y);
	}

};

struct Vector2i
{
	int x;
	int y;

	Vector2i()
	{
		this->x = 0;
		this->y = 0;
	}

	Vector2i(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	const Vector2i operator+(const Vector2i v2)
	{
		return Vector2i(this->x + v2.x, this->y + v2.y);
	}
};

struct Vector3i
{
	int x;
	int y;
	int z;

	Vector3i()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vector3i(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	const Vector3i operator+(const Vector3i v3)
	{
		return Vector3i(this->x + v3.x, this->y + v3.y, this->z + v3.z);
	}

	bool operator==(const Vector3i v3)
	{
		return (v3.x == this->x && v3.y == this->y && v3.z == this->z);
	}

	bool operator!=(const Vector3i v3)
	{
		return !(*this == v3);
	}
};

struct Vector4;
struct Vector3
{
	float x;
	float y;
	float z;

	Vector3()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vector3(float x, float y, float z);
	Vector3(Vector4 vec);

	Vector3 operator*(float x);

	Vector3 operator/(float x);

	Vector3 operator+(Vector3 x);
	Vector3 operator-(Vector3 x);

	Vector3 operator+=(Vector3 x);
	Vector3 operator-=(Vector3 x);

	Vector3 operator-();
	bool operator==(Vector3 x);
};

struct Vector4
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		struct
		{
			Vector3 xyz;
			float w;
		};
	};


	Vector4() :x(0), y(0), z(0), w(0) {};
	Vector4(Vector3 vec, float w);

	Vector4(float x, float y, float z, float w);

	Vector4 operator -();
	Vector4 operator +=(Vector4);
	Vector4 operator -=(Vector4);
	Vector4 operator *(float x);
	Vector4 operator *(float x) const;
	Vector4 operator +(Vector4);
	Vector4 operator *=(float x);
	Vector4 operator /=(float x);
};

Vector3 operator*(float x, Vector3 v);

struct Rect
{
	float left;
	float top;
	float width;
	float height;

	Rect()
	{
		this->left = 0;
		this->top = 0;
		this->width = 0;
		this->height = 0;
	}

	Rect(float left, float top, float width, float height)
	{
		this->left = left;
		this->top = top;
		this->width = width;
		this->height = height;
	}
};

struct Rectangle
{
	float x;
	float y;
	float width;
	float height;
	Rectangle()
	{
		this->x = 0;
		this->y = 0;
		this->width = 0;
		this->height = 0;
	}

	Rectangle(float x, float y, float width, float height)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}
};

struct Matrix4x4
{
	union
	{
		float x[16];
		struct
		{
			Vector4 v1;
			Vector4 v2;
			Vector4 v3;
			Vector4 v4;
		};
	};

	Matrix4x4() :
		x{ 0 }
	{

	}


	const float& operator[](int index) const
	{
		return x[index];
	}

	float& operator[](int index)
	{
		return x[index];
	}

	const Matrix4x4 operator *(const Matrix4x4 m)
	{
		Matrix4x4 result;
		result[0] = x[0] * m[0] + x[4] * m[1] + x[8] * m[2] + x[12] * m[3];
		result[1] = x[1] * m[0] + x[5] * m[1] + x[9] * m[2] + x[13] * m[3];
		result[2] = x[2] * m[0] + x[6] * m[1] + x[10] * m[2] + x[14] * m[3];
		result[3] = x[3] * m[0] + x[7] * m[1] + x[11] * m[2] + x[15] * m[3];

		result[4] = x[0] * m[4] + x[4] * m[5] + x[8] * m[6] + x[12] * m[7];
		result[5] = x[1] * m[4] + x[5] * m[5] + x[9] * m[6] + x[13] * m[7];
		result[6] = x[2] * m[4] + x[6] * m[5] + x[10] * m[6] + x[14] * m[7];
		result[7] = x[3] * m[4] + x[7] * m[5] + x[11] * m[6] + x[15] * m[7];

		result[8] = x[0] * m[8] + x[4] * m[9] + x[8] * m[10] + x[12] * m[11];
		result[9] = x[1] * m[8] + x[5] * m[9] + x[9] * m[10] + x[13] * m[11];
		result[10] = x[2] * m[8] + x[6] * m[9] + x[10] * m[10] + x[14] * m[11];
		result[11] = x[3] * m[8] + x[7] * m[9] + x[11] * m[10] + x[15] * m[11];

		result[12] = x[0] * m[12] + x[4] * m[13] + x[8] * m[14] + x[12] * m[15];
		result[13] = x[1] * m[12] + x[5] * m[13] + x[9] * m[14] + x[13] * m[15];
		result[14] = x[2] * m[12] + x[6] * m[13] + x[10] * m[14] + x[14] * m[15];
		result[15] = x[3] * m[12] + x[7] * m[13] + x[11] * m[14] + x[15] * m[15];
		return result;
	}

	const Matrix4x4 operator *(const Matrix4x4 m) const
	{
		Matrix4x4 result;
		result[0] = x[0] * m[0] + x[4] * m[1] + x[8] * m[2] + x[12] * m[3];
		result[1] = x[1] * m[0] + x[5] * m[1] + x[9] * m[2] + x[13] * m[3];
		result[2] = x[2] * m[0] + x[6] * m[1] + x[10] * m[2] + x[14] * m[3];
		result[3] = x[3] * m[0] + x[7] * m[1] + x[11] * m[2] + x[15] * m[3];

		result[4] = x[0] * m[4] + x[4] * m[5] + x[8] * m[6] + x[12] * m[7];
		result[5] = x[1] * m[4] + x[5] * m[5] + x[9] * m[6] + x[13] * m[7];
		result[6] = x[2] * m[4] + x[6] * m[5] + x[10] * m[6] + x[14] * m[7];
		result[7] = x[3] * m[4] + x[7] * m[5] + x[11] * m[6] + x[15] * m[7];

		result[8] = x[0] * m[8] + x[4] * m[9] + x[8] * m[10] + x[12] * m[11];
		result[9] = x[1] * m[8] + x[5] * m[9] + x[9] * m[10] + x[13] * m[11];
		result[10] = x[2] * m[8] + x[6] * m[9] + x[10] * m[10] + x[14] * m[11];
		result[11] = x[3] * m[8] + x[7] * m[9] + x[11] * m[10] + x[15] * m[11];

		result[12] = x[0] * m[12] + x[4] * m[13] + x[8] * m[14] + x[12] * m[15];
		result[13] = x[1] * m[12] + x[5] * m[13] + x[9] * m[14] + x[13] * m[15];
		result[14] = x[2] * m[12] + x[6] * m[13] + x[10] * m[14] + x[14] * m[15];
		result[15] = x[3] * m[12] + x[7] * m[13] + x[11] * m[14] + x[15] * m[15];
		return result;
	}

	// matrix * vector
	const Vector4 operator * (const Vector4 v)
	{
		Vector4 result;

		// result.x by sa malo rovnat this->v1 dot v, ale teraz to tak nebude pretoze this-v1 == Vector4(x[0], x[1], x[2], x[3]
		// a tu sa pouziva (stlpcovy) vektor x[0], x[4], x[8], x[12]

		// pri column major maticiach by davalo vacsi zmysel, ak by bola operacia * definovana pre Vector4 a nie pre Matrix4x4,
		// kedze na papieri je poradie taketo: vektor * matica (pre column major maticu a row vektor)
		// a u nas v kode to vyzera takto: matica * vektor (na papieri pre row major maticu a column vector)
		result.x = x[0] * v.x + x[4] * v.y + x[8] * v.z + x[12] * v.w;
		result.y = x[1] * v.x + x[5] * v.y + x[9] * v.z + x[13] * v.w;
		result.z = x[2] * v.x + x[6] * v.y + x[10] * v.z + x[14] * v.w;
		result.w = x[3] * v.x + x[7] * v.y + x[11] * v.z + x[15] * v.w;
		return result;
	}

	const Vector4 operator * (const Vector4 v) const
	{
		Vector4 result;

		result.x = x[0] * v.x + x[4] * v.y + x[8] * v.z + x[12] * v.w;
		result.y = x[1] * v.x + x[5] * v.y + x[9] * v.z + x[13] * v.w;
		result.z = x[2] * v.x + x[6] * v.y + x[10] * v.z + x[14] * v.w;
		result.w = x[3] * v.x + x[7] * v.y + x[11] * v.z + x[15] * v.w;
		return result;
	}

	const Vector3 operator *(const Vector3 v)
	{
		Vector4 result = (*this) * Vector4(v, 0.0f);
		return Vector3(result.x, result.y, result.z);
	}

};

namespace Math
{
	const float Sin(const float t);
	const float Cos(const float t);
	float Acos(const float t);
	const float Tan(float t);
	float Fmod(float x, float d);

	int32_t Sign(float x);
	float Abs(float x);
	float Floor(float x);
	float Ceil(float x);
	float Sqrt(float x);
	float Pow(float base, float exponent);
	float Clamp(float x, float lower, float higher);
	float Smoothstep(float edge1, float edge2, float x);
	float Mix(float edge1, float edge2, float x);
	float Deg2Rad(float degrees);

	float Random(float min = 0.0f, float max = 1.0f);

	bool FloatEqual(float a, float b, float epsilon = 0.0001f);
	bool FloatEqual(Vector3 v1, Vector3 v2, float epsilon = 0.0001f);
	uint32_t Max(uint32_t a, uint32_t b);
	float Max(float a, float b);

	Vector3 RotateAroundZ(Vector3 v, float angle);
	Matrix4x4 Invert(Matrix4x4);

	float Length(Vector2 v);
	float Dot(Vector2 v1, Vector2 v2);
	float Dot(Vector3 v1, Vector3 v2);
	Vector2 Normalize(Vector2 v);
	Vector2 Rotate(float angle, Vector2 v);
	float Distance(Vector2 v1, Vector2 v2);
	float Distance(Vector3 v1, Vector3 v2);

	float AngularDistance(float alpha, float beta);

	float ExponentialFunction(float amplitude, float coef, float t);

	const Matrix4x4 GetOrthographicsProjectionDXRH(float left, float right, float bottom, float top,
												 float zNear, float zFar);
	const Matrix4x4 GetPerspectiveProjectionDXRH(const float fov, const float aspectRatio,
												 const float zNear, const float zFar);
	const Matrix4x4 GetPerspectiveProjectionDXLH(const float fov, const float aspectRatio,
												 const float zNear, const float zFar);
	Matrix4x4 LookAt(Vector3 eye, Vector3 target, Vector3 up);
	Matrix4x4 Transpose(const Matrix4x4& m);
	Matrix4x4 GetTranslation(Vector3 vec);
	Matrix4x4 GetTranslation(const float dx, const float dy, const float dz);
	Matrix4x4 GetRotation(const float angle, Vector3 axis);
	Matrix4x4 GetIdentity();
	Matrix4x4 GetReflectionY();
	Matrix4x4 GetScale(const float sx, const float sy, const float sz);

	Vector3 Normalize(Vector3 v);
	float Length(Vector3 v);
	Vector3 CrossProduct(Vector3 v1, Vector3 v2);

}

#undef near
#undef far

#ifdef DRAW

float Math::Random(float min, float max)
{
	return (rand() % 1000) / 1000.0f * (max - min) + min;
}

const Matrix4x4 Math::GetOrthographicsProjectionDXRH(float left, float right,
												   float bottom, float top,
												   float near, float far)
{
	Matrix4x4 result;
	result[0] = 2.0f / (right - left);
	result[5] = 2.0f / (top - bottom);
	result[10] = 1.0f / (far - near);
	result[12] = -(right + left) / (right - left);
	result[13] = -(top + bottom) / (top - bottom);
	result[14] = -(near) / (far - near);
	result[15] = 1;
	return result;
}

Matrix4x4 Math::LookAt(Vector3 eye, Vector3 target, Vector3 up)
{
	Matrix4x4 matrix;
	Vector3 x, y, z;

	z = eye - target;
	z = Math::Normalize(z);

	x = Math::CrossProduct(up, z);
	x = Math::Normalize(x);
	y = Math::CrossProduct(z, x);

	y = Math::Normalize(y);

	matrix.v1 = Vector4(x.x, y.x, z.x, 0);
	matrix.v2 = Vector4(x.y, y.y, z.y, 0);
	matrix.v3 = Vector4(x.z, y.z, z.z, 0);
	matrix.v4 = Vector4(-Math::Dot(x, eye),
						-Math::Dot(y, eye),
						-Math::Dot(z, eye),
						1.0f);
	return matrix;
}

#include <math.h>
const Matrix4x4 Math::GetPerspectiveProjectionDXRH(const float fov, const float aspectRatio,
												   const float near, const float far)
{
	float cotFov = 1 / (tanf(fov / 2.0f));

	Matrix4x4 result;

	result[0] = cotFov / aspectRatio;
	result[5] = cotFov;
	result[10] = -far / (far - near);
	result[11] = -1.0f;
	result[14] = -near * far / (far - near);

	return result;
}

const Matrix4x4 Math::GetPerspectiveProjectionDXLH(const float fov, const float aspectRatio,
												   const float near, const float far)
{
	float cotFov = 1 / (tanf(fov / 2.0f));

	Matrix4x4 result;

	result[0] = cotFov / aspectRatio;
	result[5] = cotFov;
	result[10] = far / (far - near);
	result[11] = 1.0f;
	result[14] = -near * far / (far - near);

	return result;
}

Vector3 Math::CrossProduct(Vector3 v1, Vector3 v2)
{
	Vector3 crossProduct;
	crossProduct.x = v1.y * v2.z - v1.z * v2.y;
	crossProduct.y = v1.z * v2.x - v1.x * v2.z;
	crossProduct.z = v1.x * v2.y - v1.y * v2.x;
	return crossProduct;
}

Vector3 Math::RotateAroundZ(Vector3 v, float angle)
{
	Vector3 vec;
	vec.x = v.x * cosf(angle) - v.y * sinf(angle);
	vec.y = v.x * sinf(angle) + v.y * cosf(angle);
	vec.z = v.z;
	return vec;
}


Matrix4x4 Math::Transpose(const Matrix4x4& m)
{
	/*
	0 4 8  12
	1 5 9  13
	2 6 10 14
	3 7 11 15
	*/
	Matrix4x4 r = m;

	r[1] = m[4];
	r[2] = m[8];
	r[3] = m[12];

	r[4] = m[1];
	r[6] = m[9];
	r[7] = m[13];

	r[8] = m[2];
	r[9] = m[6];
	r[11] = m[14];

	r[12] = m[3];
	r[13] = m[7];
	r[14] = m[11];

	return r;
}

Matrix4x4 Math::GetTranslation(Vector3 vec)
{
	return Math::GetTranslation(vec.x, vec.y, vec.z);
}

Matrix4x4 Math::GetTranslation(const float dx, const float dy, const float dz)
{
	Matrix4x4 result;

	result[0] = 1;
	result[5] = 1;
	result[10] = 1;
	result[12] = dx;
	result[13] = dy;
	result[14] = dz;
	result[15] = 1;

	return result;
}

Matrix4x4 Math::Invert(Matrix4x4 m)
{
	Matrix4x4 inv;
	float det;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return Math::GetIdentity();

	det = 1.0f / det;

	Matrix4x4 result;
	for (int i = 0; i < 16; i++)
		result[i] = inv[i] * det;

	return result;
}

Matrix4x4 Math::GetRotation(const float angle, Vector3 axis)
{
	float c = Cos(angle);
	float s = Sin(angle);

	Matrix4x4 result;
	float ax = axis.x;
	float ay = axis.y;
	float az = axis.z;
	result[0] = c + (1 - c) * ax * ax;
	result[1] = (1 - c) * ax * ay + s * az;
	result[2] = (1 - c) * ax * az - s * ay;
	result[3] = 0;

	result[4] = (1 - c) * ax * ay - s * az;
	result[5] = c + (1 - c) * ay * ay;
	result[6] = (1 - c) * ay * az + s * ax;
	result[7] = 0;

	result[8] = (1 - c) * ax * az + s * ay;
	result[9] = (1 - c) * ay * az - s * ax;
	result[10] = c + (1 - c) * az * az;
	result[11] = 0;

	result[12] = 0;
	result[13] = 0;
	result[14] = 0;
	result[15] = 1;

	return result;
}

Matrix4x4 Math::GetIdentity()
{
	Matrix4x4 result;
	result.v1 = Vector4(1, 0, 0, 0);
	result.v2 = Vector4(0, 1, 0, 0);
	result.v3 = Vector4(0, 0, 1, 0);
	result.v4 = Vector4(0, 0, 0, 1);
	return result;
}

Matrix4x4 Math::GetReflectionY()
{
	Matrix4x4 result = GetIdentity();
	result.v2 = -result.v2;
	return result;
}

Matrix4x4 Math::GetScale(const float sx, const float sy, const float sz)
{
	Matrix4x4 result = GetIdentity();
	result[0] = sx;
	result[5] = sy;
	result[10] = sz;
	return result;
}

float Math::Abs(float x)
{
	return x < 0 ? -x : x;
}

short Floor(float x)
{
	return (short)(x + 32768.0f) - 32768;
}

const float Math::Tan(float t)
{
	return tanf(t);
}

const float Math::Sin(const float t)
{
	return sinf(t);
}

const float Math::Cos(const float t)
{
	return Sin(t + PIHALF);
}

float Math::Acos(const float t)
{
	return acosf(t);
}

float Math::Fmod(float x, float d)
{
	if (d == 0)
	{
		return 0;
	}
	while (x >= d)
	{
		x -= d;
	}
	return x;
}

float Math::AngularDistance(float alpha, float beta)
{
#if 0
	float d = Abs(alpha - beta);
	if (d > PI)
		d = PI2 - d;
	return d;
#endif
	return 0;
}

float Math::Length(Vector2 v)
{
	float result = sqrtf(v.x * v.x + v.y * v.y);
	return result;
}

Vector2 Math::Rotate(float angle, Vector2 v)
{
	Vector2 result;
	result.x = Math::Cos(angle) * v.x - Math::Sin(angle) * v.y;
	result.y = Math::Sin(angle) * v.x + Math::Cos(angle) * v.y;
	return result;
}

float Math::Length(Vector3 v)
{
	float result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return result;
}

Vector3 Math::Normalize(Vector3 v)
{
	// TODO: if v = 0, 0, 0, ?
	Vector3 result = v / Length(v);
	return result;
}

float Math::Sqrt(float x)
{
	return sqrtf(x);
}

float Math::Deg2Rad(float degrees)
{
	float result = degrees *  PI / 180;
	return result;
}

uint32_t Math::Max(uint32_t a, uint32_t b)
{
	if (a > b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

float Math::Max(float a, float b)
{
	if (a > b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

int32_t Math::Sign(float x)
{
	if (Math::FloatEqual(x, 0))
	{
		return 0;
	}
	else if (x > 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

bool Math::FloatEqual(float a, float b, float epsilon)
{
	bool result = Math::Abs(a - b) < epsilon;
	return result;
}

bool Math::FloatEqual(Vector3 v1, Vector3 v2, float epsilon)
{
	bool result = Math::FloatEqual(v1.x, v2.x, epsilon) &&
		Math::FloatEqual(v1.y, v2.y, epsilon) &&
		Math::FloatEqual(v1.z, v2.z, epsilon);
	return result;
}

float Math::Dot(Vector2 v1, Vector2 v2)
{
	float result = v1.x * v2.x + v1.y * v2.y;
	return result;
}

float Math::Dot(Vector3 v1, Vector3 v2)
{
	float result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return result;
}

float Math::Distance(Vector2 v1, Vector2 v2)
{
	float result = Math::Length(v1 - v2);
	return result;
}

float Math::Distance(Vector3 v1, Vector3 v2)
{
	float result = Math::Length(v1 - v2);
	return result;
}

Vector2 Math::Normalize(Vector2 v)
{
	Vector2 result = v / Length(v);
	return result;
}


float Math::Floor(float x)
{
	return floorf(x);
}

float Math::Ceil(float x)
{
	return ceilf(x);
}

float Math::Pow(float base, float exponent)
{
	float result = powf(base, exponent);
	return result;
}

float Math::Smoothstep(float edge1, float edge2, float x)
{
	x = Clamp((x - edge1) / (edge2 - edge1), 0.0, 1.0);
	return x*x*(3 - 2 * x);
}

float Math::Mix(float edge1, float edge2, float x)
{
	return edge1 * (1 - x) + edge2 * x;
}

float Math::ExponentialFunction(float amplitude, float coef, float t)
{
	float result = amplitude * Math::Pow(2.0f, coef * t);
	return result;
}

float Math::Clamp(float x, float lower, float higher)
{
	if (x < lower)
		return lower;
	if (x > higher)
		return higher;
	return x;
}

Vector3::Vector3(Vector4 vec)
{
	this->x = vec.x;
	this->y = vec.y;
	this->z = vec.z;

}

Vector3::Vector3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3 Vector3::operator*(float x)
{
	return Vector3(this->x * x, this->y * x, this->z * x);
}

Vector3 operator*(float x, Vector3 v)
{
	return Vector3(v.x * x, v.y * x, v.z * x);
}

Vector3 Vector3::operator/(float x)
{
	return Vector3(this->x / x, this->y / x, this->z / x);
}

Vector3 Vector3::operator+(Vector3 x)
{
	return Vector3(this->x + x.x, this->y + x.y, this->z + x.z);
}

Vector3 Vector3::operator-(Vector3 x)
{
	return Vector3(this->x - x.x, this->y - x.y, this->z - x.z);
}

Vector3 Vector3::operator+=(Vector3 x)
{
	this->x += x.x;
	this->y += x.y;
	this->z += x.z;
	return *this;
}

Vector3 Vector3::operator-=(Vector3 x)
{
	this->x -= x.x;
	this->y -= x.y;
	this->z -= x.z;
	return *this;
}

Vector3 Vector3::operator-()
{
	return Vector3(-this->x, -this->y, -this->z);
}

bool Vector3::operator==(const Vector3 v)
{
	return (this->x == v.x) && (this->y == v.y) && (this->z == v.z);
}

Vector4::Vector4(Vector3 vec, float w)
{
	this->x = vec.x;
	this->y = vec.y;
	this->z = vec.z;
	this->w = w;
}

Vector4::Vector4(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Vector4 Vector4::operator-()
{
	return Vector4(-x, -y, -z, -w);
}

Vector4 Vector4::operator+=(Vector4 x)
{
	this->x += x.x;
	this->y += x.y;
	this->z += x.z;
	this->w += x.w;
	return *this;
}

Vector4 Vector4::operator-=(Vector4 x)
{
	this->x -= x.x;
	this->y -= x.y;
	this->z -= x.z;
	this->w -= x.w;
	return *this;
}

Vector4 Vector4::operator+(Vector4 x)
{
	return Vector4(this->x + x.x, this->y + x.y, this->z + x.z, this->w + x.w);
}

Vector4 Vector4::operator/=(float x)
{
	this->x /= x;
	this->y /= x;
	this->z /= x;
	this->w /= x;
	return *this;
}

Vector4 Vector4::operator*=(float x)
{
	this->x *= x;
	this->y *= x;
	this->z *= x;
	this->w *= x;
	return *this;
}

Vector4 Vector4::operator*(float x) const
{
	Vector4 result(this->x * x, this->y * x, this->z * x, this->w * x);
	return result;
}

Vector4 Vector4::operator*(float x)
{
	return Vector4(this->x * x, this->y * x, this->z * x, this->w * x);
}

#endif


