//
// Created by yrustt on 25.11.17.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <stdint.h>
#include <algorithm>


typedef uint32_t data_type;

uint32_t BLOCK_SIZE = 10000;

uint32_t tree_get(uint32_t x, std::vector<uint32_t>& tree) {
    uint32_t root = x;

    while (tree[root] != root) {
        root = tree[root];
    }

    uint32_t i = x, j;
    while (tree[i] != i) {
        j = tree[i];
        tree[i] = root;
        i = j;
    }

    return root;
}


void tree_union(uint32_t x, uint32_t y, std::vector<uint32_t>& tree) {
    uint32_t root_x = tree_get(x, tree);
    uint32_t root_y = tree_get(y, tree);

    if (root_x == root_y) {
        return;
    }
    if (root_x < root_y) {
        tree[root_y] = root_x;
    } else {
        tree[root_x] = root_y;
    }
}


int main() {
    std::ifstream in("input.bin", std::ios::in | std::ios::binary);
    std::ofstream out("output.bin", std::ios::out | std::ios::binary);

    uint32_t n, m;
    in.read((char*) &n, sizeof(data_type));
    in.read((char*) &m, sizeof(data_type));

    std::pair<uint32_t, uint32_t> block[BLOCK_SIZE];
    uint32_t read_size;

    std::vector<uint32_t> tree(n);
    for (uint32_t i = 0; i < n; ++i) {
        tree[i] = i;
    }

    uint32_t a, b;
    for (uint32_t i = 0; i < m; i += BLOCK_SIZE) {
        read_size = BLOCK_SIZE;
        if (i + BLOCK_SIZE >= m) {
            read_size = m - i;
        }

        in.read((char*) block, 8 * read_size);

        for (uint32_t j = 0; j < read_size; ++j) {
            a = block[j].first - 1;
            b = block[j].second - 1;
            tree_union(a, b, tree);
        }
    }

    for (uint32_t i = 0; i < n; ++i) {
        tree[i] = tree[tree[i]];
    }

    uint32_t count = 1, prev = tree[0];

    for (uint32_t i = 1; i < n; ++i) {
        if (tree[i] > prev) {
            count++;
            prev = tree[i];
        }
    }

    out.write((char*) &count, sizeof(uint32_t));

    in.close();
    out.close();

    return 0;
}