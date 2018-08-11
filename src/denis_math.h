//----------------------------------------------------
//	  denis_math.h
// Written by Denis Levesque
// NOTE: this will only compile with a C++ compiler
//----------------------------------------------------

#if defined(UP_POSITIVE_Y)
#define LOWER_BY -
#define RAISE_BY +
#else
#define LOWER_BY +
#define RAISE_BY -
#endif

#ifndef DENIS_MATH_H_
#define DENIS_MATH_H_

#include "denis_types.h"

//TODO(denis): remove these eventually?
#define _USE_MATH_DEFINES
#include "math.h"

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#define ABS_VALUE(x) ((x) < 0 ? -(x) : (x))

#define CLAMP_MIN(value, min) MIN(value, min)
#define CLAMP_RANGE(value, min, max) ((value) > (min) ? MIN(value, max) : (min))
#define CLAMP_MAX(value, max) MAX(value, max)

//---------------------------------------------------------------------------
// Type Declarations:
union v2;
union v2f;
union v3;
union v3f;
union v4f;

struct Matrix4f;

struct Rect2;
struct Rect2f;

//---------------------------------------------------------------------------
// Functions Declarations:

static inline bool pointInRect(v2 point, Rect2 rect);

static inline bool pointInCircle(v2 point, v2 pos, s32 radius);
static inline bool pointInCircle(v2f point, v2f pos, s32 radius);

static inline f32 slope(v2 point1, v2 point2);
static inline f32 inverseSlope(v2 point1, v2 point2);

static inline f32 magnitude(v2 v);
static inline f32 magnitude(v3f v);

static inline v3f normalize(v3f vector);

static inline f32 dot(v3f v1, v3f v2);

static inline v3 cross(v3 v1, v3 v2);
static inline v3f cross(v3f v1, v3f v2);
static inline v4f cross(v4f v1, v4f v2);


//---------------------------------------------------------------------------
// Vector types:

union v2
{
	struct
	{
		s32 x;
		s32 y;
	};
	struct
	{
		s32 w;
		s32 h;
	};
	s32 e[2];
};

union v2f
{
	struct
	{
		f32 x;
		f32 y;
	};
	struct
	{
		f32 w;
		f32 h;
	};
	f32 e[2];
};

union v3
{
	struct
	{
		s32 x;
		s32 y;
		s32 z;
	};
	struct
	{
		s32 r;
		s32 g;
		s32 b;
	};
	struct
	{
		v2 xy;
		s32 filler;
	};
	s32 e[3];

	s32& operator[](u32 index)
	{
		return e[index];
	}
};

union v3f
{
	struct
	{
		f32 x;
		f32 y;
		f32 z;
	};
	struct
	{
		f32 r;
		f32 g;
		f32 b;
	};
	struct
	{
		v2f xy;
		f32 _unused;
	};
	f32 e[3];

	f32& operator[](u32 index)
	{
		return e[index];
	}

	v3f& operator+=(v3f right)
	{
		this->x = this->x + right.x;
		this->y = this->y + right.y;
		this->z = this->z + right.z;
		return *this;
	}
	v3f& operator-=(v3f right)
	{
		this->x = this->x - right.x;
		this->y = this->y - right.y;
		this->z = this->z - right.z;
		return *this;
	}
};

union v4f
{
	struct
	{
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
	struct
	{
		f32 r;
		f32 g;
		f32 b;
		f32 a;
	};
	struct
	{
		v3f xyz;
		f32 _unused;
	};
	f32 e[4];

	f32& operator[](u32 index)
	{
		return e[index];
	}
};

//---------------------------------------------------------------------------
// Matrix types:
// 4 x 4 transformation matrix in column-major style

struct Matrix4f
{
	f32 elements[4][4];

	v3f currentScale;

	f32* operator[](u32 index)
	{
		return elements[index];
	}

	void setRow(u32 row, v3f newValues);
	void setRow(u32 row, v4f newValues);

	void setTranslation(f32 x, f32 y, f32 z);
	void setTranslation(v3f translation);

	v3f getTranslation();

	void translate(f32 x, f32 y, f32 z);
	void translate(v3f translation);
	
	void setScale(f32 x, f32 y, f32 z);
	void setScale(v3f newScale);

	v3f getScale();
	
	void scale(f32 x, f32 y, f32 z);
	
	//NOTE(denis): these are in radians
	void setRotation(f32 xAngle, f32 yAngle, f32 zAngle);

