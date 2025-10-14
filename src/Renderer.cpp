#include "Renderer.h"
#include "constraints.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;

Renderer::Renderer(float screen_height,
                   float screen_width,
                   float world_size,
                   float dt,
                   float sim_duration,
                   const float grid_spacing)
    : screen_height(screen_height),
      screen_width(screen_width),
      world_size(world_size),
      grid_spacing(grid_spacing),
      window(sf::VideoMode(screen_width, screen_height), "Radar Simulation"),
      dt(dt),
      sim_time(0.0f),
      sim_duration(sim_duration),
      isPaused(false),
      isDragging(false),
      currentMousePosition(sf::Vector2i(0, 0)),
      previousMousePosition(sf::Vector2i(0, 0)),
      colors{sf::Color::Green, sf::Color::Red}
{
    window.setFramerateLimit(60);
    if (!load_font())
    {
        cerr << "\033[31m" << "Error loading font into the renderer" << "\033[0m\n";
        exit(1);
    }
}

bool Renderer::load_font()
{
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
    return fontLoaded;
}

sf::Vector2f Renderer::worldToScreen(float x, float y)
{
    float xScale = screen_width / world_size;
    float yScale = screen_height / world_size;
    return sf::Vector2f(
        screen_width / 2 + x * xScale,   // Center of x axis + scaled x coordinate
        screen_height / 2 - y * yScale); // ...
}

void Renderer::draw_grid()
{
    for (float i = -world_size * 1000; i <= world_size * 1000; i += grid_spacing)
    {
        sf::Color color = (i == 0) ? sf::Color::White : sf::Color(50, 50, 50);
        sf::Vertex vLine[] = {
            sf::Vertex(worldToScreen(i, -world_size * 1000), color),
            sf::Vertex(worldToScreen(i, world_size * 1000), color)};
        sf::Vertex hLine[] = {
            sf::Vertex(worldToScreen(-world_size * 1000, i), color),
            sf::Vertex(worldToScreen(world_size * 1000, i), color)};
        window.draw(vLine, 2, sf::Lines);
        window.draw(hLine, 2, sf::Lines);
    }
}

void Renderer::draw_radar(const Radar &radar)
{
    auto radar_pos = radar.get_pos();
    sf::Vector2f pos = worldToScreen(radar_pos[0], radar_pos[1]);
    float rangeRadius = radar.get_max_range();

    float scaledRadius = rangeRadius * min(screen_width / world_size, screen_height / world_size);
    sf::CircleShape rangeCircle(scaledRadius);
    rangeCircle.setOrigin(scaledRadius, scaledRadius);
    rangeCircle.setPosition(pos);
    rangeCircle.setFillColor(sf::Color::Transparent);
    rangeCircle.setOutlineColor(sf::Color(0, 255, 0, 127));
    rangeCircle.setOutlineThickness(1);
    window.draw(rangeCircle);

    sf::CircleShape dot(4);
    dot.setOrigin(4, 4);
    dot.setPosition(pos);
    dot.setFillColor(sf::Color::Green);
    window.draw(dot);

    float radar_range = radar.get_max_range();

    float downAngle = radar.getScanAngle() - radar.getBeamWidth() / 2;
    if (downAngle >= 360.0f)
        downAngle -= 360;
    if (downAngle < 0)
        downAngle += 360;
    sf::Vector2f downWorld(radar_pos[0] + radar_range * cos(downAngle * M_PI / 180.0f),
                           radar_pos[1] + radar_range * sin(downAngle * M_PI / 180.0f));
    sf::Vector2f downPos = worldToScreen(downWorld.x, downWorld.y);
    sf::Vertex downLine[] = {
        sf::Vertex(pos, sf::Color(0, 255, 0, 127)),
        sf::Vertex(downPos, sf::Color(0, 255, 0, 127))};

    float upAngle = radar.getScanAngle() + radar.getBeamWidth() / 2;
    if (upAngle >= 360.0f)
        upAngle -= 360;
    if (upAngle < 0)
        upAngle += 360;

    sf::Vector2f upWorld(radar_pos[0] + radar_range * cos(upAngle * M_PI / 180.0f),
                         radar_pos[1] + radar_range * sin(upAngle * M_PI / 180.0f));
    sf::Vector2f upPos = worldToScreen(upWorld.x, upWorld.y);
    sf::Vertex upLine[] = {
        sf::Vertex(pos, sf::Color(0, 255, 0, 127)),
        sf::Vertex(upPos, sf::Color(0, 255, 0, 127))};

    window.draw(downLine, 2, sf::Lines);
    window.draw(upLine, 2, sf::Lines);
}

