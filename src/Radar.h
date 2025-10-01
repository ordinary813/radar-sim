#include <vector>

class Radar
{
private:
    std::vector<float> pos;
    float max_range;

public:
    Radar(std::vector<float> pos, float max_range);
    int scan();

    void update_max_range(float new_max_range) { this->max_range = new_max_range; }

    std::vector<float> get_pos() const { return pos; }
    float get_max_range() const { return max_range; }
};