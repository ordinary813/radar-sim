#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <ostream>
#include <SFML/Graphics.hpp>
#include "Radar.h"
#include "Body.h"

class Renderer
{
public:
    Renderer(float screen_height,
             float screen_width,
             float world_size,
             float sim_duration = 60.0f,
             const float grid_spacing = 10.0f);

    bool load_font();
    void draw_grid();
    void draw_radar(const Radar &radar);
    void draw_body(const Body &body,
                   bool detected,
                   const sf::Vector2f &radarPos);

    float get_screen_height();
    float get_screen_width();

    bool isRunning();
    void processEvents();

private:
    sf::RenderWindow window;
    sf::Font font;
    std::vector<sf::Color> colors;

    float screen_height;
    float screen_width;
    float world_size; // zoom level -/+
    const float grid_spacing;

    float dt;
    float sim_time;
    float sim_duration;
    bool isPaused;
};

#endif