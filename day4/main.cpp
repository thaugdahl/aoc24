#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <filesystem>
#include <format>
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



std::size_t find_xmas(const std::vector<std::vector<char>> &instance, std::size_t N, std::size_t y, std::size_t x, int dy, int dx, std::size_t progress)
{
    static constexpr std::string_view fullstring = "XMAS";
    char to_find = fullstring[progress];

    if ( y < 0 || y >= N ) return 0;
    if ( x < 0 || x >= N ) return 0;


    if ( instance[y][x] == to_find ) {
        if ( progress == fullstring.size() - 1 ) {
            return 1;
        }

        return find_xmas(instance, N, y + static_cast<std::size_t>(dy), x + static_cast<std::size_t>(dx), dy, dx, progress + 1);

    } else {
        return 0;
    }
}

std::size_t task1(std::filesystem::path path)
{

    std::fstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Failed to open file {} for task 1\n", path.string());
    }

    std::string current_line;

    std::vector<std::vector<char>> problem{};


    while ( std::getline(fh, current_line) ) {
        std::istringstream ss{current_line};
        problem.emplace_back(consumeStream<char>(ss));
    }

    std::size_t all = 0;
    std::size_t N = problem[0].size();

    for ( std::size_t y = 0; y < problem.size(); y++ ) {
        for ( std::size_t x = 0; x < problem[0].size(); x++ ) {
            all += find_xmas(problem, N, y, x, -1, -1, 0);
            all += find_xmas(problem, N, y, x, -1, 0, 0);
            all += find_xmas(problem, N, y, x, -1, 1, 0);
            all += find_xmas(problem, N, y, x, 0, 1, 0);
            all += find_xmas(problem, N, y, x, 1, 1, 0);
            all += find_xmas(problem, N, y, x, 1, 0, 0);
            all += find_xmas(problem, N, y, x, 1, -1, 0);
            all += find_xmas(problem, N, y, x, 0, -1, 0);
        }
    }

    std::cout << all << "\n";

    return 0;
}

std::size_t task2(std::filesystem::path path)
{

    std::fstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Failed to open file {} for task 1\n", path.string());
    }

    std::string current_line;

    std::vector<std::vector<char>> problem{};


    while ( std::getline(fh, current_line) ) {
        std::istringstream ss{current_line};
        problem.emplace_back(consumeStream<char>(ss));
    }

    std::size_t all = 0;

    for ( std::size_t y = 1; y < problem.size()-1; y++ ) {
        for ( std::size_t x = 1; x < problem[0].size()-1; x++ ) {
            if ( problem[y][x] == 'A' ) {
                // Check above and below
                const bool verticalForwardMAS = problem[y-1][x-1] == 'M' && problem[y+1][x+1] == 'S';
                const bool verticalBackwardMAS = problem[y-1][x-1] == 'S' && problem[y+1][x+1] == 'M';
                const bool horizontalForwardMAS = problem[y-1][x+1] == 'M' && problem[y+1][x-1] == 'S';
                const bool horizontalBackwardMAS = problem[y-1][x+1] == 'S' && problem[y+1][x-1] == 'M';

                const bool verticalMAS = verticalForwardMAS || verticalBackwardMAS;
                const bool horizontalMAS = horizontalForwardMAS || horizontalBackwardMAS;

                if ( verticalMAS && horizontalMAS ) all++;
            }
        }
    }

    std::cout << all << "\n";

    return 0;
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
