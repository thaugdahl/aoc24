#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <format>
#include <numeric>
#include <sstream>
#include <algorithm>
#include <vector>
#include <unordered_map>

constexpr std::size_t NUM_BLINKS = 75;


std::vector<uint64_t> blink(std::vector<uint64_t> &&stones) {
    std::vector<uint64_t> result{};

    for ( auto stone : stones ) {
        std::string string_repr = std::to_string(stone);
        std::size_t string_repr_len = string_repr.size();

        if ( stone == 0 ) {
            result.emplace_back(1ULL);
        } else if ( string_repr_len % 2 == 0 ) {
            std::string left = string_repr.substr(0, string_repr_len/2);
            std::string right = string_repr.substr(string_repr_len/2);

            result.emplace_back(std::stoull(left));
            result.emplace_back(std::stoull(right));

        } else {
            result.emplace_back(stone * 2024);
        }
    }

    return result;
}

uint64_t task1(std::filesystem::path path)
{
    std::ifstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Couldn't open {}!\n", path.string());
    }

    std::vector<uint64_t> stones{};

    uint64_t input_num{0};

    while ( fh >> input_num ) {
        stones.emplace_back(input_num);
    }

    fh.close();

    for ( auto i = 0; i < NUM_BLINKS; i++ ) {
        stones = blink(std::move(stones));
    }

    return stones.size();
}

std::unordered_map<uint64_t, uint64_t> blink_fast(std::unordered_map<uint64_t, uint64_t> &&stones)
{
    std::unordered_map<uint64_t, uint64_t> result{};


    for ( auto [stone, amt] : stones ) {

        std::string string_repr = std::to_string(stone);
        std::size_t string_repr_len = string_repr.size();

        if ( stone == 0 ) {
            result[1] += amt;
        } else if ( string_repr_len % 2 == 0 ) {
            std::string left = string_repr.substr(0, string_repr_len/2);
            std::string right = string_repr.substr(string_repr_len/2);

            uint64_t l = std::stoull(left);
            uint64_t r = std::stoull(right);

            result[l] += amt;
            result[r] += amt;
        } else {
            result[stone * 2024] += amt;
        }
    }

    return result;
}

uint64_t task2(std::filesystem::path path)
{
    std::ifstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Couldn't open {}!\n", path.string());
    }

    std::vector<uint64_t> stones{};

    uint64_t input_num{0};

    while ( fh >> input_num ) {
        stones.emplace_back(input_num);
    }

    fh.close();

    std::unordered_map<uint64_t, uint64_t> stones_map{};

    for ( auto stone : stones ) {
        // Zero-initialization?
        stones_map[stone]++;
    }

    for ( auto i = 0; i < NUM_BLINKS; i++ ) {
        stones_map = blink_fast(std::move(stones_map));
    }

    // Sum up
    return std::transform_reduce(std::begin(stones_map), std::end(stones_map), 0ULL,
            std::plus<>(), [] ( auto p ) {
                return p.second;
            });


    // return 0;
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

    auto t1 = task2(file_to_read);

    std::cout << std::format("Result: {}\n", t1);

    return 0;
}
