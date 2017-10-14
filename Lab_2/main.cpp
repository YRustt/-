#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>


int main(int argc, char *argv[])
{
    int n = atoi(argv[1]), m = atoi(argv[2]);

    for (size_t _ = 0; _ < 5; _++) {
        std::default_random_engine generator;
        std::uniform_real_distribution<double> distribution(0.0, 1.0);

        std::vector<int> degs(n * m, 0);
        std::vector<std::vector<int>> g(n * m, std::vector<int>());
        std::vector<std::vector<int>> f(n, std::vector<int>());

        g[0].push_back(0);
        degs[0] = 2;

        for (int i = 1; i < n * m; ++i) {
            double num_gen = distribution(generator), cum_sum = 0.0;
            degs[i] += 1;

            for (int j = 0; j <= i; ++j) {
                double from = cum_sum / (2 * (i + 1) - 1), to = (cum_sum + degs[j]) / (2 * (i + 1) - 1);

                cum_sum += degs[j];
                if (num_gen >= from && num_gen < to)
                {
                    g[i].push_back(j);
                    if (i != j) {
                        g[j].push_back(i);
                    }
                    degs[j] += 1;
                    break;
                }
            }
        }

        for (size_t i = 0; i < g.size(); ++i) {
            for (size_t j = 0; j < g[i].size(); ++j) {
                f[(int) i / m].push_back((int) g[i][j] / m);
            }
        }

        std::ofstream file("data/result_" + std::to_string(_) + "_" + std::to_string(n) + "_" + std::to_string(m) + ".txt");

        for (size_t i = 0; i < f.size(); ++i) {
            for (size_t j = 0; j < f[i].size(); ++j) {
                file << f[i][j] << " ";
            }
            file << "\n";
        }

        file << "\n";

        file.close();
    }
    return 0;
}