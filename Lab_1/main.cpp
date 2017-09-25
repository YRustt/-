#include <iostream>
#include <sstream>
#include <random>
#include <ctime>

#define MY_TYPE double

//__declspec(align(16))
// __restrict
namespace {
    void MultSimple(const MY_TYPE* __restrict a, const MY_TYPE* __restrict b, MY_TYPE* __restrict c, int n)
    {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                c[i * n + j] = 0.f;
                for (int k = 0; k < n; ++k) {
                    c[i * n + j] += a[i * n + k] * b[k * n + j];
                }
            }
        }
    }

    void MultSimpleBlock(const MY_TYPE* __restrict a, const MY_TYPE* __restrict b, MY_TYPE* __restrict c, int n)
    {
        const int BLOCK = 10;
        for (int i = 0; i < n; i += BLOCK) {
            for (int j = 0; j < n; j += BLOCK) {
                for (int k = 0; k < n; k += BLOCK) {
                    for (int ib = 0; ib < BLOCK && i + ib < n; ++ib) {
                        for (int jb = 0; jb < BLOCK && j + jb < n; ++jb) {
                            if (k == 0) 
                                c[(i + ib) * n + (j + jb)] = 0.f;
                            for (int kb = 0; k + kb < n && kb < BLOCK; ++kb) {
                                c[(i + ib) * n + j + jb] += a[(i + ib) * n + k + kb] * b[(k + kb) * n + j + jb];
                            }                        
                        }
                    }
                }
            }
        }
    }
    
    void MultSwappedLoops(const MY_TYPE* __restrict a, const MY_TYPE* __restrict b, MY_TYPE* __restrict c, int n)
    {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                c[i * n + j] = 0.f;
            }

            for (int k = 0; k < n; ++k) {
                for (int j = 0; j < n; ++j) {
                    c[i * n + j] += a[i * n + k] * b[k * n + j];
                }
            }
        }
    }

    void FillRandom(MY_TYPE* a, int n)
    {
        std::default_random_engine eng;
        std::uniform_real_distribution<float> dist;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                a[i * n + j] = dist(eng);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    const int n = atoi(argv[1]);
    std::cout << n << "; ";

    MY_TYPE* a = new MY_TYPE[n * n];
    MY_TYPE* b = new MY_TYPE[n * n];
    MY_TYPE* c = new MY_TYPE[n * n];

    FillRandom(a, n);
    FillRandom(b, n);

    {
        const auto startTime = std::clock();
        MultSimple(a, b, c, n);
        const auto endTime = std::clock();

        // Simple
        std::cout << double(endTime - startTime) / CLOCKS_PER_SEC << "; ";
    }

    {
        const auto startTime = std::clock();
        MultSimpleBlock(a, b, c, n);
        const auto endTime = std::clock();

        // Block
        std::cout << double(endTime - startTime) / CLOCKS_PER_SEC << "; ";
    }
    
    {
        const auto startTime = std::clock();
        MultSwappedLoops(a, b, c, n);
        const auto endTime = std::clock();

        // Swapped
        std::cout << double(endTime - startTime) / CLOCKS_PER_SEC << '\n';
    }

    delete[] a;
    delete[] b;
    delete[] c;
}