	void rotate(f32 xAngle, f32 yAngle, f32 zAngle);
};

//---------------------------------------------------------------------------
// Rectangle types:

//TODO(denis): should my rectangles have a centre origin instead of top-left?
//TODO(denis): also, maybe put rectangles into a "denis_geometry.h" file once I get some more geometry code
//TODO(denis): put all types together at the top?
// Rectangle types
struct Rect2
{
	v2 min; // min is top-left
	v2 max; // max is bottom-right

	Rect2(s32 x, s32 y, s32 width, s32 height);
	Rect2(v2 min, v2 max);
	
	s32 getLeft() { return min.x; };
	s32 getRight() { return max.x; };
	s32 getTop() { return min.y; };
	s32 getBottom() { return max.y; };

	s32 getWidth() { return max.x - min.x; };
	s32 getHeight() { return ABS_VALUE(max.y - min.y); };

	void moveLeft(s32 amount) { setX(min.x - amount); }
	void moveRight(s32 amount) { setX(min.x + amount); }
	void moveUp(s32 amount) { setY(min.y RAISE_BY amount); }
	void moveDown(s32 amount) { setY(min.y LOWER_BY amount); }

	void setX(s32 newX);
	void setY(s32 newY);
	void setPos(v2 newPos);
};

struct Rect2f
{
	v2f min;
	v2f max;

	Rect2f(f32 x, f32 y, f32 width, f32 height);
	Rect2f(v2f min, v2f max);
	
	f32 getLeft() { return min.x; };
	f32 getRight() { return max.x; };
	f32 getTop() { return min.y; };
	f32 getBottom() { return max.y; };

	f32 getWidth() { return max.x - min.x; };
	f32 getHeight() { return ABS_VALUE(max.y - min.y); };

