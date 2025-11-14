#include "xinnodb/src/alloc/include/alloc_api.hpp"
#include <cstddef>
#include <iostream>
int main() {
    std::cout << "offset of multimap_link: " << offsetof(alloc_free_block_header, multimap_link) << std::endl;
    std::cout << "offset of freelist_link: " << offsetof(alloc_pooled_free_block_header, freelist_link) << std::endl;
    return 0;
}
