//
// Created by yrustt on 3.12.17.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <vector>
#include <string>
#include <functional>


template<uint32_t size, uint32_t k>
class BloomFilter {
public:
    BloomFilter() {
        this->filter = std::vector<bool>(size, false);
    }
    void add(const std::string& s) {
        for (uint32_t i = 0; i < k; ++i) {
            this->filter[this->hash(i + 1, s)] = true;
        }
    }
    bool test(const std::string& s) {
        for (uint32_t i = 0; i < k; ++i) {
            if (!this->filter[this->hash(i + 1, s)]) {
                return false;
            }
        }
        return true;
    }
private:
    uint32_t hash(uint32_t i, const std::string& s) {
        return (i * hash_fn(s)) % size;
    }

    std::vector<bool> filter;
    std::hash<std::string> hash_fn;
};

int main() {
    std::ifstream in("input.txt");
    std::ofstream out("output.txt");
    uint32_t N, idx;
    std::string s;

    in >> N;

    BloomFilter<11000000, 10> filter;
    for (uint32_t i = 0; i < N; ++i) {
        in >> idx >> s;
        if (idx == 1) {
            filter.add(s);
        } else {
            out << (filter.test(s))? 1 : 0;
        }
    }

    return 0;
}