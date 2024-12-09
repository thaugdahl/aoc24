#include <cassert>
#include <execution>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <format>
#include <numeric>
#include <ranges>
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

enum class BlockType {
    FREE,
    FILE,
    INVALID
};

struct Block {
    BlockType type;
    std::size_t length;
    int64_t ID;

    explicit Block(BlockType type_, std::size_t length_, int64_t ID_)
        : type{type_}, length{length_}, ID{ID_}
    { }

    void print() {
        if ( type == BlockType::FILE ) {
            for ( std::size_t i = 0; i < length; i++ ) {
                std::cout << ID;
            }
        } else {
            for ( std::size_t i = 0; i < length; i++ ) {
                std::cout << '.';
            }
        }
    }
};


auto print_blocks(std::deque<Block> &blocks) {

    for ( auto &b : blocks ) {
        std::string repr = std::to_string(b.ID);

        if ( b.type == BlockType::FREE ) {
            repr = std::format("[{}]", b.length);
            std::cout << repr;
            continue;
        }

        for ( [[maybe_unused]] auto i : std::ranges::views::iota(0uLL, b.length) ) {
            std::cout << repr;
        }
    }

    std::cout << "\n";

}

bool is_compacted(std::deque<Block> &blocks)
{
    auto firstFree = std::find_if(std::begin(blocks), std::end(blocks), [] (Block b){
        return b.type == BlockType::FREE;
    });

    auto fileAfterFree = std::find_if(firstFree, std::end(blocks), [] (Block b) {
        return b.type == BlockType::FILE;
    });

    if ( fileAfterFree != std::end(blocks) ) {
        return false;
    }

    return true;
}

std::deque<Block> compact(std::deque<Block> &blocks)
{
    std::deque<Block> compacted = blocks;


    std::size_t iter_limit = 5000;

    while ( ! is_compacted(compacted) && iter_limit-- ) {

        // Backmost file
        auto backmost_file_iter = std::find_if(std::rbegin(compacted), std::rend(compacted), [] (Block &b) {
            return b.type == BlockType::FILE;
        });

        Block &back = *backmost_file_iter;
        Block copy = back;

        back.type = BlockType::FREE;
        back.ID = -1;

        std::size_t len_left = copy.length;

        auto firstFree = std::begin(compacted);


        while ( len_left > 0 ) {
            firstFree = std::find_if(firstFree, std::end(compacted), [] (Block &b) {
                return b.type == BlockType::FREE;
            });

            Block &to_alloc = *firstFree;

            const std::size_t target_block_size = to_alloc.length;
            const std::size_t moved_block_size = len_left;

            const std::size_t len_to_alloc = std::min(moved_block_size, target_block_size);
            const std::size_t new_len_left = moved_block_size - len_to_alloc;
            const std::size_t remaining_free_space = target_block_size - moved_block_size;

            to_alloc.type = BlockType::FILE;
            to_alloc.ID = copy.ID;
            to_alloc.length = len_to_alloc;

            if ( target_block_size > moved_block_size ) {
                // Insert block after
                Block newFreeBlock = Block{BlockType::FREE, remaining_free_space, -1};
                compacted.insert(std::next(firstFree), newFreeBlock);


            }
            len_left = new_len_left;
        }
    }

    return compacted;
}


auto find_first_free_slot(std::deque<Block> &blocks, std::size_t req_size = 1)
{
    return std::find_if(
        std::begin(blocks),
        std::end(blocks),
        [req_size] (Block &b) -> bool {
            bool isFile = b.type == BlockType::FREE;
            bool hasSpace = b.length >= req_size;
            return isFile && hasSpace;
        });
}


std::deque<Block> combine_empty_blocks(std::deque<Block> &&blocks)
{
    std::deque<Block> compacted{};

    const auto N = blocks.size();

    for ( std::size_t i = 0; i < N; ) {

        if ( blocks[i].type == BlockType::FREE ) {
            std::size_t j = i;

            std::size_t accumulated_free_space = 0;
            while ( j < N && blocks[j].type == BlockType::FREE ) {
                accumulated_free_space += blocks[j++].length;
            }

            compacted.emplace_back(Block{BlockType::FREE, accumulated_free_space, -1});

            i = j;
        } else {
            compacted.push_back(blocks[i++]);
        }
    }

    return compacted;
}

std::deque<Block> compact_fit(std::deque<Block> &blocks)
{
    std::deque<Block> compacted = blocks;

    auto file_iter = std::rbegin(compacted);
    std::size_t iter_limit = 200;

    while ( file_iter != std::rend(compacted) && iter_limit ) {
        file_iter = std::find_if(file_iter, std::rend(compacted), [] (Block &b) {
            return b.type == BlockType::FILE;
        });

        Block &candidateFile = *file_iter;

        // Find first available slot
        auto free_slot_iter = find_first_free_slot(compacted, candidateFile.length);

        // Check if this spot is past the file
        auto forward_file_iter = (file_iter + 1).base();
        if ( std::distance(forward_file_iter, free_slot_iter) > 0 ) {
            // Past the file, cannot be moved
            file_iter = std::next(file_iter);
            continue;
        }

        // Can be moved
        Block copy = candidateFile;

        candidateFile.ID = -1;
        candidateFile.type = BlockType::FREE;

        // Move block into free space
        std::size_t remaining_free_space = (*free_slot_iter).length - candidateFile.length;

        *free_slot_iter = copy;
        if ( remaining_free_space > 0 ) {
            compacted.insert(std::next(free_slot_iter), Block{BlockType::FREE, remaining_free_space, -1});

            file_iter = std::find_if(std::rbegin(compacted), std::rend(compacted), [&copy] (Block &b) {
                return b.ID == copy.ID - 1;
            });
        }
    }


    return combine_empty_blocks(std::move(compacted));}

std::size_t blocks_checksum(std::deque<Block> &blocks)
{
    std::size_t total = 0;
    std::size_t pos = 0;
    for ( auto &b : blocks ) {
        if ( b.type == BlockType::FILE ) {
            std::size_t offset = 0;
            while (  offset < b.length ) {
                total += (pos + offset) * static_cast<std::size_t>(b.ID);
                offset++;
            }
        }

        pos += b.length;
    }

    return total;
}



uint64_t task1(std::filesystem::path path)
{

    std::ifstream fh{path};

    if ( ! fh.is_open() ) {
        std::cerr << std::format("Couldn't open {}!\n", path.string());
    }

    char c;

    bool currentType = true;

    std::deque<Block> blocks{};

    int64_t currentID = 0;
    while ( fh >> c ) {
        // Assumes all input characters are between 0-9

        if ( c < '0' || c > '9' ) {
            std::cerr << "Got invalid input\n";
            std::abort();
        }

        std::size_t len = static_cast<std::size_t>(c - '0');

        BlockType selectedBlockType = currentType ? BlockType::FILE : BlockType::FREE;
        int64_t blockID = selectedBlockType == BlockType::FILE ? currentID : -1;

        blocks.emplace_back(Block{selectedBlockType, len, blockID});

        if ( selectedBlockType == BlockType::FILE ) {
            currentID++;
        }

        currentType = !currentType;
    }

    decltype(compact_fit(blocks)) compacted{};

    if constexpr ( PART2 ) {
        compacted = compact_fit(blocks);
    } else {
        compacted = compact(blocks);
    }

    auto checksum = blocks_checksum(compacted);
    return checksum;
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
