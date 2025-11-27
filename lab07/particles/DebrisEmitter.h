#ifndef DEBRIS_EMITTER_H
#define DEBRIS_EMITTER_H

#include "IntParticleEmitter.h"

class DebrisEmitter : public IntParticleEmitter {
public:
    DebrisEmitter(Drawable* _model, int number);
    void updateParticles(float time, float dt, glm::vec3 camera_pos) override;
    void createNewParticle(int index) override;
};

#endif // DEBRIS_EMITTER_H
