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

sf::Vector2f worldToScreen(float x, float y, float screenSize, float worldSize)
{
    float scale = screenSize / worldSize;
    return sf::Vector2f(screenSize / 2 + x * scale, screenSize / 2 - y * scale);
}

void drawGrid(sf::RenderWindow &window, float screenSize, float worldSize, float spacing)
{
    for (float i = -worldSize * 1000; i <= worldSize * 1000; i += spacing)
    {
        sf::Color color = (i == 0) ? sf::Color::White : sf::Color(50, 50, 50);
        sf::Vertex vLine[] = {
            sf::Vertex(worldToScreen(i, -worldSize * 1000, screenSize, worldSize), color),
            sf::Vertex(worldToScreen(i, worldSize * 1000, screenSize, worldSize), color)};
        sf::Vertex hLine[] = {
            sf::Vertex(worldToScreen(-worldSize * 1000, i, screenSize, worldSize), color),
            sf::Vertex(worldToScreen(worldSize * 1000, i, screenSize, worldSize), color)};
        window.draw(vLine, 2, sf::Lines);
        window.draw(hLine, 2, sf::Lines);
    }
}

void drawRadar(sf::RenderWindow &window, const Radar &radar, float screenSize, float worldSize)
{
    auto radar_pos = radar.get_pos();
    sf::Vector2f pos = worldToScreen(radar_pos[0], radar_pos[1], screenSize, worldSize);
    float rangeRadius = radar.get_max_range() * (screenSize / worldSize);

    // Range circle
    sf::CircleShape rangeCircle(rangeRadius);
    rangeCircle.setOrigin(rangeRadius, rangeRadius);
    rangeCircle.setPosition(pos);
    rangeCircle.setFillColor(sf::Color::Transparent);
    rangeCircle.setOutlineColor(sf::Color(0, 255, 255, 80));
    rangeCircle.setOutlineThickness(2);
    window.draw(rangeCircle);

    // Radar dot
    sf::CircleShape dot(4);
    dot.setOrigin(4, 4);
    dot.setPosition(pos);
    dot.setFillColor(sf::Color::Cyan);
    window.draw(dot);
}

void drawTarget(sf::RenderWindow &window, const Body &target, bool detected,
                const sf::Vector2f &radarPos, float screenSize, float worldSize,
                sf::Color color, const sf::Font &font)
{
    auto pos = target.get_pos();
    sf::Vector2f screenPos = worldToScreen(pos[0], pos[1], screenSize, worldSize);

    // Target dot
    sf::CircleShape dot(8);
    dot.setOrigin(8, 8);
    dot.setPosition(screenPos);
    dot.setFillColor(color);
    dot.setOutlineColor(sf::Color::White);
    dot.setOutlineThickness(2);
    window.draw(dot);

    sf::Text velocityText;
    velocityText.setFont(font);
    velocityText.setCharacterSize(12);
    velocityText.setFillColor(sf::Color::White);

    sf::Text accelerationText;
    accelerationText.setFont(font);
    accelerationText.setCharacterSize(12);
    accelerationText.setFillColor(sf::Color::White);

    auto vel = target.get_vel();
    std::stringstream velStream;
    velStream.precision(2);
    velStream << std::fixed << "Vel: (" << vel[0] << ", " << vel[1] << ")";
    velocityText.setString(velStream.str());

    auto acc = target.get_accel();
    std::stringstream accStream;
    accStream.precision(2);
    accStream << std::fixed << "Acc: (" << acc[0] << ", " << acc[1] << ")";
    accelerationText.setString(accStream.str());

    velocityText.setPosition(screenPos.x + 12, screenPos.y - 12);
    accelerationText.setPosition(screenPos.x + 12, screenPos.y + 4);

    window.draw(velocityText);
    window.draw(accelerationText);
}

void drawRadarScan(sf::RenderWindow &window, const Radar &radar, float screenSize, float worldSize){

}

