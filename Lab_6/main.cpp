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

    uint32_t n;
    fin.read((char*) &n, sizeof(n));

    fout.write((char*) &n, sizeof(n));
    fout.write((char*) &n, sizeof(n));

    uint32_t offset_B = 8, offset_A = 16 + sizeof(char) * n * n;
    uint32_t block_size = 300, x_size = 0, y_size = 0, z_size;

    unsigned char* B = (unsigned char*) malloc(sizeof(unsigned char) * (block_size * block_size));
    unsigned char* A = (unsigned char*) malloc(sizeof(unsigned char) * (block_size * block_size));
    unsigned char* C = (unsigned char*) malloc(sizeof(unsigned char) * (block_size * block_size));


    for (uint32_t i = 0; i < n; i += block_size) {
        for (uint32_t k = 0; k < n; k += block_size) {
            x_size = block_size;
            if (i + block_size > n) {
                x_size = n - i;
            }
            z_size = block_size;
            if (k + block_size > n) {
                z_size = n - k;
            }

            for (uint32_t ib = 0; ib < x_size; ++ib) {
                for (uint32_t kb = 0; kb < z_size; ++kb) {
                    C[ib * block_size + kb] = 0;
                }
            }

            for (uint32_t j = 0; j < n; j += block_size) {
                y_size = block_size;
                if (j + block_size > n) {
                    y_size = n - j;
                }

//                printf("B:\n");
                for (uint32_t ib = 0; ib < x_size; ++ib) {
                    fin.seekg(offset_B + (i + ib) * n + j);
                    fin.read((char*) B + ib * block_size, y_size);
//                    for(uint32_t jb = 0; jb < y_size; ++jb) {
//                        printf("%c", B[ib * block_size + jb]);
//                    }
//                    printf("\n");
                }
//                printf("\nA:\n");
                for (uint32_t jb = 0; jb < y_size; ++jb) {
                    fin.seekg(offset_A + (j + jb) * n + k);
                    fin.read((char*) A + jb * block_size, z_size);
//                    for(uint32_t kb = 0; kb < z_size; ++kb) {
//                        printf("%c", A[jb * block_size + kb]);
//                    }
//                    printf("\n");
                }
//                printf("\n");
                for (uint32_t ib = 0; ib < x_size; ++ib) {
                    for (uint32_t kb = 0; kb < z_size; ++kb) {
                        unsigned char tmp = 0;
                        for (uint32_t jb = 0; jb < y_size; ++jb) {
                            unsigned char b = ((ib < x_size) && (jb < y_size)) ? B[ib * block_size + jb]: 0;
                            unsigned char a = ((jb < y_size) && (kb < z_size)) ? A[jb * block_size + kb]: 0;
                            tmp += a * b;
                        }
                        C[ib * block_size + kb] += tmp;
//                        int flag = 0;
//                        if ((ib < x_size) && (kb < z_size)) {
//                            printf("%d", C[ib * block_size + kb]);
//                            flag = 1;
//                        }
//                        if (flag) {
//                            printf("\n");
//                        }
                    }
                }

//                printf("\nC:\n");
//                for (uint32_t ib = 0; ib < x_size; ++ib) {
//                    for (uint32_t kb = 0; kb < z_size; ++kb) {
//                        printf("%c", C[ib * block_size + kb]);
//                    }
//                    printf("\n");
//                }
            }
            for (uint32_t ib = 0; ib < x_size; ++ib) {
                fout.seekp(8 + (i + ib) * n + k);
                fout.write((char*) C + ib * block_size, z_size);
            }
        }
    }


    free(B);
    free(A);
    free(C);
    fin.close();
    fout.close();
    return 0;
}