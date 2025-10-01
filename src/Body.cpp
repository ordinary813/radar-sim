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
    vel[0] += accel[0] * dt;
    vel[1] += accel[1] * dt;

    pos[0] += vel[0] * dt;
    pos[1] += vel[1] * dt;
}

void Body::update(float dt, std::vector<float> accel)
{
    this->accel = accel;
    Body::update(dt);
}