#include "Body.h"
#include <cmath>

Body::Body(std::vector<float> pos, std::vector<float> vel, std::vector<float> accel) {
    this->pos = pos;
    this->vel = vel;
    this->accel = accel;
}

Body::Body(std::vector<float> pos, std::vector<float> vel) {
    this->pos = pos;
    this->vel = vel;
    this->accel = {0, 0};
}

void Body::update(float dt)
{
    pos[0] += vel[0] * dt + accel[0] / 2.0f * std::pow(dt, 2);
    pos[1] += vel[1] * dt + accel[1] / 2.0f * std::pow(dt, 2);
}