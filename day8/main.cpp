#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <execution>
#include <ranges>
#include <set>
#include <string>
#include <filesystem>
#include <format>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static constexpr bool PART2 = true;

template <class TypeT,
    template <class Stored, class Allocator = std::allocator<Stored>> class Container = std::vector>
Container<TypeT> consumeStream(std::basic_istream<char> &stream)
{
    Container<TypeT> result{};

    TypeT current;

    while ( stream >> current ) {
        result.emplace_back(current);
    }

    return result;
}

struct Vec2 {
    int64_t x;
    int64_t y;

    bool operator==(Vec2 other) const {
        return x == other.x && y == other.y;
    }
};

template <class T>
struct Vec2_hash {
    size_t operator()(const T &v) const {
        return std::hash<int64_t>{}(v.x) ^ std::hash<int64_t>{}(v.y);
    }
};

struct AntennaMap {
    Vec2 bounds;
    std::unordered_map<char, std::vector<Vec2>> antennae;
    std::unordered_set<Vec2, Vec2_hash<Vec2>> antinodes;
};

AntennaMap read_antennae(std::ifstream &fh)
{

    AntennaMap result{};
    std::unordered_map<char, std::vector<Vec2>> &antennae = result.antennae;



    int64_t y = 0;

    std::string current_line;

    while ( std::getline(fh, current_line) ) {
        int64_t current_y = y++;

        for ( std::size_t x = 0; x < current_line.size(); x++ ) {
            char current_char = current_line[x];
            if ( current_char != '.' ) {
                antennae[current_char].emplace_back(x, current_y);
            }
        }

        // Wonk
        result.bounds.y = current_y+1;
        result.bounds.x = static_cast<int64_t>(current_line.size());
    }

    return result;
}

void insert_antinodes(AntennaMap &map, std::vector<Vec2> &antennae, auto &antinodes) {

    for ( std::size_t i = 1; i < antennae.size(); i++ ) {
        for ( std::size_t j = 0; j < i; j++ ) {

            Vec2 antenna1 = antennae[i];
            Vec2 antenna2 = antennae[j];


            int64_t dx = antenna2.x - antenna1.x;
            int64_t dy = antenna2.y - antenna1.y;

            auto insideBounds = [&map] (Vec2 v) -> bool {
                return v.x >= 0 && v.x < map.bounds.x
                    && v.y >= 0 && v.y < map.bounds.y;
            };

            Vec2 antinodeForward = {
                static_cast<int64_t>(antenna2.x + dx),
                static_cast<int64_t>(antenna2.y + dy),
            };

            Vec2 antinodeBackward = {
                static_cast<int64_t>(antenna1.x - dx),
                static_cast<int64_t>(antenna1.y - dy),
            };

            if constexpr ( PART2 ) {
                antinodes.insert(antenna1);
                antinodes.insert(antenna2);

                while ( insideBounds(antinodeForward) ) {
                    antinodes.insert(antinodeForward);
                    antinodeForward.x += dx;
                    antinodeForward.y += dy;
                }
            } else if ( insideBounds(antinodeForward) ) {
                antinodes.insert(antinodeForward);
            }

            if constexpr ( PART2 ) {
                while ( insideBounds(antinodeBackward) ) {
                    antinodes.insert(antinodeBackward);
                    antinodeBackward.x -= dx;
                    antinodeBackward.y -= dy;
                }
            } else if ( insideBounds(antinodeBackward) ) {
                antinodes.insert(antinodeBackward);
            }
        }
    }
}

void insert_all_antinodes(AntennaMap &map)
{
    for ( auto pair : map.antennae ) {
        // For all pairs
        insert_antinodes(map, pair.second, map.antinodes);
    }
}

char antenna_at(const AntennaMap &map, Vec2 coord)
{
    char repr = '.';

    for ( auto &pair : map.antennae ) {
        if ( std::find(std::begin(pair.second), std::end(pair.second), coord) != std::end(pair.second) )
            return pair.first;
    }

    return repr;
}

void print_map(const AntennaMap &map) {
    for ( int64_t y = 0; y < map.bounds.y; y++ ) {
        for ( int64_t x = 0; x < map.bounds.x; x++ ) {

            Vec2 coord(x,y);

            char repr = antenna_at(map, coord);


            if ( repr == '.' && map.antinodes.contains(coord) ) {
                repr = '#';
            }

            std::cout << repr;
        }
        std::cout << "\n";
    }
}


uint64_t task1(std::filesystem::path path)
{

    std::ifstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Couldn't open {}!\n", path.string());
    }

    auto map = read_antennae(fh);

    insert_all_antinodes(map);

    std::cout << std::format("Bounds {} x {}\n", map.bounds.x, map.bounds.y);

    auto result = map.antinodes.size();
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