int main()
{
    filesystem::path data_dir("../data");
    if (!filesystem::exists(data_dir))
    {
        if (!filesystem::create_directory(data_dir))
        {
            cerr << "Error creating directory: " << data_dir << endl;
            return 1;
        }
    }

    
    // Window setup
    const float SCREEN_SIZE = 800.0f;
    const float WORLD_SIZE = 400.0f;
    const float GRID_SPACING = 10.0f;
    
    Renderer renderer(SCREEN_SIZE, SCREEN_SIZE, WORLD_SIZE);

    float scan_interval = 1.0f;
    float next_scan_time = 0.0f;
    float log_interval = 0.1f;
    float next_log_time = 0.0f;

    // Targets
    vector<Body> targets;
    targets.push_back(Body({0, -25}));

    vector<bool> isDetected(targets.size(), false);

    Radar radar({0, 0}, 50.0f, 2.0f);
    auto radar_pos = radar.get_pos();
    sf::Vector2f radarScreenPos = worldToScreen(radar_pos[0], radar_pos[1], SCREEN_SIZE, WORLD_SIZE);

    bool isDragging = false;
    sf::Vector2i previousMousePosition;

    ofstream traj_file(data_dir / "trajectory.csv");
    ofstream detect_file(data_dir / "detections.csv");
    traj_file << "time,target_id,x,y,vx,vy\n";
    detect_file << "time,target_id,detected,distance,bearing,radial_velocity,actual_distance\n";

    cout << "Simulation started. Press SPACE to pause, R to reset, ESC to quit.\n";

    while (renderer.isRunning())
    {
        // Events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Space)
                    isPaused = !isPaused;
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                if (event.key.code == sf::Keyboard::R)
                {
                    // Reset
                    sim_time = 0.0f;
                    next_scan_time = 0.0f;
                    next_log_time = 0.0f;
                    targets.clear();
                    targets.push_back(Body({0, 0}, {5, 5}));
                }
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    isDragging = true;
                    previousMousePosition = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                }
            }
            if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    isDragging = false;
                }
            }
            if (event.type == sf::Event::MouseMoved)
            {
                if (isDragging)
                {
                    sf::Vector2i currentMousePosition = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
                    sf::Vector2f delta = window.mapPixelToCoords(previousMousePosition) - window.mapPixelToCoords(currentMousePosition);
                    ;
                    sf::View view = window.getView();
                    view.move(delta);
                    window.setView(view);

                    previousMousePosition = currentMousePosition;
                }
            }

            if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
            }
        }

        // Update simulation
        if (!isPaused)
        {
            // Update all targets
            for (auto &target : targets)
            {
                target.update(dt);
            }

            sim_time += dt;

            // Log trajectory data at intervals
            if (sim_time >= next_log_time)
            {
                for (size_t i = 0; i < targets.size(); i++)
                {
                    auto pos = targets[i].get_pos();
                    auto vel = targets[i].get_vel();
                    traj_file << sim_time << "," << i << ","
                              << pos[0] << "," << pos[1] << ","
                              << vel[0] << "," << vel[1] << "\n";
                }
                next_log_time += log_interval;
            }

            // Radar scan
            if (sim_time >= next_scan_time)
            {
                cout << "\n=== Scan at t=" << fixed << setprecision(1) << sim_time << "s ===\n";

                auto detections = radar.scan(targets, sim_time);
                cout << "Detected " << detections.size() << " targets.\n";

                // Update detection status
                fill(isDetected.begin(), isDetected.end(), false);

                // Log detections
                for (const auto &det : detections)
                {
                    auto tgt_pos = targets[det.target_id].get_pos();
                    float dx = tgt_pos[0] - radar_pos[0];
                    float dy = tgt_pos[1] - radar_pos[1];
                    float true_distance = sqrt(dx * dx + dy * dy);

                    detect_file << det.timestamp << "," << det.target_id << ",1,"
                                << det.distance << "," << det.azimuth << ","
                                << det.radial_velocity << "," << true_distance << "\n";

                    cout << "  Target " << det.target_id
                         << ": Distance=" << det.distance << "m"
                         << ", Bearing=" << det.azimuth << "°"
                         << ", Velocity=" << det.radial_velocity << "m/s\n";

                    isDetected[det.target_id] = true;
                }

                // Log misses
                for (size_t i = 0; i < targets.size(); i++)
                {
                    if (!isDetected[i])
                    {
                        detect_file << sim_time << "," << i << ",0,0,0,0,0\n";
                    }
                }

                next_scan_time += scan_interval;
            }
        }

        // Render
        window.clear(sf::Color::Black);
        drawGrid(window, SCREEN_SIZE, WORLD_SIZE, GRID_SPACING);
        drawRadar(window, radar, SCREEN_SIZE, WORLD_SIZE);

        for (size_t i = 0; i < targets.size(); i++)
        {
            drawTarget(window, targets[i], isDetected[i], radarScreenPos,
                       SCREEN_SIZE, WORLD_SIZE,
                       isDetected[i] ? colors[1] : colors[0],
                       font);
        }

        // Text overlay
        if (fontLoaded)
        {
            //window.setView(window.getDefaultView()); 
            stringstream ss;
            ss << fixed << setprecision(1) << "Time: " << sim_time << "s / 60s";
            if (isPaused)
                ss << " [PAUSED]";

            sf::Text text;
            text.setFont(font);
            text.setString(ss.str());
            text.setCharacterSize(16);
            text.setFillColor(sf::Color::White);
            text.setPosition(10, 10);
            window.draw(text);

            int detCount = 0;
            for (bool d : isDetected)
                if (d)
                    detCount++;

            text.setString("Detected: " + to_string(detCount) + "/" + to_string(targets.size()));
            text.setFillColor(sf::Color::Green);
            text.setPosition(10, 35);
            window.draw(text);

            text.setString("SPACE: Pause  |  R: Reset  |  ESC: Quit");
            text.setCharacterSize(12);
            text.setFillColor(sf::Color(200, 200, 200));
            text.setPosition(10, window.getSize().y - 20);
            window.draw(text);
        }

        window.display();
    }

    traj_file.close();
    detect_file.close();

    cout << "\n Trajectories --> data/trajectory.csv\n";
    cout << " Detections --> data/detection.csv\n";

    return 0;
}