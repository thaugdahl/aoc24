#include <cstdlib>
#include <execution>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <filesystem>
#include <format>
#include <vector>

int64_t task1( std::filesystem::path file ) {

    std::fstream fs{file};

    if ( ! fs.is_open() )  {
        std::cerr << std::format("Failed to open {}\n", file.string());
        return -1;
    }

    int64_t a, b;

    std::vector<int64_t> left;
    std::vector<int64_t> right;


    while ( fs >> a >> b ) {
        left.emplace_back(a);
        right.emplace_back(b);
    }

    std::sort(std::begin(left), std::end(left));
    std::sort(std::begin(right), std::end(right));


    return std::transform_reduce(std::execution::par,
        std::begin(left), std::end(left),
        std::begin(right), int64_t{0},
        std::plus<>{},
        [] ( int64_t x, int64_t y ) { return std::abs(x-y); } );
}

int64_t task2( std::filesystem::path file ) {

    std::fstream fs{file};

    if ( ! fs.is_open() )  {
        std::cerr << std::format("Failed to open {}\n", file.string());
        return -1;
    }

    std::string line;

    int64_t a, b;

    std::vector<int64_t> left;
    std::vector<int64_t> right;


    while ( fs >> a >> b ) {
        left.emplace_back(a);
        right.emplace_back(b);
    }

    return std::accumulate(std::begin(left), std::end(left),
                    int64_t{0}, [&right] ( int64_t acc, int64_t x ) {
                            return acc + x * std::count(std::begin(right), std::end(right), x);
                       });
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

    auto result1 = task1(file_to_read);
    auto result2 = task2(file_to_read);

    std::cout << std::format("Task 1: {}\n", result1);
    std::cout << std::format("Task 2: {}\n", result2);

    return 0;
}
