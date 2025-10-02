#include "Body.h"
#include "Radar.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <iomanip>

using namespace std;

int main()
{
    float dt = 0.01;
    float scan_interval = 1.0f;
    float next_scan_time = 0.0f;

    vector<Body> targets;
    targets.push_back(Body({0, 0}, {5, 5}));
    targets.push_back(Body({40, 40}, {-2, -3}));
    targets.push_back(Body({-20, 45}, {3, -3}));

    Radar radar({0, 0}, 50.0f, 2.0f);

    std::ofstream traj_file("../data/trajectory.csv");
    std::ofstream detect_file("../data/detections.csv");

    traj_file << "time,target_id,x,y,vx,vy\n";
    detect_file << "time,target_id,detected,range,bearing,radial_velocity,true_range\n";

    for (int t = 0; t < 60; t += dt)
    {
        // update all bodies
        for (auto &target : targets)
        {
            target.update(dt);
        }

        // log actual pos of targets
        for (size_t i = 0; i < targets.size(); i++)
        {
            auto pos = targets[i].get_pos();
            auto vel = targets[i].get_vel();
            traj_file << t << "," << i << ","
                      << pos[0] << "," << pos[1] << ","
                      << vel[0] << "," << vel[1] << "\n";
        }

        // radar scan
        if (t >= next_scan_time)
        {
            cout << "\n=== Scan at t=" << fixed << setprecision(1)
                 << t << "s ===\n";

            auto detections = radar.scan(targets, t);

            cout << "Detected " << detections.size() << " targets." << endl;

            // log hits
            for (const auto &det : detections)
            {
                float dx = targets[det.target_id].get_pos()[0] - radar.get_pos()[0];
                float dy = targets[det.target_id].get_pos()[1] - radar.get_pos()[1];
                float true_distance = sqrt(dx * dx + dy * dy);

                detect_file << det.timestamp << "," << det.target_id << ",1,"
                           << det.distance << "," << det.azimuth << ","
                           << det.radial_velocity << "," << true_distance << "\n";
                
                cout << "  Target " << det.target_id 
                         << ": Range=" << det.distance << "m"
                         << ", Bearing=" << det.azimuth << "°"
                         << ", Velocity=" << det.radial_velocity << "m/s\n";
            }

            //log unknowns
            for(const auto& det: detections)
            {
                if(!det.detected)
                {
                    detect_file << t << "," << det.target_id << ",0,0,0,0,0\n";
                }
            }

            next_scan_time += scan_interval;
        }
    }

    traj_file.close();
    detect_file.close();

    cout << "\n Trajectories --> data/trajectory.csv\n";
    cout << " Detections --> data/detection.csv\n";

    return 0;
}