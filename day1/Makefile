CXXFLAGS+=-fsanitize=address -std=c++20 -Werror -Wall -Wpedantic -Wunused -Wconversion -ltbb

main: main.cpp
	clang++ $(CXXFLAGS) -o main $+
