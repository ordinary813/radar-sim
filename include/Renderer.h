#ifndef RENDERER_H
#define RENDERER_H

#include "Radar.h"
#include "Body.h"
#include "constraints.h"

#include <vector>
#include <ostream>
#include <SFML/Graphics.hpp>

class Renderer
{
public:
    Renderer(float screen_height,
             float screen_width,
             float world_size,
             float dt = 0.016f,
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
    bool processEvents(sf::Event &event);

    void close();
    void reset();
    void render(const Radar &radar, vector<Body> &targets, vector<bool> &detected);

    void flipPause();
    void setMouseDragging(
        bool state,
        sf::Vector2i *previousMousePosition = nullptr);

    void updateView(
        ViewAction action,
        sf::Vector2i *currentMousePosition = nullptr,
        const sf::Event *event);

    void advanceSimTime();

    bool isDragging;
    bool isPaused;

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

    sf::Vector2i currentMousePosition;
    sf::Vector2i previousMousePosition;
};

#endif