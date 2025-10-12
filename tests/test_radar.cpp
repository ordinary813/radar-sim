#include "Radar.h"
#include "Body.h"
#include <iostream>
#include <cmath>
#include <sstream>

// Helper: compare floats with tolerance
bool almost_equal(float a, float b, float tol = 1e-3f)
{
    return std::fabs(a - b) < tol;
}

void check_equal(std::string name, float value, float expected, float tol = 1e-3)
{
    if (!almost_equal(value, expected, tol))
    {
        std::cerr << "\033[31m[X]\033[0m " << name
                  << "\nExcpected: \033[32m" << expected << "\033[0m"
                  << "\tGot: \033[31m" << value << "\033[0m"
                  << "\tDifference: \033[31m" << std::fabs(value - expected) << "\033[0m"
                  << std::endl;
        std::exit(1);
    }
    else
    {
        std::cout << "\033[32m[V]\033[0m " << name << "\t \033[32m" << value << "\033[0m" << std::endl;
    }
}

int main()
{
    std::stringstream ss;
    Radar radar({0, 0}, 50.0f, 2.0f);
    Body target1({1, 0});
    Body target2({0, 1});
    Body target3({-1, 0});
    Body target4({0, -1});

    std::cout << "Unit Vectors Test" << std::endl;

    float d = radar.calculateDistance(target1);
    float a = radar.calculateAzimuth(target1);
    std::vector<float> pos = target1.get_pos();
    ss << "Target1 " << target1 << " Distance";
    check_equal(ss.str(), d, 1.0f);
    ss.str(""); ss.clear();
    ss << "Target1 " << target1 << " Azimuth";
    check_equal(ss.str(), a, 0);
    ss.str(""); ss.clear();

    d = radar.calculateDistance(target2);
    a = radar.calculateAzimuth(target2);
    pos = target2.get_pos();
    ss << "Target2 " << target2 << " Distance";
    check_equal(ss.str(), d, 1.0f);
    ss.str(""); ss.clear();
    ss << "Target2 " << target2 << " Distance";
    check_equal(ss.str(), a, 90.0f);
    ss.str(""); ss.clear();

    d = radar.calculateDistance(target3);
    a = radar.calculateAzimuth(target3);
    pos = target3.get_pos();
    ss << "Target3 " << target3 << " Distance";
    check_equal(ss.str(), d, 1.0f);
    ss.str(""); ss.clear();
    ss << "Target3 " << target3 << " Distance";
    check_equal(ss.str(), a, 180.0f);
    ss.str(""); ss.clear();

    d = radar.calculateDistance(target4);
    a = radar.calculateAzimuth(target4);
    pos = target4.get_pos();
    ss << "Target4 " << target4 << " Distance";
    check_equal(ss.str(), d, 1.0f);
    ss.str(""); ss.clear();
    ss << "Target4 " << target4 << " Distance";
    check_equal(ss.str(), a, 270.0f);
    ss.str(""); ss.clear();

    std::cout << "\033[32m";
    std::cout << "Unit Vectors Test Success\n";
    std::cout << "\033[0m";

    // Azimuth Test
    Body target5({1, 1});
    Body target6({-1, 1});
    Body target7({-1, -1});
    Body target8({1, -1});

    std::cout << "Azimuth Test" << std::endl;

    a = radar.calculateAzimuth(target5);
    check_equal("Target5 Azimuth", a, 45.0f);

    a = radar.calculateAzimuth(target6);
    check_equal("Target6 Azimuth", a, 135.0f);

    a = radar.calculateAzimuth(target7);
    check_equal("Target7 Azimuth", a, 225.0f);

    a = radar.calculateAzimuth(target8);
    check_equal("Target8 Azimuth", a, 315.0f);

    std::cout << "\033[32m";
    std::cout << "Azimuth Test Success\n";
    std::cout << "\033[0m";

    std::cout << "\e[1;92m";
    std::cout << "All tests passed!\n";
    std::cout << "\033[0m";
    return 0;
}