	void setX(f32 newX);
	void setY(f32 newY);
	void setPos(v2f newPos);
};

//---------------------------------------------------------------------------
// Vector Constructors

static inline v2 V2(s32 x, s32 y)
{
	v2 result = {x, y};
	return result;
}
static inline v2 V2(v2f v2f)
{
	v2 result = {(s32)v2f.x, (s32)v2f.y};
	return result;
}

static inline v2f V2f(f32 x, f32 y)
{
	v2f result = {x, y};
	return result;
}
static inline v2f V2f(v2 v2)
{
	v2f result = {(f32)v2.x, (f32)v2.y};
	return result;
}

static inline v3 V3(s32 x, s32 y, s32 z)
{
	v3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}
static inline v3 V3(v2 v, s32 z)
{
	v3 result = V3(v.x, v.y, z);
	return result;
}

static inline v3f V3f(f32 x, f32 y, f32 z)
{
	v3f result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}
static inline v3f V3f(v4f v)
{
	return V3f(v.x, v.y, v.z);
}

static inline v4f V4f(f32 x, f32 y, f32 z, f32 w)
{
	v4f result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}
static inline v4f V4f(f32 x, f32 y, f32 z)
{
	v4f result = V4f(x, y, z, 1.0f);
	return result;
}
static inline v4f V4f(v3f v, f32 w)
{
	v4f result = V4f(v.x, v.y, v.z, w);
	return result;
}


//---------------------------------------------------------------------------
// Vector operator overloads

v2 operator+(v2 left, v2 right)
{
	v2 result;
	result.x = left.x + right.x;
	result.y = left.y + right.y;
	return result;
}
v2 operator-(v2 left, v2 right)
{
	v2 result;
	result.x = left.x - right.x;
	result.y = left.y - right.y;
	return result;
}
v2 operator*(v2 left, s32 right)
{
	v2 result;
	result.x = left.x * right;
	result.y = left.y * right;
	return result;
}
v2 operator/(v2 left, s32 right)
{
	v2 result;
	result.x = left.x / right;
	result.y = left.y / right;
	return result;
}

v2f operator+(v2f left, v2f right)
{
	v2f result;
	result.x = left.x + right.x;
	result.y = left.y + right.y;
	return result;
}
v2f operator-(v2f left, v2f right)
{
	v2f result;
	result.x = left.x - right.x;
	result.y = left.y - right.y;
	return result;
}
v2f operator*(v2f left, f32 right)
{
	v2f result;
	result.x = left.x * right;
	result.y = left.y * right;
	return result;
}
v2f operator/(v2f left, f32 right)
{
	v2f result;
	result.x = left.x / right;
	result.y = left.y / right;
	return result;
}

static inline v3 operator+(v3 left, v3 right)
{
	v3 result;
	result.x = left.x + right.x;
	result.y = left.y + right.y;
	result.z = left.z + right.z;
	return result;
}
static inline v3 operator-(v3 left, v3 right)
{
	v3 result;
	result.x = left.x - right.x;
	result.y = left.y - right.y;
	result.z = left.z - right.z;
	return result;
}
static inline v3 operator*(v3 left, s32 right)
{
	v3 result;
	result.x = left.x * right;
	result.y = left.y * right;
	result.z = left.z * right;
	return result;
}
static inline v3 operator/(v3 left, s32 right)
{
	v3 result;
	result.x = left.x/right;
	result.y = left.y/right;
	result.z = left.z/right;
	return result;
}

static inline v3f operator-(v3f& v)
{
	v3f result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;		
	return result;
}
static inline v3f operator+(v3f left, v3f right)
{
	v3f result;
	result.x = left.x + right.x;
	result.y = left.y + right.y;
	result.z = left.z + right.z;	
	return result;
}
static inline v3f operator-(v3f left, v3f right)
{
	v3f result;
	result.x = left.x - right.x;
	result.y = left.y - right.y;
	result.z = left.z - right.z;
	return result;
}
static inline v3f operator*(s32 left, v3f right)
{
	v3f result;
	result.x = right.x * left;
	result.y = right.y * left;
	result.z = right.z * left;
	return result;
}
static inline v3f operator*(v3f left, s32 right)
{
	return right*left;
}
static inline v3f operator*(v3f left, f32 right)
{
	v3f result;
	result.x = left.x * right;
	result.y = left.y * right;
	result.z = left.z * right;
	return result;
}
static inline v3f operator*(f32 left, v3f right)
{
	return right*left;
}
static inline v3f operator/(v3f left, f32 right)
{
	v3f result;
	result.x = left.x / right;
	result.y = left.y / right;
	result.z = left.z / right;
	return result;
}
static inline bool operator!=(v3f left, v3f right)
{
	return left.x != right.x || left.y != right.y || left.z != right.z;
}

static inline v4f operator+(v4f left, v4f right)
{
	v4f result;
	result.x = left.x + right.x;
	result.y = left.y + right.y;
	result.z = left.z + right.z;
	result.w = left.w + right.w;
	return result;
}
static inline v4f operator-(v4f left, v4f right)
{
	v4f result;
	result.x = left.x - right.x;
	result.y = left.y - right.y;
	result.z = left.z - right.z;
	result.w = left.w - right.w;	
	return result;
}
static inline v4f operator/(v4f left, f32 scalar)
{
	v4f result;
	result.x = left.x / scalar;
	result.y = left.y / scalar;
	result.z = left.z / scalar;
	result.w = left.w / scalar;
	return result;
}

//--------------------------------------------------------------------------
// Matrix Helper Functions

static inline Matrix4f getIdentityMatrix4f()
{
	Matrix4f result = {};
	result.currentScale = V3f(1.0f, 1.0f, 1.0f);

	result[0][0] = 1.0f;
	result[1][1] = 1.0f;
	result[2][2] = 1.0f;
	result[3][3] = 1.0f;
	
	return result;
}

static inline Matrix4f M4f()
{
	Matrix4f result = getIdentityMatrix4f();
	return result;
}

static inline Matrix4f getXRotationMatrix(f32 xAngle)
{
	Matrix4f xRotation = getIdentityMatrix4f();
	xRotation[1][1] = (f32)cos(xAngle);
	xRotation[1][2] = (f32)-sin(xAngle);
	xRotation[2][1] = (f32)sin(xAngle);
	xRotation[2][2] = (f32)cos(xAngle);

	return xRotation;
}
static inline Matrix4f getYRotationMatrix(f32 yAngle)
{
	Matrix4f yRotation = getIdentityMatrix4f();
	yRotation[0][0] = (f32)cos(yAngle);
	yRotation[0][2] = (f32)sin(yAngle);
	yRotation[2][0] = (f32)-sin(yAngle);
	yRotation[2][2] = (f32)cos(yAngle);

	return yRotation;
}
static inline Matrix4f getZRotationMatrix(f32 zAngle)
{
	Matrix4f zRotation = getIdentityMatrix4f();
	zRotation[0][0] = (f32)cos(zAngle);
	zRotation[0][1] = (f32)-sin(zAngle);
	zRotation[1][0] = (f32)sin(zAngle);
	zRotation[1][1] = (f32)cos(zAngle);

	return zRotation;
}

//--------------------------------------------------------------------------
// Matrix Operator Overloads

//TODO(denis): not efficient at all!
static inline Matrix4f operator*(Matrix4f left, Matrix4f right)
{
	Matrix4f result = M4f();

	for (u32 row = 0; row < 4; ++row)
	{
		for (u32 col = 0; col < 4; ++col)
		{
			f32 sum = 0;
			for (u32 element = 0; element < 4; ++element)
			{
				sum += left[row][element] * right[element][col];
			}
			result[row][col] = sum;
		}
	}
	
	return result;
}

static inline v4f operator*(Matrix4f left, v4f right)
{
	v4f result;

	result.x = left[0][0]*right.x + left[0][1]*right.y + left[0][2]*right.z + left[0][3]*right.w;
	result.y = left[1][0]*right.x + left[1][1]*right.y + left[1][2]*right.z + left[1][3]*right.w;
	result.z = left[2][0]*right.x + left[2][1]*right.y + left[2][2]*right.z + left[2][3]*right.w;
	result.w = left[3][0]*right.x + left[3][1]*right.y + left[3][2]*right.z + left[3][3]*right.w;
	
	return result;
}
static inline v3f operator*(Matrix4f left, v3f right)
{
	v4f result = left * V4f(right, 1.0f);
	return result.xyz;
}

//--------------------------------------------------------------------------
// Matrix Member Functions

void Matrix4f::setRow(u32 row, v3f newValues)
{
	if (row > 3)
		return;

	elements[row][0] = newValues.x;
	elements[row][1] = newValues.y;
	elements[row][2] = newValues.z;
}
void Matrix4f::setRow(u32 row, v4f newValues)
{
	if (row > 3)
		return;

	elements[row][0] = newValues.x;
	elements[row][1] = newValues.y;
	elements[row][2] = newValues.z;
	elements[row][3] = newValues.w;
}

void Matrix4f::setTranslation(f32 x, f32 y, f32 z)
{
	elements[0][3] = x;
	elements[1][3] = y;
	elements[2][3] = z;
}
void Matrix4f::setTranslation(v3f translation)
{
	setTranslation(translation.x, translation.y, translation.z);
}

v3f Matrix4f::getTranslation()
{
	v3f result = V3f(elements[0][3], elements[1][3], elements[2][3]);
	return result;
}

void Matrix4f::translate(f32 x, f32 y, f32 z)
{
	elements[0][3] += x;
	elements[1][3] += y;
	elements[2][3] += z;
}
void Matrix4f::translate(v3f translation)
{
	translate(translation.x, translation.y, translation.z);
}

void Matrix4f::setScale(f32 x, f32 y, f32 z)
{
	//NOTE(denis): first we need to remove the current scale
	elements[0][0] /= currentScale.x;
	elements[1][1] /= currentScale.y;
	elements[2][2] /= currentScale.z;

	elements[0][0] *= x;
	elements[1][1] *= y;
	elements[2][2] *= z;

	currentScale = V3f(x, y, z);
}
void Matrix4f::setScale(v3f newScale)
{
	setScale(newScale.x, newScale.y, newScale.z);
}

v3f Matrix4f::getScale()
{
	return currentScale;
}

void Matrix4f::scale(f32 x, f32 y, f32 z)
{
	setScale(currentScale.x * x, currentScale.y * y, currentScale.z * z);
}

//NOTE(denis): these are in radians
void Matrix4f::setRotation(f32 xAngle, f32 yAngle, f32 zAngle)
{
	v3f savedTranslation = getTranslation();
	v3f savedScale = currentScale;
	currentScale = V3f(1.0f, 1.0f, 1.0f);

	Matrix4f xRotation = getXRotationMatrix(xAngle);
	Matrix4f yRotation = getYRotationMatrix(yAngle);
	Matrix4f zRotation = getZRotationMatrix(zAngle);
	Matrix4f transformation = zRotation * yRotation * xRotation;

	*this = transformation;
	setTranslation(savedTranslation);
	setScale(savedScale);
}

//TODO(denis): probably super inefficient
void Matrix4f::rotate(f32 xAngle, f32 yAngle, f32 zAngle)
{
	v3f savedTranslation = getTranslation();
	v3f savedScale = currentScale;
	setScale(1.0f, 1.0f, 1.0f);
		
	Matrix4f xRotation = getXRotationMatrix(xAngle);
	Matrix4f yRotation = getYRotationMatrix(yAngle);
	Matrix4f zRotation = getZRotationMatrix(zAngle);
	Matrix4f transformation = zRotation * yRotation * xRotation;
	
	*this = operator*(transformation, *this);
		
	setTranslation(savedTranslation);
	setScale(savedScale);
}

//--------------------------------------------------------------------------
// Rectangle Member Functions

Rect2::Rect2(s32 x, s32 y, s32 width, s32 height)
{
	min = V2(x, y);
	max = V2(x + width, y LOWER_BY height);
}
Rect2::Rect2(v2 min, v2 max)
{
	this->min = min;
	this->max = max;
}

void Rect2::setX(s32 newX)
{
	s32 width = getWidth();
	min.x = newX;
	max.x = newX + width;
}
void Rect2::setY(s32 newY)
{
	s32 height = getHeight();
	min.y = newY;
	max.y = newY LOWER_BY height;
}
void Rect2::setPos(v2 newPos)
{
	setX(newPos.x);
	setY(newPos.y);
}


Rect2f::Rect2f(f32 x, f32 y, f32 width, f32 height)
{
	min = V2f(x, y);
	max = V2f(x + width, y LOWER_BY height);
}
Rect2f::Rect2f(v2f min, v2f max)
{
	this->min = min;
	this->max = max;
}

void Rect2f::setX(f32 newX)
{
	f32 width = getWidth();
	min.x = newX;
	max.x = newX + width;
}
void Rect2f::setY(f32 newY)
{
	f32 height = getHeight();
	min.y = newY;
	max.y = newY LOWER_BY height;
}
void Rect2f::setPos(v2f newPos)
{
	setX(newPos.x);
	setY(newPos.y);
}

//--------------------------------------------------------------------------
// General Function Definitions

#if defined(UP_POSITIVE_Y)
static inline bool pointInRect(v2 point, Rect2 rect)
{
	return point.x > rect.getLeft() && point.x < rect.getRight() &&
		point.y < rect.getTop() && point.y > rect.getBottom();
}
#else
static inline bool pointInRect(v2 point, Rect2 rect)
{
	return point.x > rect.getLeft() && point.x < rect.getRight() &&
		point.y > rect.getTop() && point.y < rect.getBottom();
}
#endif

//TODO(denis): for now this only checks the smallest rect that contains the given circle
static inline bool pointInCircle(v2 point, v2 pos, s32 radius)
{
	return point.x > pos.x - radius && point.x < pos.x + radius &&
		point.y > pos.y - radius && point.y < pos.y + radius;
}
static inline bool pointInCircle(v2f point, v2f pos, s32 radius)
{
	return point.x > pos.x - radius && point.x < pos.x + radius &&
		point.y > pos.y - radius && point.y < pos.y + radius;	
}

f32 slope(v2 point1, v2 point2)
{
	return (f32)(point2.y - point1.y) / (f32)(point2.x - point1.x);
}
f32 inverseSlope(v2 point1, v2 point2)
{
	return (f32)(point2.x - point1.x) / (f32)(point2.y - point1.y);
}

static inline f32 magnitude(v2 v)
{
	return (f32)sqrt((f32)(v.x*v.x + v.y*v.y));
}
static inline f32 magnitude(v3f v)
{
	return (f32)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static inline v3f normalize(v3f vector)
{
	v3f result = {};
	
	f32 vectorMagnitude = magnitude(vector);
	if (vectorMagnitude > 0)
		result = vector/vectorMagnitude;
	
	return result;
}

static inline v3 cross(v3 v1, v3 v2)
{
	v3 result;

	result.x = v1.y*v2.z - v1.z*v2.y;
	result.y = v1.z*v2.x - v1.x*v2.z;
	result.z = v1.x*v2.y - v1.y*v2.x;

	return result;
}
static inline v3f cross(v3f v1, v3f v2)
{
	v3f result;
	
	result.x = v1.y*v2.z - v1.z*v2.y;
	result.y = v1.z*v2.x - v1.x*v2.z;
	result.z = v1.x*v2.y - v1.y*v2.x;

	return result;
}
static inline v4f cross(v4f v1, v4f v2)
{
	v3f reducedVector1 = V3f(v1.x, v1.y, v1.z);
	v3f reducedVector2 = V3f(v2.x, v2.y, v2.z);

	v4f result = V4f(cross(reducedVector1, reducedVector2), 1.0f);
	return result;
}

static inline f32 dot(v3f v1, v3f v2)
{
	f32 result = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
	return result;
}

static inline v3f hadamard(v3f v1, v3f v2)
{
	v3f result = V3f(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z);
	return result;
}

#endif
