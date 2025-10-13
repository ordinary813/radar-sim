#include "Renderer.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

using namespace std;

Renderer::Renderer(float screen_height,
                   float screen_width,
                   float world_size,
                   float sim_duration,
                   const float grid_spacing)
    : screen_height(screen_height),
      screen_width(screen_width),
      world_size(world_size),
      grid_spacing(grid_spacing),
      window(sf::VideoMode(screen_width, screen_height), "Radar Simulation"),
      dt(0.0f),
      sim_time(0.0f),
      sim_duration(sim_duration),
      isPaused(false),
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
