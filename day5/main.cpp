#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <numeric>
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




int task1(std::filesystem::path path)
{

    std::fstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Failed to open file {} for task 1\n", path.string());
    }

    std::string current_line;

    std::vector<std::pair<int, int>> orderings{};

    while ( std::getline(fh, current_line) && current_line != "" ) {
        std::istringstream ss{current_line};

        char sep;
        int page1;
        int page2;

        ss >> page1 >> sep >> page2;

        orderings.emplace_back(std::make_pair(page1, page2));
    }

    int sumOfMiddles = 0;

    while ( std::getline(fh, current_line) ) {
        std::vector<int> pages{};
        for ( const auto &word : std::views::split(current_line, ',') ) {
            int pageNo = std::stoi(std::string{std::begin(word), std::end(word)});
            pages.emplace_back(pageNo);
        }


        bool isValid = true;
        for ( std::size_t i = 1; i < pages.size(); i++ ) {
            // Try to find a rule that doesn't match
            int currentPageNo = pages[i];

            for ( std::size_t j = 0; j < i; j++ ) {
                int candidatePageNo = pages[j];

                for ( auto [p1, p2] : orderings ) {
                    if ( candidatePageNo == p2 && currentPageNo == p1 ) {
                        isValid = false;
                    }
                }
            }
        }


        if ( isValid ) {
            sumOfMiddles += pages[(pages.size() + 1) / 2 - 1];
        }

    }


    return sumOfMiddles;

}


int task2(std::filesystem::path path)
{

    std::fstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Failed to open file {} for task 1\n", path.string());
    }

    std::string current_line;

    std::vector<std::pair<int, int>> orderings{};

    while ( std::getline(fh, current_line) && current_line != "" ) {
        std::istringstream ss{current_line};

        char sep;
        int page1;
        int page2;

        ss >> page1 >> sep >> page2;

        orderings.emplace_back(std::make_pair(page1, page2));

        // std::cout << std::format("Page {} must come before Page {}\n",page1,page2);
    }

    int sumOfMiddles = 0;

    while ( std::getline(fh, current_line) ) {
        std::vector<int> pages{};
        for ( const auto &word : std::views::split(current_line, ',') ) {
            int pageNo = std::stoi(std::string{std::begin(word), std::end(word)});
            pages.emplace_back(pageNo);
        }


        bool isValid = true;
        for ( std::size_t i = 1; i < pages.size(); i++ ) {
            // Try to find a rule that doesn't match

            // Every occurring number should be first up to the point
            int currentPageNo = pages[i];


            for ( std::size_t j = 0; j < i; j++ ) {
                int candidatePageNo = pages[j];

                for ( auto [p1, p2] : orderings ) {
                    if ( candidatePageNo == p2 && currentPageNo == p1 ) {
                        isValid = false;
                        std::swap( pages[i], pages[j]);
                    }
                }
            }
        }


        if ( ! isValid ) {
            sumOfMiddles += pages[(pages.size() + 1) / 2 - 1];
        }
    }

    return sumOfMiddles;

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
