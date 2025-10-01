#include "Body.h"
#include "Radar.h"

#include <iostream>

int main()
{
    int dt = 0.1;

    Body body({0,0}, {1,1});

    // Simulation loop
    for(int t = 0; t < 60; t += dt)
    {
        body.update(dt);
        std::cout << "Position: " << body.get_pos()[0] << ", " << body.get_pos()[1] << "\n";
    }

    return 0;
}