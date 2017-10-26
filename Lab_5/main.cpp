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

    uint32_t block_size = 400, n_size = 0, m_size = 0;
    uint32_t n_block = block_size, m_block = block_size;
    uint32_t M = 200000;

    if (n < n_block) {
        n_block = n;
        m_block = M / n_block;

        char* __restrict B = (char*) malloc(sizeof(char) * (n_block * m_block));
        char* __restrict A = (char*) malloc(sizeof(char) * (m_block * n_block));

        for (uint32_t j = 0; j < m; j += m_block) {
            n_size = n_block;
            m_size = m_block;
            if (j + m_size > m) {
                m_size = m - j;
            }

            for (uint32_t ib = 0; ib < n_size; ++ib) {
                fin.seekg(8 + ib * m + j);
                fin.read(B + ib * m_block, m_size);
            }

            for (uint32_t ib = 0; ib < n_size; ++ib) {
                for (uint32_t jb = 0; jb < m_size; ++jb) {
                    A[jb * n_block + ib] = B[ib * m_block + jb];
                }
            }

            fout.seekp(8 + j * n);
            fout.write(A, m_size * n_size);
        }

        free(B);
        free(A);
        fin.close();
        fout.close();
        return 0;
    }

    if (m < m_block){
        m_block = m;
        n_block = M / m_block;

        char* __restrict B = (char*) malloc(sizeof(char) * (n_block * m_block));
        char* __restrict A = (char*) malloc(sizeof(char) * (m_block * n_block));

        for (uint32_t i = 0; i < n; i += n_block) {
            n_size = n_block;
            if (i + n_size > n) {
                n_size = n - i;
            }
            m_size = m_block;

            fin.seekg(8 + i * m);
            fin.read(B, m_size * n_size);

            for (uint32_t ib = 0; ib < n_size; ++ib) {
                for (uint32_t jb = 0; jb < m_size; ++jb) {
                    A[jb * n_block + ib] = B[ib * m_block + jb];
                }
            }

            for (uint32_t jb = 0; jb < m_size; ++jb) {
                fout.seekp(8 + jb * n + i);
                fout.write(A + jb * n_block, n_size);
            }
        }


        free(B);
        free(A);
        fin.close();
        fout.close();
        return 0;
    }

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
                fin.read(B + ib * block_size, m_size);
            }

            for (uint32_t ib = 0; ib < std::max(n_block, m_block); ++ib) {
                for (uint32_t jb = ib + 1; jb < std::max(n_block, m_block); ++jb) {
                    std::swap(B[jb * block_size + ib], B[ib * block_size + jb]);
                }
            }

            for (uint32_t jb = 0; jb < m_size; ++jb) {
                fout.seekp(8 + (j + jb) * n + i);
                fout.write(B + jb * block_size, n_size);
            }
        }
    }

    free(B);
    fin.close();
    fout.close();
    return 0;
}