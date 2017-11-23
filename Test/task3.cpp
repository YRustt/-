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

    uint32_t offset_B = 8, offset_A = 8 + sizeof(char) * n * m;
    uint32_t block_size = 300, x_size = 0, y_size = 0, z_size;

    uint32_t n_block_size = block_size, m_block_size = block_size;

    if (m < block_size) {
        m_block_size = m;
        n_block_size = 1000 / m;
    }
    else if (n < block_size) {
        n_block_size = n;
        m_block_size = 1000 / n;
    }

    unsigned char* B = (unsigned char*) malloc(sizeof(unsigned char) * (n_block_size * m_block_size));
    unsigned char* A = (unsigned char*) malloc(sizeof(unsigned char) * m_block_size);
    unsigned char* C = (unsigned char*) malloc(sizeof(unsigned char) * n_block_size);

    for (uint32_t i = 0; i < n; i += n_block_size) {
        x_size = n_block_size;
        if (i + n_block_size > n) {
            x_size = n - i;
        }
        z_size = 1;

        for (uint32_t ib = 0; ib < x_size; ++ib) {
            C[ib] = 0;
        }

        for (uint32_t j = 0; j < m; j += m_block_size) {
            y_size = m_block_size;
            if (j + m_block_size > m) {
                y_size = m - j;
            }

            if (m > block_size) {
                for (uint32_t ib = 0; ib < x_size; ++ib) {
                        fin.seekg(offset_B + (i + ib) * m + j);
                        fin.read((char *) B + ib * m_block_size, y_size);

                }
            } else {
                fin.seekg(offset_B + i * m);
                fin.read((char *) B, x_size * y_size);
            }

            fin.seekg(offset_A + j);
            fin.read((char*) A, y_size);

            for (uint32_t ib = 0; ib < x_size; ++ib) {
                unsigned char tmp = 0;
                for (uint32_t jb = 0; jb < y_size; ++jb) {
                    unsigned char b = ((ib < x_size) && (jb < y_size)) ? B[ib * m_block_size + jb]: 0;
                    unsigned char a = (jb < y_size) ? A[jb]: 0;
                    tmp += a * b;
                }
                C[ib] += tmp;
            }
        }

        fout.seekp(i);
        fout.write((char*) C, x_size);
    }


    free(B);
    free(A);
    free(C);
    fin.close();
    fout.close();
    return 0;
}