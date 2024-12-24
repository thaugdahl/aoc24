#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <format>
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_map>

struct Vec2
{
    int64_t x;
    int64_t y;

    Vec2 operator+(const Vec2 &other) {
        return Vec2{x + other.x, y + other.y};
    }

    bool operator==(const Vec2 &other) const {
        return x == other.x && y == other.y;
    }
};

struct Vec2Hash {
    static std::size_t hash(Vec2 v) {
        return static_cast<std::size_t>(v.x ^ (v.y << 4));
    }

    std::size_t operator()(Vec2 p) const {
        return hash(p);
    }
};

struct Map
{
    int at(int64_t x, int64_t y) const {
        return heights[static_cast<std::size_t>(y)*static_cast<std::size_t>(nx) + static_cast<std::size_t>(x)];
    }

    int & at(int64_t x, int64_t y) {
        return heights[static_cast<std::size_t>(y)*static_cast<std::size_t>(nx) + static_cast<std::size_t>(x)];
    }

    const std::vector<Vec2> &nexts_at(int64_t x, int64_t y) const {
        return nexts[static_cast<std::size_t>(y)*static_cast<std::size_t>(nx) + static_cast<std::size_t>(x)];
    }


    void print(std::ostream &os)
    {
        for ( auto y = 0; y < ny; y++) {
            for ( auto x = 0; x < nx; x++)
            {
                const std::size_t idx = std::size_t(x) + std::size_t(y) * nx;
                if ( nexts[idx].size() > 0 ) {
                    os << nexts[idx].size();
                } else {
                    os << ".";
                }
            }
            os << "\n";
        }
    }

    using PartialPath = std::pair<Vec2, std::vector<Vec2>>;

    int64_t evaluate_trailhead(Vec2 pos) const
    {

        std::vector<PartialPath> stack;
        std::unordered_map<Vec2, bool, Vec2Hash> visited{};

        stack.emplace_back(std::make_pair(pos, std::vector<Vec2>{}));

        std::vector<PartialPath> resultingPaths{};

        while ( ! stack.empty() ) {

            auto [current_pos, current_path] = std::move(stack.back());
            stack.pop_back();

            // The popped position is not inside the map
            if ( ! isInside(current_pos.x, current_pos.y) ) {
                continue;
            }

            if ( visited[current_pos] ) {
                continue;
            }

            auto val = at(current_pos.x, current_pos.y );

            if ( val == 9 ) {
                resultingPaths.push_back(PartialPath{current_pos, current_path});
            }

            visited[current_pos] = true;

            auto &point_nexts = nexts_at(current_pos.x, current_pos.y);

            for ( auto &pt : point_nexts ) {
                // Current partial path
                std::vector<Vec2> new_partial_path = current_path;
                new_partial_path.emplace_back(current_pos);
                stack.emplace_back(std::make_pair(pt, new_partial_path));
            }
        }

        return int64_t(resultingPaths.size());
    }

    int64_t evaluate_trailhead_rating(Vec2 pos) const
    {

        std::vector<PartialPath> stack;
        // std::unordered_map<Vec2, bool, Vec2Hash> visited{};

        stack.emplace_back(std::make_pair(pos, std::vector<Vec2>{}));

        std::vector<PartialPath> resultingPaths{};

        while ( ! stack.empty() ) {

            auto [current_pos, current_path] = std::move(stack.back());
            stack.pop_back();

            // The popped position is not inside the map
            if ( ! isInside(current_pos.x, current_pos.y) ) {
                continue;
            }

            auto val = at(current_pos.x, current_pos.y );

            if ( val == 9 ) {
                resultingPaths.push_back(PartialPath{current_pos, current_path});
            }

            auto &point_nexts = nexts_at(current_pos.x, current_pos.y);

            for ( auto &pt : point_nexts ) {
                // Current partial path
                std::vector<Vec2> new_partial_path = current_path;
                new_partial_path.emplace_back(current_pos);
                stack.emplace_back(std::make_pair(pt, new_partial_path));
            }
        }

        return int64_t(resultingPaths.size());
    }

    std::vector<Vec2> find_trailheads() const
    {
        std::vector<Vec2> trailheads{};

        for ( auto it = std::begin(heights); it != std::end(heights); it++ )
        {
            auto idx = std::distance(std::begin(heights), it);

            auto y = idx / int64_t(nx);
            auto x = idx % int64_t(nx);

            if ( *it == 0 ) {
                trailheads.emplace_back(Vec2{x,y});
            }
        }

        return trailheads;
    }

    std::vector<Vec2> populate_next_at(Vec2 pos) {

        std::vector<Vec2> result{};

        if ( ! isInside(pos.x, pos.y) ) {
            return {};
        }

        auto val = at(pos.x, pos.y);


        static constexpr std::array<Vec2, 4> directions{
            Vec2{0,-1}, Vec2{0,1}, Vec2{-1,0}, Vec2{1,0}
        };

        for ( auto &dir : directions ) {
            Vec2 next = pos + dir;

            if ( isInside(next.x, next.y ) ) {
                if ( at(next.x, next.y) == val + 1 ) {
                    result.emplace_back(next);
                }
            }
        }

        return result;
    }

    void populate_nexts() {

        nexts.resize(heights.size());

        for ( auto i = 0; i < heights.size(); i++ )
        {
            std::vector<Vec2> point_nexts{};

            auto y = i / int64_t(nx);
            auto x = i % int64_t(nx);

            nexts[size_t(y) * nx + size_t(x)] = populate_next_at(Vec2{x,y});
        }
    }


    bool isInside(int64_t x, int64_t y) const {
        return x >= 0 && x < static_cast<int>(nx)
            && y >= 0 && y < static_cast<int>(ny);
    }

    Map(std::vector<int> && heights_, std::size_t nx_, std::size_t ny_)
        : heights{std::move(heights_)}, nx{nx_}, ny{ny_}
    {
        populate_nexts();
    }

private:
    std::vector<int> heights;

    std::vector<std::vector<Vec2>> nexts;

    std::size_t nx;
    [[maybe_unused]] std::size_t ny;
};


Map read_map(std::ifstream &fh) {
    std::vector<int> heights{};
    std::string current_line{};

    std::size_t ny = 0, nx = 0;

    while ( std::getline(fh, current_line) ) {
        ny++;
        std::size_t cnx{0};

        std::istringstream ss{current_line};
        char c;

        while ( ss >> c ) {
            cnx++;
            heights.emplace_back(c - '0');
        }

        nx = std::max(cnx, nx);
    }

    return Map(std::move(heights), nx, ny);
}

uint64_t task1(std::filesystem::path path)
{
    std::ifstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Couldn't open {}!\n", path.string());
    }

    Map map = read_map(fh);
    auto trailheads = map.find_trailheads();

    auto sum = 0;
    for ( auto trailhead : trailheads ) {
        sum += map.evaluate_trailhead_rating(trailhead);
    }

    map.print(std::cout);

    return std::size_t(sum);
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
