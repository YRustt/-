//
// Created by yrustt on 23.11.17.
//

#include <fstream>
#include <stdint.h>


int main() {
    std::fstream in("input.bin", std::ios::in | std::ios::binary);
    std::fstream out("output.bin", std::ios::out | std::ios::binary);

    uint32_t a, b, c;

    in.read((char*) &a, sizeof(uint32_t));
    in.read((char*) &b, sizeof(uint32_t));

    c = a + b;

    out.write((char*) &c, sizeof(uint32_t));

    in.close();
    out.close();
    return 0;
}