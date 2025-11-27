#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>

class Box;
class Sphere;
void handleBoxSphereCollision(Box& box, Sphere& sphere);

void handleSphereSphereCollision(Sphere& sphere, Sphere& sphere2);


#endif
