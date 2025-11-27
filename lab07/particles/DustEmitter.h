#ifndef DUST_EMITTER_H
#define DUST_EMITTER_H

#include "IntParticleEmitter.h"

class DustEmitter : public IntParticleEmitter {
public:
    DustEmitter(Drawable* _model, int number, glm::vec3 center, float radius);
    void updateParticles(float time, float dt, glm::vec3 camera_pos) override;
    void createNewParticle(int index) override;

private:
    glm::vec3 center_position; // Center of the dust cloud
    float radius; // Radius of distribution

};

#endif // DUST_EMITTER_H
