#ifndef RADAR_H
#define RADAR_H

#include "Body.h"

#include <array>
#include <vector>
#include <random>

using namespace std;

struct Detection
{
    bool detected = false;
    float distance,
        azimuth,
        radial_velocity,
        timestamp;
    int target_id;
};

class Radar
{
private:
    vector<float> pos;
    float max_range;
    float scan_interval;
    float scan_angle;
    float beam_width;

    float distance_noise_std;
    float azimuth_noise_std;
    float velocity_noise_std;
    float detection_prob;

    // Randomness helpers
    default_random_engine generator;
    normal_distribution<float> norm_dist;
    uniform_real_distribution<float> uniform_dist;

    // Decides if the target is detected, considering detection probability and distance
    bool shouldDetect(float distance, float azimuth);

public:
    Radar(vector<float> pos, float max_range, float scan_interval = 0.25f, float beam_width = 10.0f, float noise_std = 2.0f);

    void update(float dt);
    void reset();

    Detection scan(const Body &target, int target_id, float current_time);
    vector<Detection> scan(const vector<Body> &targets, float current_time);

    // Calculation functions
    float calculateDistance(const Body &target) const;
    float calculateAzimuth(const Body &target) const;
    float calculateVelocity(const Body &target) const;

    vector<float> get_pos() const { return pos; }
    float get_max_range() const { return max_range; }
    float getScanInterval() const { return scan_interval; }
    float getScanAngle() const { return scan_angle; }
    float getBeamWidth() const { return beam_width; }
};

#endif