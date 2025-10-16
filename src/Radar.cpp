#include "Radar.h"
#include <vector>
#include <cmath>
#include <random>
#include <iostream>

using namespace std;

Radar::Radar(vector<float> pos, float max_range, float scan_interval, float beam_width, float noise_std)
    : pos(pos),
      max_range(max_range),
      scan_interval(scan_interval),
      scan_angle(0.0f),
      beam_width(beam_width),
      distance_noise_std(noise_std),
      azimuth_noise_std(0.5f),
      velocity_noise_std(0.5f),
      detection_prob(0.95f),
      generator(random_device{}()),
      norm_dist(0.0f, 0.5f),
      uniform_dist(0.0f, 0.5f)
{
}

void Radar::update(float dt)
{
    float deg = 360.0f * scan_interval * dt;
    if (deg + scan_angle >= 360)
        scan_angle -= 360;
    scan_angle += deg;
    //  detections.clear();
    for (auto it = detections.begin(); it != detections.end();)
    {
        it->lifespan -= dt;
        if (it->lifespan <= 0)
            it = detections.erase(it);
        else
            it++;
    }
}

void Radar::reset()
{
    scan_angle = 0.0f;
}

float Radar::calculateDistance(const Body &target) const
{
    float dx = target.get_pos()[0] - pos[0];
    float dy = target.get_pos()[1] - pos[1];
    return sqrt(dx * dx + dy * dy);
}

float Radar::calculateAzimuth(const Body &target) const
{
    float dx = target.get_pos()[0] - pos[0];
    float dy = target.get_pos()[1] - pos[1];
    float rad = atan2(dy, dx);
    float deg = rad * 180.0f / M_PI;
    if (deg < 0.0f && deg > -180.0f)
        deg += 360;
    return deg;
}

float Radar::calculateVelocity(const Body &target) const
{
    auto vel = target.get_vel();
    float dx = target.get_pos()[0] - pos[0];
    float dy = target.get_pos()[1] - pos[1];
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < 0.001f)
        return 0.0f;

    // Calculate radial velocity of the body
    float ux = dx / distance;
    float uy = dy / distance;

    return vel[0] * ux + vel[1] * uy;
}

bool Radar::shouldDetect(float distance, float azimuth)
{
    float degToCompareUp = scan_angle + beam_width / 2;
    float degToCompareDown = scan_angle - beam_width / 2;
    if (degToCompareUp >= 360)
        degToCompareUp -= 360;
    if (degToCompareDown < 0)
        ;
    degToCompareUp += 360;

    if (azimuth > scan_angle + beam_width / 2 ||
        azimuth < scan_angle - beam_width / 2)
        return false;
    // simulate probability of detection according to range, sigmoid based
    float prob = detection_prob * (2 / (1 + pow(M_E, distance * 0.0001)));
    return (distance < max_range) && (rand() % 100 < detection_prob * 100);
}

Detection Radar::scan(const Body &target, int target_id, float current_time)
{
    Detection det;
    det.timestamp = current_time;
    det.target_id = target_id;

    float distance = calculateDistance(target);
    float azimuth = calculateAzimuth(target);
    float radial_velocity = calculateVelocity(target);
    bool isDetected = shouldDetect(distance, azimuth);

    if (distance <= max_range && isDetected)
    {
        det.detected = true;
        det.distance = distance + norm_dist(generator) * distance_noise_std;
        det.azimuth = azimuth + norm_dist(generator) * azimuth_noise_std;
        det.radial_velocity = radial_velocity + norm_dist(generator) * velocity_noise_std;
        if (det.distance < 0)
            det.distance = 0;
        det.lifespan = 1.0f;
    }
    else
    {
        det.detected = false;
        det.distance = 0;
        det.azimuth = 0;
        det.radial_velocity = 0;
        det.lifespan = 0;
    }

    return det;
}

// Checks if detection has a close by existing detection
bool Radar::checkDetection(Detection detection, float azimuth_threshold, float distance_threshold)
{
    for (auto &d : detections)
    {
        float distance_delta = detection.distance - d.distance;
        float azimuth_delta = detection.azimuth - d.azimuth;

        if (abs(distance_delta) > distance_threshold ||
            abs(azimuth_delta) > azimuth_threshold)
            return true;
    }
    return false;
}

vector<Detection> Radar::scan(const vector<Body> &targets, float current_time)
{
    for (auto &d : detections)
    {
        d.detected = false;
    }

    for (size_t i = 0; i < targets.size(); i++)
    {
        Detection det = scan(targets[i], i, current_time);
        if (checkDetection(det))
            detections.push_back(det);
    }

    return detections;
}