void Renderer::draw_body(const Body &body,
                         bool detected)
{
    auto pos = body.get_pos();
    sf::Vector2f screenPos = worldToScreen(pos[0], pos[1]);

    sf::Color color = detected ? colors[0] : sf::Color(0, 255, 0, 50);

    sf::CircleShape dot(8);
    dot.setOrigin(8, 8);
    dot.setPosition(screenPos);
    dot.setFillColor(color);
    window.draw(dot);

    // Text overlay
    sf::Text positionText;
    positionText.setFont(font);
    positionText.setCharacterSize(12);
    positionText.setFillColor(sf::Color::White);

    sf::Text velocityText;
    velocityText.setFont(font);
    velocityText.setCharacterSize(12);
    velocityText.setFillColor(sf::Color::White);

    sf::Text accelerationText;
    accelerationText.setFont(font);
    accelerationText.setCharacterSize(12);
    accelerationText.setFillColor(sf::Color::White);

    pos = body.get_pos();
    std::stringstream posStream;
    posStream.precision(1);
    posStream << std::fixed << "Pos: (" << pos[0] << ", " << pos[1] << ")";
    positionText.setString(posStream.str());

    auto vel = body.get_vel();
    std::stringstream velStream;
    velStream.precision(1);
    velStream << std::fixed << "Vel: (" << vel[0] << ", " << vel[1] << ")";
    velocityText.setString(velStream.str());

    auto acc = body.get_accel();
    std::stringstream accStream;
    accStream.precision(1);
    accStream << std::fixed << "Acc: (" << acc[0] << ", " << acc[1] << ")";
    accelerationText.setString(accStream.str());

    positionText.setPosition(screenPos.x + 12, screenPos.y - 16);
    velocityText.setPosition(screenPos.x + 12, screenPos.y);
    accelerationText.setPosition(screenPos.x + 12, screenPos.y + 16);

    window.draw(positionText);
    window.draw(velocityText);
    window.draw(accelerationText);
}

float Renderer::get_screen_height() { return screen_height; }

float Renderer::get_screen_width() { return screen_width; }

bool Renderer::isRunning() { return window.isOpen() && sim_time < sim_duration; }

bool Renderer::processEvents(sf::Event &event)
{
    return window.pollEvent(event);
}

void Renderer::close()
{
    window.close();
}

void Renderer::reset()
{
    sim_time = 0.0f;
}

void Renderer::render(const Radar &radar, vector<Body> &targets, vector<bool> &detected)
{
    window.clear(sf::Color::Black);
    draw_grid();
    draw_radar(radar);

    for (size_t i = 0; i < targets.size(); i++)
    {
        draw_body(targets[i], detected[i]);
    }

    // Text overlay
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
    for (bool d : detected)
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

    window.display();
}

void Renderer::flipPause()
{
    isPaused = !isPaused;
}

void Renderer::setMouseDragging(bool state, sf::Vector2i *previousMousePosition)
{
    if (state)
        this->previousMousePosition = *previousMousePosition;
    isDragging = state;
}

void Renderer::updateView(
    ViewAction action,
    sf::Vector2i *currentMousePosition,
    const sf::Event *event)
{
    sf::View view = window.getView();

    if (action == ViewAction::DRAG)
    {
        this->currentMousePosition = *currentMousePosition;
        sf::Vector2f delta = window.mapPixelToCoords(previousMousePosition) - window.mapPixelToCoords(this->currentMousePosition);
        sf::View view = window.getView();
        view.move(delta);
        window.setView(view);

        previousMousePosition = this->currentMousePosition;
    }

    if (action == ViewAction::RESIZE)
    {
        sf::FloatRect visibleArea(0, 0, event->size.width, event->size.height);
        window.setView(sf::View(visibleArea));
    }
}

float Renderer::advanceSimTime()
{
    sim_time += dt;
    return sim_time;
}
