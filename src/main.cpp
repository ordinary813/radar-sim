#include "Body.h"
#include "Radar.h"
#include "Renderer.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <cmath>

using namespace std;

int main()
{
    // Window setup
    const float SCREEN_SIZE = 800.0f;
    const float WORLD_SIZE = 400.0f;
    const float GRID_SPACING = 10.0f;
    
    float dt = 0.016f;
    
    Renderer renderer(SCREEN_SIZE, SCREEN_SIZE, WORLD_SIZE, dt);
    
    // Targets
    vector<Body> targets;
    targets.push_back(Body({0, -25}));

    vector<Detection> detected(targets.size());

    const vector<float> RADAR_POS = {0, 0};
    const float MAX_RANGE = 100.0f;
    const float SCAN_INTERVAL = 0.5f;
    const float BEAM_WIDTH = 50.0f;

    Radar radar(RADAR_POS, MAX_RANGE, SCAN_INTERVAL, BEAM_WIDTH);
    auto radar_pos = radar.get_pos();
    sf::Vector2f radarScreenPos = renderer.worldToScreen(radar_pos[0], radar_pos[1]);

    cout << "Simulation started. Press SPACE to pause, R to reset, ESC to quit.\n";

    while (renderer.isRunning())
    {
        // Events
        sf::Event event;
        while (renderer.processEvents(event))
        {
            if (event.type == sf::Event::Closed)
                renderer.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Space)
                    renderer.flipPause();
                if (event.key.code == sf::Keyboard::Escape)
                    renderer.close();
                if (event.key.code == sf::Keyboard::R)
                {
                    renderer.reset();
                    radar.reset();
                    targets.clear();
                    targets.push_back(Body({0, 0}, {5, 5}));
                    targets.push_back(Body({10, 0}));
                    targets.push_back(Body({-25, -10}, {5, 5}, {1,1}));
                }
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    renderer.setMouseDragging(true, new sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                }
            }
            if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    renderer.setMouseDragging(false);
                }
            }
            if (event.type == sf::Event::MouseMoved)
            {
                if (renderer.isDragging)
                {
                    renderer.updateView(
                        ViewAction::DRAG,
                        new sf::Vector2i(event.mouseMove.x, event.mouseMove.y),
                        &event
                    );
                }
            }

            if (event.type == sf::Event::Resized)
            {
                renderer.updateView(
                        ViewAction::RESIZE,
                        new sf::Vector2i(event.mouseMove.x, event.mouseMove.y),
                        &event
                    );
            }
        }

        // Update simulation
        if (!renderer.isPaused)
        {
            // Update all targets
            for (auto &target : targets)
            {
                target.update(dt);
            }
            radar.update(dt);

            float current_sim_time = renderer.advanceSimTime();

            cout << "\n=== Scan at t=" << fixed << setprecision(1) << current_sim_time << "s ===\n";

            // NEED TO FIX SCAN, the radar shouldnt get all targets and decide which ones fits in the cone,
            // the simulation gets a scan request from the radar and return only the targets within the cone, then
            // return a detection/body list to the radar
            auto curr_detections = radar.scan(targets, current_sim_time);
            cout << "Detected " << curr_detections.size() << " targets.\n";

            // Reset detection status
            for(auto &d: detected)
            {
                d.detected = false;
            }

            // Log detections
            for (const auto &det : curr_detections)
            {
                cout << "  Target " << det.target_id
                        << ": Distance=" << det.distance << "m"
                        << ", Bearing=" << det.azimuth << "°"
                        << ", Velocity=" << det.radial_velocity << "m/s\n";

                detected[det.target_id].detected = true;
            }
        }
        vector<Detection> curr_detections = radar.getDetections();
        renderer.render(radar, targets, curr_detections);
    }
    return 0;
}