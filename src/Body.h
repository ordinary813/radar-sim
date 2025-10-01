#ifndef BODY_H
#define BODY_H
#include <vector>

class Body
{
public:
    Body(std::vector<float> pos, std::vector<float> vel, std::vector<float> accel);
    Body(std::vector<float> pos, std::vector<float> vel);

    void update(float dt);

    std::vector<float> get_pos() const { return pos; }
    std::vector<float> get_vel() const { return vel; }
    std::vector<float> get_accel() const { return accel; }

private:
    std::vector<float> pos;
    std::vector<float> vel;
    std::vector<float> accel;
};

#endif