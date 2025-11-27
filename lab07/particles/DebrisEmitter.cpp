#include "DebrisEmitter.h"
#include <iostream>

DebrisEmitter::DebrisEmitter(Drawable* _model, int number)
    : IntParticleEmitter(_model, number) {
    for (int i = 0; i < number_of_particles; i++) {
        createNewParticle(i);
    }
}

void DebrisEmitter::updateParticles(float time, float dt, glm::vec3 camera_pos) {
    for (int i = 0; i < number_of_particles; i++) {
        auto& p = p_attributes[i];

        p.velocity += glm::vec3(0.0f, -9.8f * dt, 0.0f); // Gravity
        p.position += p.velocity * dt;

        // If the particle hits the ground, make it bounce
        if (p.position.y < 0.0f) {
            p.velocity.y *= -0.5f;  // Reduce speed on bounce
            p.position.y = 0.01f;   // Prevent it from getting stuck below ground
        }

        // Reduce life over time
        p.life -= dt * 0.2f;
        if (p.life <= 0.0f) {
            createNewParticle(i);
        }
    }
}

void DebrisEmitter::createNewParticle(int index) {
    particleAttributes& particle = p_attributes[index];

    particle.position = emitter_pos + glm::vec3(1 - RAND * 2, RAND * 2, 1 - RAND * 2);
    particle.velocity = glm::vec3((RAND - 0.5f) * 5.0f, RAND * 10.0f, (RAND - 0.5f) * 5.0f);

    particle.mass = RAND + 1.0f; // Heavier particles
    particle.life = 1.0f;        // Alive
}