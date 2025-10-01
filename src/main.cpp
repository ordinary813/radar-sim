#include "Body.h"
#include "Radar.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>

using namespace std;

int main()
{
    float dt = 0.01;

    cout.setf(ios::fixed,ios::floatfield);
    cout.precision(3);

    Body body({0,0}, {1,1}, {10,10});

    #ifdef _WIN32
        _mkdir("../data");
    #else
        mkdir("../data", 0777);
    #endif

    ofstream logfile("../data/trajectory.csv");
    if (!logfile.is_open()) {
        cerr << "Error: Could not create trajectory.csv\n";
        return 1;
    }

    logfile << "time,x,y\n";

    // Simulation loop
    for(float t = 0; t < 60; t += dt)
    {
        auto pos = body.get_pos();
        logfile << t << "," << pos[0] << "," << pos[1] << "\n";
        body.update(dt);
    }

    logfile.close();
    cout << "✓ Data saved to data/trajectory.csv\n";
    cout << "\nRun visualization:\n";
    cout << "  python scripts/visualize.py\n";

    return 0;
}