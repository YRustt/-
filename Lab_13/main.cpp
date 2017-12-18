
#include<stdio.h>
#include<stdint.h>
#include<map>


int main() {
    int32_t k, curr;
    scanf("%d", &k);

    std::map<int32_t, uint32_t> candidates;

    do {
        scanf("%d", &curr);
        if (curr == -1) {
            break;
        }

        if (curr == 0) {
            int32_t count = 0;
            for (auto it = candidates.begin(); it != candidates.end(); it++) {
                printf("%d ", it->first);
                count++;
            }
            while (count < k - 1) {
                printf("%d ", 1);
                count++;
            }

            printf("\n");
        }
        else {
            if (candidates.find(curr) != candidates.end()) {
                candidates[curr]++;
            } else {
                if (candidates.size() < k - 1) {
                    candidates[curr] = 1;
                } else {

                    std::map<int32_t, uint32_t> new_map;
                    for (auto it = candidates.begin(); it != candidates.end(); it++) {
                        it->second--;
                        if (it->second != 0) {
                            new_map[it->first] = it->second;
                        }
                    }
                    candidates = new_map;
                }
            }
        }

    } while (true);

    return 0;
}