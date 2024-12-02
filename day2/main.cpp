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



bool safe_pair( int64_t fst, int64_t snd, int64_t sign )
{
    int64_t diff = snd - fst;
    if ( diff == 0 ) return false;

    int64_t new_sign = diff / std::abs(diff);
    return new_sign == sign && std::abs(diff) <= 3;
}

bool is_safe(std::vector<int64_t> &report)
{
    bool inc = false;
    bool dec = false;

    for ( auto [fst, snd] : report | std::views::adjacent<2> )
    {
        inc |= snd > fst;
        dec |= snd < fst;

        if ( std::abs(snd - fst) > 3 || snd == fst ) return false;
    }

    return inc != dec;
}

std::size_t task1(std::filesystem::path path)
{

    std::fstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Failed to open file {} for task 1\n", path.string());
    }

    std::string current_line;
    std::size_t safe_reports = 0;

    while ( std::getline(fh, current_line) ) {
        std::istringstream ss{current_line};
        std::vector<int64_t> report = consumeStream<int64_t>(ss);
        safe_reports += is_safe(report); // == true ? 1 : 0;
    }

    return safe_reports;
}



bool dampened_safe_report(std::vector<int64_t> &report)
{
    if ( is_safe(report) ) return true;

    for ( int i = 0; i < report.size(); i++ ) {
        std::vector<int64_t> copies{report};
        auto pos_iter = std::begin(copies);
        std::advance(pos_iter, i);
        copies.erase(pos_iter);

        if ( is_safe(copies) ) return true;
    }

    return false;
}

std::size_t task2(std::filesystem::path path)
{
    std::fstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Failed to open file {} for task 1\n", path.string());
    }

    std::string current_line;
    std::size_t safe_reports = 0;

    while ( std::getline(fh, current_line) ) {
        std::istringstream ss{current_line};
        std::vector<int64_t> report = consumeStream<int64_t>(ss);
        safe_reports += dampened_safe_report(report);
    }

    return safe_reports;
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
