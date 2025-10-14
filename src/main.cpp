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
    float scan_interval = 1.0f;
    float next_scan_time = 0.0f;
    
    Renderer renderer(SCREEN_SIZE, SCREEN_SIZE, WORLD_SIZE, dt);
    
    // Targets
    vector<Body> targets;
    targets.push_back(Body({0, -25}));

    vector<Detection> detected(targets.size());

    Radar radar({10, 25}, 100.0f, 0.5f, 20.0f);
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
                    next_scan_time = 0.0f;
                    targets.clear();
                    targets.push_back(Body({0, 0}, {5, 5}));
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

            float updated_sim_time = renderer.advanceSimTime();

            cout << "\n=== Scan at t=" << fixed << setprecision(1) << updated_sim_time << "s ===\n";

            auto detections = radar.scan(targets, updated_sim_time);
            cout << "Detected " << detections.size() << " targets.\n";

            // Update detection status
            for(auto &d: detected)
            {
                d.detected = false;
            }

            for (const auto &det : detections)
            {
                auto tgt_pos = targets[det.target_id].get_pos();
                float dx = tgt_pos[0] - radar_pos[0];
                float dy = tgt_pos[1] - radar_pos[1];
                float true_distance = sqrt(dx * dx + dy * dy);

                cout << "  Target " << det.target_id
                        << ": Distance=" << det.distance << "m"
                        << ", Bearing=" << det.azimuth << "°"
                        << ", Velocity=" << det.radial_velocity << "m/s\n";

                detected[det.target_id].detected = true;
            }

            next_scan_time += dt;
            
        }

        renderer.render(radar, targets, detected);
    }
    return 0;
}