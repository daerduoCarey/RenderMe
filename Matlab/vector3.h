#include <cstdio>
#include <cstdlib>
#include <cmath>

class vector3 {

public:
	vector3(float xx, float yy, float zz) {
		x = xx;
		y = yy;
		z = zz;
	}

	vector3(const vector3& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}

	vector3(const double* v) {
		x = float(*v);
		y = float(*(v+1));
		z = float(*(v+2));
	}

	void add(const vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}

	void sub(const vector3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}

	void mul(const vector3& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
	}

	void cross(const vector3& v) {
		float new_x = y * v.z - z * v.y;
		float new_y = z * v.x - x * v.z;
		float new_z = x * v.y - y * v.x;
		x = new_x;
		y = new_y;
		z = new_z;
	}

	void normalize() {
		float len = sqrt(x * x + y * y + z * z);
		x /= len;
		y /= len;
		z /= len;
	}

	float x, y, z;
};
