#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <format>
#include <iterator>
#include <numeric>
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_map>


struct Vec2 {
    int64_t x;
    int64_t y;

    Vec2 operator+(const Vec2 &other) {
        return Vec2{x + other.x, y + other.y};
    }
};

struct Vec2Hash {
    uint64_t operator()(Vec2 v) {
        return uint64_t(v.y << 3 ^ v.x);
    }
};

struct GardenPlot {

    unsigned char at(Vec2 pos) const {
        return plants[nx*std::size_t(pos.y) + std::size_t(pos.x)];
    }

    template <class Self>
    auto &component_at(this Self &&self, Vec2 pos) {
        return std::forward<Self>(self).componentIDs[self.nx*std::size_t(pos.y) + std::size_t(pos.x)];
    }


    bool isInside(Vec2 pos) {
        return pos.x >= 0 && pos.x < nx &&
            pos.y >= 0 && pos.y < ny;
    }


    std::vector<bool> visited;
    std::vector<uint64_t> componentIDs;

    uint64_t componentID_counter{1};

    std::vector<unsigned char> plants;
    std::size_t nx = 0, ny = 0;
};



GardenPlot read_garden(std::ifstream &fh)
{
    GardenPlot plot{};

    uint64_t nx{0}, ny{0};


    std::string current_line{};
    while ( std::getline(fh, current_line) ) {
        ny++;
        nx = std::max(nx,current_line.size());

        for ( auto c : current_line ) {
            plot.plants.emplace_back(c);
        }
    }

    plot.nx = nx;
    plot.ny = ny;

    plot.visited.resize(plot.plants.size(), false);
    plot.componentIDs.resize(plot.plants.size(), false);

    return plot;
}

static constexpr std::array<Vec2, 4> directions{
    Vec2{1,0},
    Vec2{-1,0},
    Vec2{0,1},
    Vec2{0,-1},
};

void fill_component_extension(GardenPlot &plot, Vec2 pos, unsigned char plant, uint64_t component)
{
    // Already visited
    if ( plot.component_at(pos) > 0 ) return;

    if ( plot.at(pos) == plant ) {
        plot.component_at(pos) = component;

        for ( auto dir : directions ) {
            Vec2 new_pos = pos + dir;

            if ( ! plot.isInside(new_pos) ) continue;

            fill_component_extension(plot, new_pos, plant, component);
        }
    }
}

void fill_component(GardenPlot &plot, Vec2 pos)
{
    unsigned  char plant = plot.at(pos);
    auto ID = plot.component_at(pos);

    if ( ID > 0 ) {
        return;
    }

    uint64_t new_component_id = plot.componentID_counter++;

    // plot.component_at(pos) = new_component_id;
    fill_component_extension(plot, pos, plant, new_component_id);
}



uint64_t task1(std::filesystem::path path)
{
    std::ifstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Couldn't open {}!\n", path.string());
    }


    auto plot = read_garden(fh);
    fh.close();

    for ( auto y = 0; y < plot.ny; y++ ) {
        for ( auto x = 0; x < plot.nx; x++ ) {
            fill_component(plot, Vec2{x,y});
        }
    }

    std::cout << "Task 1 finished\n";

    std::unordered_map<uint64_t, uint64_t> areas{};
    std::unordered_map<uint64_t, uint64_t> circumferences{};
    std::unordered_map<uint64_t, unsigned char> component_char{};

    for ( auto y = 0; y < plot.ny; y++ ) {
        for ( auto x = 0; x < plot.nx; x++ ) {

            unsigned char thisChar = plot.at({x,y});
            uint64_t component = plot.component_at(Vec2{x,y});
            component_char[component] = thisChar;

            areas[component] += 1;

            for ( auto dir : directions ) {
                Vec2 new_point = Vec2{x,y} + dir;

                if ( ! plot.isInside(new_point) ) {
                    circumferences[component]++;
                    continue;
                }

                if ( plot.at(new_point) != thisChar ) {
                    circumferences[component]++;
                }
            }
        }
    }

    std::cout << "task 1 finished\n";

    auto result = 0ULL;



    for ( auto i = 0ULL; i < plot.componentID_counter; i++ ) {
        std::cout << std::format("Component {} with char {} has area {} and circumference {}\n", i, char(component_char[i]), areas[i], circumferences[i]);
        result += circumferences[i] * areas[i];
    }

    return result;
}

int main(int argc, char *argv[])
{

    std::filesystem::path file_to_read = "input";

    if ( argc > 1 ) {
        file_to_read = argv[1];
    }

    if (  ! std::filesystem::exists(file_to_read) ) {
        std::cout << std::format("File {} does not exist\n", file_to_read.string()) << "\n";
    }

    auto t1 = task1(file_to_read);

    std::cout << std::format("Result: {}\n", t1);

    return 0;
}
