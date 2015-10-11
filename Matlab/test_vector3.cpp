#include <iostream>
#include "vector3.h"

using namespace std;

int main()
{
	vector3 x(0.5f, 1.5f, 2.5f);
	vector3 y = x;
	y.normalize();
	x.sub(y);
	printf("x = (%f, %f, %f)\n", x.x, x.y, x.z);	
	printf("y = (%f, %f, %f)\n", y.x, y.y, y.z);	
	y.add(x);
	printf("y = (%f, %f, %f)\n", y.x, y.y, y.z);	
	vector3 z(1.0f, 0.0f, 0.0f);
	z.cross(x);
	printf("z = (%f, %f, %f)\n", z.x, z.y, z.z);	
}
