#include "Body.h"
#include "Radar.h"

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
    sf::CircleShape dot(6);
    dot.setOrigin(6, 6);
    dot.setPosition(pos);
    dot.setFillColor(sf::Color::Cyan);
    window.draw(dot);
}

void drawTarget(sf::RenderWindow &window, const Body &target, bool detected,
                const sf::Vector2f &radarPos, float screenSize, float worldSize,
                const vector<sf::Vector2f> &trail, sf::Color color, const sf::Font &font)
{
    auto pos = target.get_pos();
    sf::Vector2f screenPos = worldToScreen(pos[0], pos[1], screenSize, worldSize);

    // Trail
    if (trail.size() > 1)
    {
        sf::Color trailColor = color;
        trailColor.a = 100;
        for (size_t j = 1; j < trail.size(); j++)
        {
            sf::Vertex line[] = {
                sf::Vertex(trail[j - 1], trailColor),
                sf::Vertex(trail[j], trailColor)};
            window.draw(line, 2, sf::Lines);
        }
    }

    // Detection line
    if (detected)
    {
        sf::Vertex line[] = {
            sf::Vertex(radarPos, sf::Color(0, 255, 0, 150)),
            sf::Vertex(screenPos, sf::Color(0, 255, 0, 150))};
        window.draw(line, 2, sf::Lines);
    }

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

    sf::RenderWindow window(sf::VideoMode(SCREEN_SIZE, SCREEN_SIZE), "Radar Simulation");
    window.setFramerateLimit(60);

    sf::Font font;
    vector<string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:\\Windows\\Fonts\\arial.ttf"};
    bool fontLoaded = false;
    for (const auto &path : fontPaths)
    {
        if (font.loadFromFile(path))
        {
            fontLoaded = true;
            break;
        }
    }

    float dt = 0.016;
    float sim_time = 0.0f;
    float scan_interval = 1.0f;
    float next_scan_time = 0.0f;
    float log_interval = 0.1f;
    float next_log_time = 0.0f;
    bool isPaused = false;

    // CUSTOM TARGETS
    vector<Body> targets;
    targets.push_back(Body({0, 0}, {5, 5}));
    targets.push_back(Body({40, 40}, {-2, -3}, {1, 1}));
    targets.push_back(Body({-20, 45}, {3, -3}, {-1, 1}));

    vector<sf::Color> colors = {
        sf::Color::Red,
        sf::Color::Green};

    vector<vector<sf::Vector2f>> trails(targets.size());
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

    while (window.isOpen() && sim_time < 60.0f)
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
                    targets.push_back(Body({40, 40}, {-2, -3}));
                    targets.push_back(Body({-20, 45}, {3, -3}));
                    for (auto &trail : trails)
                        trail.clear();
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
        }

        // Update simulation
        if (!isPaused)
        {
            // Update all targets
            for (auto &target : targets)
            {
                target.update(dt);
            }

            // Add to trails
            for (size_t i = 0; i < targets.size(); i++)
            {
                auto pos = targets[i].get_pos();
                sf::Vector2f screenPos = worldToScreen(pos[0], pos[1], SCREEN_SIZE, WORLD_SIZE);
                trails[i].push_back(screenPos);
                if (trails[i].size() > 500)
                    trails[i].erase(trails[i].begin());
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
                       SCREEN_SIZE, WORLD_SIZE, trails[i],
                       isDetected[i] ? colors[1] : colors[0],
                       font);
        }

        // Text overlay
        if (fontLoaded)
        {
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
            text.setPosition(10, SCREEN_SIZE - 20);
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