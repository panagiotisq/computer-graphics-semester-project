#include "DustEmitter.h"
#include <iostream>

DustEmitter::DustEmitter(Drawable* _model, int number, glm::vec3 center, float radius)
    : IntParticleEmitter(_model, number), center_position(center), radius(radius) {
    for (int i = 0; i < number_of_particles; i++) {
        createNewParticle(i);
    }
}
void DustEmitter::updateParticles(float time, float dt, glm::vec3 camera_pos) {
    
    glm::vec3 wind_direction = glm::vec3(-1.0f, 0.0f, 0.05f); // Adjust wind force

    for (int i = 0; i < number_of_particles; i++) {
        auto& p = p_attributes[i];

        // Apply wind effect
        p.velocity += wind_direction * dt * 2.5f;

        // Simulate slow floating with randomness
        p.velocity += glm::vec3((RAND - 0.5f) * 0.02f, (RAND - 0.5f) * 0.01f, (RAND - 0.5f) * 0.02f);
        p.position += p.velocity * dt;

        // Reset particles that go too high or too far
        if (glm::length(p.position - center_position) > radius){
            createNewParticle(i);
        }
        
    }
}

void DustEmitter::createNewParticle(int index) {
    particleAttributes& particle = p_attributes[index];

    // Spread dust across a wide area    

    particle.position = center_position + glm::vec3(
        2*(RAND - 0.5f) * radius,  // Random X position
        RAND * (100.0f - center_position.y),     // Random height in the range [center_position.y-100]
        2*(RAND - 0.5f) * radius    // Random Z position
    );

    // Slow floating movement
    particle.velocity = glm::vec3(
        (RAND - 0.5f) * 0.1f,  // Small horizontal drift
        (RAND * 0.05f),        // Slow upward drift
        (RAND - 0.5f) * 0.1f   // Small horizontal drift
    );

    particle.mass = 0.2f;  // Small particles
    particle.life = 1.0f;   // Always alive
}