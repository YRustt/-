//
// Created by yrustt on 23.11.17.
//

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <algorithm>
#include <math.h>


int32_t BLOCK_SIZE = 10000;


void write_numbers() {
    std::ifstream in("input.bin", std::ios::in | std::ios::binary);

    std::ofstream num_1("num_1.bin", std::ios::out | std::ios::binary);
    std::ofstream num_2("num_2.bin", std::ios::out | std::ios::binary);

    uint32_t n, m;
    int32_t offset;
    uint8_t in_block[BLOCK_SIZE];
    uint8_t out_block[BLOCK_SIZE];

    uint32_t read_size;

    in.read((char*) &n, sizeof(uint32_t));
    num_1.write((char*) &n, sizeof(uint32_t));
    offset = 4;
    for (int32_t i = n + offset; i > offset; i -= BLOCK_SIZE) {
        read_size = BLOCK_SIZE;
        if (i - BLOCK_SIZE < offset) {
            read_size = i - offset;
        }

//        std::cout << i << " " << read_size << std::endl;

        in.seekg(i - read_size);
        in.read((char*) in_block, read_size);

        for (uint32_t j = 0; j < read_size; ++j) {
            out_block[read_size - 1 - j] = in_block[j];
        }

        num_1.write((char*) out_block, read_size);
    }
    num_1.close();

    offset = 4 + n;
    in.seekg(offset);
    in.read((char*) &m, sizeof(uint32_t));
    num_2.write((char*) &m, sizeof(uint32_t));

    offset = 4 + n + 4;

//    std::cout << n << " " <<  m << std::endl;
    for (int32_t i = m + offset; i > offset; i -= BLOCK_SIZE) {
        read_size = BLOCK_SIZE;
        if (i - BLOCK_SIZE < offset) {
            read_size = i - offset;
        }

        in.seekg(i - read_size);
        in.read((char*) in_block, read_size);

        for (uint32_t j = 0; j < read_size; ++j) {
            out_block[read_size - 1 - j] = in_block[j];
        }

        num_2.write((char*) out_block, read_size);
    }
    num_2.close();
}

void calc_sum() {
    std::ofstream out("out.bin", std::ios::out | std::ios::binary);
    std::ifstream ina("num_1.bin", std::ios::in | std::ios::binary);
    std::ifstream inb("num_2.bin", std::ios::in | std::ios::binary);

    int32_t n, m;
    uint8_t a[BLOCK_SIZE], b[BLOCK_SIZE];

    ina.read((char*) &n, sizeof(uint32_t));
    inb.read((char*) &m, sizeof(uint32_t));

    uint8_t carry = 0;
    int32_t count = std::max(n, m);
    int32_t a_read_size, b_read_size;

    for (int32_t i = 0; i < count; i += BLOCK_SIZE) {
        if (i < n) {
            a_read_size = BLOCK_SIZE;
            if (i + BLOCK_SIZE > n) {
                a_read_size = n - i;
            }
            ina.read((char *) a, a_read_size);
        }
        if (i < m) {
            b_read_size = BLOCK_SIZE;
            if (i + BLOCK_SIZE > m) {
                b_read_size = m - i;
            }
            inb.read((char *) b, b_read_size);
        }

        int32_t read_size = std::max(a_read_size, b_read_size);
        for (int32_t ib = 0; ib < read_size; ++ib) {
            if (i + ib >= n)
                a[ib] = 0;

            a[ib] += carry + (i + ib < m ? b[ib] : 0);
            carry = (a[ib] >= 10)? 1: 0;

            if (carry) a[ib] -= 10;
        }

        out.write((char*) a, read_size);

    }
    if (carry) {
        out.write((char*) &carry, sizeof(uint8_t));
    }

    out.close();
    ina.close();
    inb.close();
}

void write_output() {
    std::ifstream in("out.bin", std::ios::in | std::ios::binary);
    std::ofstream out("output.bin", std::ios::out | std::ios::binary);

    uint8_t in_block[BLOCK_SIZE], out_block[BLOCK_SIZE];

    in.seekg(0, std::ios::end);
    int32_t n = in.tellg() / sizeof(uint8_t);
    in.seekg(0);

    int32_t read_size;
    out.write((char*) &n, sizeof(uint32_t));
    for (int32_t i = n; i > 0; i -= BLOCK_SIZE) {
        read_size = BLOCK_SIZE;
        if (i - BLOCK_SIZE < 0) {
            read_size = i;
        }

        in.seekg(i - read_size);
        in.read((char*) in_block, read_size);

        for (int32_t j = 0; j < read_size; ++j) {
            out_block[read_size - 1 - j] = in_block[j];
        }

        out.write((char*) out_block, read_size);
    }
    out.close();
}

int main() {
    write_numbers();
    calc_sum();
    write_output();
    return 0;
}