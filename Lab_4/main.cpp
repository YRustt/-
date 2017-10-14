//
// Created by rustam on 6.10.17.
//
#include <iostream>
#include <vector>


std::vector<int> generate_input(int n, int seed) {
    std::vector<int> d(n * n);
    for (size_t i = 0; i < d.size(); ++i) {
        d[i] = seed;
        seed = ((long long) seed * 197 + 2017) & 987654;
    }
    return d;
}


long long get_hash(const std::vector<int>& d) {
    const long long MOD = 987654321054321LL;
    const long long MUL = 179;

    long long result_value = 0;
    for (size_t i = 0; i < d.size(); ++i)
        result_value = (result_value * MUL + d[i]) & MOD;
    return result_value;
}


void transpose(std::vector<int>& d, int i_min, int j_min, int size, int n) {
    int block_size = 8;
    int i, j, k, l;

    for (i = 0; i < size; i+=block_size) {
        for (j = 0; j < size; j+=block_size) {
            for (k = i; k < std::min(i + block_size, size); k++) {
                for (l = j; l < std::min(j + block_size, k); l++) {
                    std::swap(d[(i_min + k) * n + j_min + l], d[(i_min + l) * n + j_min + k]);
                }
            }
        }
    }
}


void print_matrix(const std::vector<int>& d, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cout << d[i * n + j] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    int n, seed, k;


    std::cin >> n >> seed >> k;

#ifdef DEBUG
    std::cout << n << " " << seed << std::endl;
#endif

    std::vector<int> d = generate_input(n, seed);

#ifdef DEBUG
    print_matrix(d, n);
#endif

    int i_min, j_min, size;
    for (int i = 0; i < k; i++) {
        std::cin >> i_min >> j_min >> size;

#ifdef DEBUG
        std::cout << i_min << " " << j_min << " " << size << std::endl;
#endif

        transpose(d, i_min, j_min, size, n);
    }

#ifdef DEBUG
    print_matrix(d, n);
#endif

    std::cout << get_hash(d);
    return 0;
}