#ifndef BODY_H
#define BODY_H
#include <vector>
#include <ostream>

class Body
{
public:
    Body(std::vector<float> pos, std::vector<float> vel, std::vector<float> accel);
    Body(std::vector<float> pos, std::vector<float> vel);
    Body(std::vector<float> pos);

    void update(float dt);
    void update(float dt, std::vector<float> accel);

    std::vector<float> get_pos() const { return pos; }
    std::vector<float> get_vel() const { return vel; }
    std::vector<float> get_accel() const { return accel; }

    friend std::ostream& operator<<(std::ostream& os, const Body& body);

private:
    std::vector<float> pos;
    std::vector<float> vel;
    std::vector<float> accel;
};

#endif