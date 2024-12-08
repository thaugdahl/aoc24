#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <execution>
#include <ranges>
#include <string>
#include <filesystem>
#include <format>
#include <unordered_map>
#include <vector>

#define CONCAT

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

struct Equation {
    uint64_t lhs;
    std::vector<uint64_t> rhs;
};

std::vector<Equation> read_equations(std::ifstream &fh)
{
    std::string current_line{};

    std::vector<Equation> equations{};

    while ( std::getline(fh, current_line) ) {
        std::string_view sv{current_line};

        auto colon_iter = std::find(std::begin(sv), std::end(sv), ':');

        std::string lhs{std::begin(sv), colon_iter};
        std::string rhs{std::next(colon_iter), std::end(sv)};

        Equation current_eq{};
        current_eq.lhs = std::stoull(lhs);

        std::stringstream rhs_stream{std::string{rhs}};

        uint64_t operand = 0;

        while ( rhs_stream >> operand ) {
            current_eq.rhs.emplace_back(operand);
        }

        equations.emplace_back(std::move(current_eq));
    }

    return equations;
}


bool try_solve(Equation &eq, uint64_t current_value)
{
    if ( eq.rhs.size() == 0 ) return current_value == eq.lhs;

    Equation eq_copy = eq;

    bool success = false;

    uint64_t back = eq_copy.rhs.back();
    eq_copy.rhs.pop_back();

    // Plus
    {
        uint64_t next_value = current_value + back;
        success |= try_solve(eq_copy, next_value);
    }

    // Multiply
    {
        uint64_t next_value = current_value * back;
        success |= try_solve(eq_copy, next_value);
    }

#ifdef CONCAT
    {
        std::string lhs_str = std::to_string(current_value);
        uint64_t next_value = std::stoull(lhs_str + std::to_string(back));

        success |= try_solve(eq_copy, next_value);
    }
#endif

    return success;


}

bool try_solve(Equation &eq) {

    Equation eq_copy = eq;

    if ( eq.rhs.size() == 0 ) return false;

    // Reverse the operands to pop back for left-to-right
    std::reverse(std::begin(eq_copy.rhs), std::end(eq_copy.rhs));

    uint64_t start_value = eq_copy.rhs.back();
    eq_copy.rhs.pop_back();

    return try_solve(eq_copy, start_value);
}

uint64_t task1(std::filesystem::path path)
{

    std::ifstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Couldn't open {}!\n", path.string());
    }

    auto equations = read_equations(fh);


    return std::transform_reduce(std::execution::par, std::begin(equations), std::end(equations),
                          0uLL, std::plus<>{}, [] ( Equation &eq ) -> uint64_t {
                          bool can_solve = try_solve(eq);
                          if ( can_solve ) return eq.lhs;
                          else { return 0; }
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

    auto t1 = task1(file_to_read);

    std::cout << std::format("Result: {}\n", t1);

    return 0;
}
