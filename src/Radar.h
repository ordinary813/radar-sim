#ifndef RADAR_H
#define RADAR_H

#include <vector>

#include "Body.h"
#include <array>
#include <vector>
#include <random>

struct Detection
{
    bool detected;
    float   distance,
            azimuth,
            radial_velocity,
            timestamp;
    int target_id;
};

class Radar
{
private:
    std::vector<float> pos;
    float max_range;
    float distance_noise_std;
    float azimuth_noise_std;
    float velocity_noise_std;
    float detection_prob;

    // Randomness helpers
    std::default_random_engine generator;
    std::normal_distribution<float> norm_dist;
    std::uniform_int_distribution<float> uniform_dist;

    // Helper methods
    float calculateDistance(const Body& target) const;
    float calculateAzimuth(const Body& target) const;
    float calculateVelocity(const Body& target) const;

    // Decides if the target is detected, considering detection probability and distance
    bool shouldDetect(float distance);

public:
    Radar(std::vector<float> pos, float max_range, float noise_std = 2.0f);

    Detection scan(const Body& target, int target_id, float current_time);
    std::vector<Detection> scan(const std::vector<Body>& targets, float current_time);


    std::vector<float> get_pos() const { return pos; }
    float get_max_range() const { return max_range; }
};

#endif