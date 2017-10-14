//
// Created by rustam on 10.10.17.
//

#include <vector>
#include <iostream>


typedef int time_type;
typedef std::vector<std::vector<std::pair<long long, time_type> > > cache_type;


class Cache {
public:
    Cache(long long cacheSize, int associativity, int lineSize) {
        this->time = 1;
        this->cache_hit = 0;
        this->cache_miss = 0;

        this->cacheSize = cacheSize;
        this->associativity = associativity;
        this->lineSize = lineSize;

        this->sets_count = cacheSize / lineSize / associativity;
        this->cache = cache_type(associativity, std::vector<std::pair<long long, int> >(this->sets_count, std::make_pair(-1, 0)));
    }

    void get(long long address) {
        long long cache_line = address / this->lineSize;
        long long set_num = cache_line % this->sets_count;

        size_t tmp = 0;
        bool is_hit = false;
        for (size_t i = 0; i < this->associativity; ++i) {
            if (this->cache[i][set_num].first == cache_line) {
                this->cache_hit += 1;
                this->cache[i][set_num].second = time;
                is_hit = true;
                break;
            }
            if (this->cache[i][set_num].second < this->cache[tmp][set_num].second) {
                tmp = i;
            }
        }
        if (!is_hit) {
            this->cache[tmp][set_num].first = cache_line;
            this->cache[tmp][set_num].second = this->time;
            this->cache_miss += 1;
        }
        this->time += 1;
    }

    int get_cache_miss_count() {
        return this->cache_miss;
    }
    int get_cache_hit_count() {
        return this->cache_hit;
    }
private:
    int cache_hit, cache_miss;

    int associativity;
    long long cacheSize;
    int lineSize;
    long long sets_count;
    int time;

    cache_type cache;
};


int main() {
    int associativity, lineSize;
    size_t n;
    long long cacheSize, address;

    std::cin >> cacheSize >> associativity >> lineSize >> n;
    Cache cache(cacheSize, associativity, lineSize);

    for (size_t i = 0; i < n; ++i) {
        std::cin >> address;
        cache.get(address);
    }

    std::cout << cache.get_cache_hit_count() << " " << cache.get_cache_miss_count();
}