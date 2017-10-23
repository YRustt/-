//
// Created by rustam on 22.10.17.
//

#include <fstream>
#include <cstdlib>
#include <stdint.h>
#include <algorithm>


int main() {
    std::ifstream fin("input.bin", std::ios::in | std::ios::binary);
    std::ofstream fout("output.bin", std::ios::out | std::ios::binary);

    if (!fin || !fout) {
        return 1;
    }

    uint32_t n, m;
    fin.read((char*) &n, sizeof(n));
    fin.read((char*) &m, sizeof(m));

    fout.write((char*) &m, sizeof(m));
    fout.write((char*) &n, sizeof(n));

    uint32_t block_size = 1024, n_size = 0, m_size = 0;

    char* B = (char*) malloc(sizeof(char) * (block_size * block_size));

    for (uint32_t i = 0; i < n; i += block_size) {
        for (uint32_t j = 0; j < m; j += block_size) {
            n_size = block_size;
            if (i + block_size > n) {
                n_size = n - i;
            }
            m_size = block_size;
            if (j + block_size > m) {
                m_size = m - j;
            }

            for (uint32_t ib = 0; ib < n_size; ++ib) {
                fin.seekg(8 + (i + ib) * m + j);
                fin.read(B[ib * m], m_size);
            }

            for (uint32_t ib = 0; ib < block_size; ++ib) {
                for (uint32_t jb = ib + 1; jb < block_size; ++jb) {
                    std::swap(B[ib * block_size + jb], B[jb * block_size + ib]);
                }
            }

            for (uint32_t jb = 0; jb < m_size; ++jb) {
                fout.seekp(8 + (j + jb) * n + i);
                fout.write(B + jb * block_size, n_size);
            }

        }
    }


    free(B);
    free(line);
    fin.close();
    fout.close();
    return 0;
}