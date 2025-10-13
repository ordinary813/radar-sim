#include "Renderer.h"
#include "constraints.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

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
      colors{sf::Color::Red, sf::Color::Green}
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

void Renderer::draw_grid()
{
}

void Renderer::draw_radar(const Radar &radar)
{
}

void Renderer::draw_body(const Body &body,
                         bool detected,
                         const sf::Vector2f &radarPos)
{
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

    sf::Vector2f radarScreenPos = worldToScreen(radar_pos[0], radar_pos[1], SCREEN_SIZE, WORLD_SIZE);

    for (size_t i = 0; i < targets.size(); i++)
    {
        draw_body(targets[i], detected[i], radarScreenPos);
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

void Renderer::advanceSimTime()
{
    sim_time += dt;
}