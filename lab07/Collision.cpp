#include "Collision.h"
#include "Box.h"
#include "Sphere.h"
using namespace glm;

void handleBoxSphereCollision(Box& box, Sphere& sphere);
void handleSphereSphereCollision(Sphere& sphere, Sphere& sphere2);
bool checkForBoxSphereCollision(glm::vec3 &pos, const float& r,
                                const float& size, glm::vec3& n);



void handleBoxSphereCollision(Box& box, Sphere& sphere) {
    vec3 n;
    if (checkForBoxSphereCollision(sphere.x, sphere.r, box.size, n)) {
        // Task 2b: define the velocity of the sphere after the collision
        sphere.v = sphere.v - n * glm::dot(sphere.v, n) * 2.0f; // 1.96
        sphere.P = sphere.m * sphere.v;
    }
}

void handleSphereSphereCollision(Sphere& sphere, Sphere& sphere2)
{

    // Calculate the vector between the two spheres
    vec3 distanceVec = sphere2.x - sphere.x;
    float distance = length(distanceVec);
    float radiusSum = sphere.r + sphere2.r;

    // Check if the spheres are colliding
    if (distance < radiusSum) {
        // Normalize the collision normal
        vec3 n = normalize(distanceVec);

        // Separate the spheres to prevent overlap
        float overlap = radiusSum - distance;
        float totalMass = sphere.m + sphere2.m;
        sphere.x -= n * (overlap * (sphere2.m / totalMass));
        sphere2.x += n * (overlap * (sphere.m / totalMass));

        // Compute relative velocity
        vec3 relativeVelocity = sphere2.v - sphere.v;
        float velocityAlongNormal = dot(relativeVelocity, n);

        // If spheres are moving apart, skip
        if (velocityAlongNormal > 0) {
            float restitution = 1.0f; // Elastic collision
            float impulseScalar = -(1 + restitution) * velocityAlongNormal / (1 / sphere.m + 1 / sphere2.m);

            // Apply impulse
            vec3 impulse = impulseScalar * n;
            sphere.v -= impulse / sphere.m;
            sphere2.v += impulse / sphere2.m;

        }
          //  break;

        
    }
}

bool checkForBoxSphereCollision(vec3 &pos, const float& r,
                                const float& size, vec3& n) {
    if (pos.x - r <= 0) {
        //correction
        float dis = -(pos.x - r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(-1, 0, 0);
    } else if (pos.x + r >= size) {
        //correction
        float dis = size - (pos.x + r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(1, 0, 0);
    } else if (pos.y - r <= 0) {
        //correction
        float dis = -(pos.y - r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, -1, 0);
    } else if (pos.y + r >= size) {
        //correction
        float dis = size - (pos.y + r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, 1, 0);
    } else if (pos.z - r <= 0) {
        //correction
        float dis = -(pos.z - r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, -1);
    } else if (pos.z + r >= size) {
        //correction
        float dis = size - (pos.z + r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, 1);
    } else {
        return false;
    }

    return true;
}
