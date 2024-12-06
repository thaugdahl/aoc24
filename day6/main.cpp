#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <filesystem>
#include <format>
#include <unordered_map>
#include <vector>
#include <ranges>


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
};

Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

struct Guard {
    Vec2 pos;
    Vec2 orientation;

    void turn_right() {
        int64_t x = orientation.x;
        orientation.x = -orientation.y;
        orientation.y = x;
    }
};


struct GuardMap {
    int64_t nx;
    int64_t ny;

    bool is_inside(int64_t x, int64_t y) const noexcept {
        return x >= 0 && x < nx && y >= 0 && y < ny;
    }

    template <class Self>
    auto &content_at(this Self &&self, int64_t x, int64_t y) {
        assert(x >= 0 && x < self.nx && "x outside bounds");
        assert(y >= 0 && y < self.ny && "y outside bounds");
        return std::forward<Self>(self).content[static_cast<std::size_t>(y * self.nx + x)];
    }

    template <class Self>
    auto &traversed_at(this Self &&self, int64_t x, int64_t y) {
        assert(x >= 0 && x < self.nx && "x outside bounds");
        assert(y >= 0 && y < self.ny && "y outside bounds");
        return std::forward<Self>(self).traversed[static_cast<std::size_t>(y * self.nx + x)];
    }

    bool obstacle_at(int64_t x, int64_t y) {
        assert(x >= 0 && x < nx && "x outside bounds");
        assert(y >= 0 && y < ny && "y outside bounds");

        return content[static_cast<std::size_t>(y * nx + x)] == '#';
    }

    std::string serialize() const {
        std::string result{};
        result.resize(content.size());

        std::ostringstream ss{};

        for ( auto y = 0; y < ny; y++ ) {
            for ( auto x = 0; x < nx; x++ ) {
                char reprChar = '.';

                if ( x == guard.pos.x && y == guard.pos.y ) reprChar = 'O';
                else if ( traversed_at(x, y) > 0 ) reprChar = 'X';
                else reprChar = content_at(x,y);

                ss << reprChar;
            }
        }

        return ss.str();
    }

    void print() const {
        for ( auto y = 0; y < ny; y++ ) {
            for ( auto x = 0; x < nx; x++ ) {
                char reprChar = '.';

                if ( x == guard.pos.x && y == guard.pos.y ) reprChar = 'O';
                else if ( traversed_at(x, y) > 0 ) reprChar = 'X';
                else reprChar = content_at(x,y);

                std::cout << reprChar;

            }

            std::cout << "\n";
        }
    }

    std::vector<char> content;
    std::vector<int> traversed;
    Guard guard;
};


GuardMap read_map(std::filesystem::path path)
{

    GuardMap result{};

    std::fstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Failed to open file {} for task 1\n", path.string());
    }

    std::string current_line;
    int64_t ny = 0;

    while ( std::getline(fh, current_line) && current_line != "" ) {
        ny++;

        result.nx = static_cast<int64_t>(current_line.length());
        result.content.insert(std::end(result.content), std::begin(current_line), std::end(current_line));
    }

    // Find the guard
    auto guardIt = std::find(std::begin(result.content), std::end(result.content), '^');
    assert(guardIt != std::end(result.content));

    int64_t guardIdx = std::distance(std::begin(result.content), guardIt);

    int64_t posX = guardIdx % static_cast<int64_t>(result.nx);
    int64_t posY = guardIdx / static_cast<int64_t>(result.nx);

    result.guard.orientation = {0, -1};
    result.guard.pos = {posX, posY};

    result.ny = ny;
    result.traversed.resize(result.content.size(), 0);

    return result;
}

///! Let the guard walk, returns an int if the guard successfully exits the map
std::pair<GuardMap, bool> guard_walk(GuardMap &&map) {
    // Look until obstacle found
    auto &pos = map.guard.pos;
    auto &orientation = map.guard.orientation;


    std::unordered_map<int64_t, int> encounteredTimes{};

    bool looped = false;

    Vec2 next = pos;
    while(map.is_inside(pos.x, pos.y) ) {
        // Mark position
        while ( map.is_inside(next.x, next.y) && ! map.obstacle_at(next.x, next.y)) {
            pos = next;
            next = pos + orientation;
            map.traversed_at(pos.x, pos.y)++;
        }


        // Continue turning right until no obstruction
        if ( map.is_inside(next.x, next.y) && map.obstacle_at(next.x, next.y) ) {
            map.guard.turn_right();
            next = pos + orientation;
        }

        if ( ! map.is_inside(next.x, next.y) ) break;

        int64_t serialized = pos.x + pos.y * map.nx;
        if ( ++encounteredTimes[serialized] > 5 ) {
            looped = true;
            break;
        }

    }


    return std::make_pair(map, looped);

}

int64_t task1(std::filesystem::path path)
{

    auto map = read_map(path);

    auto mapInstance = map;

    auto resultPair = guard_walk(std::move(mapInstance));

    return std::count_if(std::begin(resultPair.first.traversed), std::end(resultPair.first.traversed), [] (int p) {
        return p > 0;
    });


}

int64_t task2(std::filesystem::path path)
{
    auto map = read_map(path);

    // Need only place down obstacle in guard's path
    auto mapInstance = map;

    auto [initialWalk, _] = guard_walk(std::move(mapInstance));


    // Find all positions the guard traverses before placing obstruction
    std::vector<Vec2> walkedPositions{};
    for ( auto y = 0; y < initialWalk.ny; y++ ) {
        for ( auto x = 0; x < initialWalk.nx; x++ ) {
            if ( initialWalk.traversed_at(x, y) > 0 ) {
                walkedPositions.emplace_back(x, y);
            }
        }
    }

    // Keep track of how many places an obstacles would cause a loop
    int64_t loopWays = 0;

    // Try placing an obstacle in a walked position and let the guard walk
    for ( auto [px, py] : walkedPositions ) {
        auto currentMapInstance = map;
        currentMapInstance.content_at(px, py) = '#';
        auto [walk, looped] = guard_walk(std::move(currentMapInstance));

        if ( looped ) {
            loopWays++;
        }
    }


    return loopWays;
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
    auto t2 = task2(file_to_read);

    std::cout << std::format("Task 1: {}\n", t1);
    std::cout << std::format("Task 2: {}\n", t2);

    return 0;
}
