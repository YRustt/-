//
// Created by rustam on 29.10.17.
//

#include <fstream>
#include <sstream>

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <string>

uint64_t BLOCK_SIZE = 1000;
uint64_t MEMORY_SIZE = 50000;
uint64_t COUNT_BLOCK = MEMORY_SIZE / BLOCK_SIZE;


class Heap {
public:
    Heap(std::string filename):
            cur_read_idx(0),
            block_size(BLOCK_SIZE)
    {
        this->fin.open(filename.c_str(), std::ios::in | std::ios::binary);
        this->file_size = fin.tellg() / sizeof(uint64_t);
        this->block = new uint64_t[this->block_size];

        this->read_block();
    }

    uint64_t get_min() {
        if (this->block[0] == UINTMAX_MAX && this->cur_read_idx != this->file_size) {
            this->read_block();
        }

        uint64_t tmp = this->block[0];
        this->block[0] = UINTMAX_MAX;
        push(0);

        return tmp;
    }

    ~Heap() {
        fin.close();
        free(block);
    }
private:
    uint64_t left(uint64_t i) { return 2 * i + 1; }

    uint64_t right(uint64_t i) { return 2 * i + 2; }

    void push(uint64_t idx) {
        uint64_t l = left(idx), r = right(idx);
        uint64_t tmp = idx;
        if (l < this->block_size && this->block[l] < this->block[idx]) {
            tmp = l;
        }
        if (r < this->block_size && this->block[r] < this->block[idx]) {
            tmp = r;
        }
        if (tmp != idx) {
            std::swap(this->block[idx], this->block[tmp]);
            this->push(tmp);
        }
    }

    void read_block() {
        if (this->cur_read_idx + this->block_size > this->file_size) {
            this->block_size = this->file_size - this->cur_read_idx;
        }
        this->cur_read_idx += this->block_size;

        this->fin.read((char*) block, this->block_size);

        int64_t i = (int64_t) (this->block_size - 1) / 2;
        while (i >= 0) {
            push(i);
            --i;
        }
    }

    std::ifstream fin;
    uint64_t * block;
    uint64_t block_size;
    uint64_t file_size;
    uint64_t cur_read_idx;
};


const std::string make_name(uint64_t num_it, uint64_t i) {
    std::ostringstream oss;
    oss << num_it << "_" << i << ".bin";
    return oss.str();
}


uint64_t sort_blocks() {
    std::ifstream fin("input.bin", std::ios::in | std::ios::binary);
    uint64_t N;
    fin.read((char*) &N, sizeof(uint64_t));

    uint64_t *block = new uint64_t[MEMORY_SIZE];
    uint64_t count_files = 0, num_file = 1;

    for (uint64_t i = 0; i < N; i += MEMORY_SIZE) {
        uint64_t read_size = MEMORY_SIZE;
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }

        fin.read((char*) block, sizeof(uint64_t) * read_size);
        std::sort(block, block + read_size);

        std::ofstream fout(make_name((uint64_t) 1, num_file).c_str(), std::ios::out | std::ios::binary);
        fout.write((char*) block, sizeof(uint64_t) * read_size);
        fout.close();

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


uint64_t merge(uint64_t count_block, uint64_t prev_count_files, uint64_t num_it) {
    uint64_t count_files = 0, num_file = 1;
    uint64_t *block = new uint64_t[MEMORY_SIZE];

    for (uint64_t i = 0; i < prev_count_files; i += count_block) {
        std::ofstream fout(make_name(num_it + 1, num_file).c_str(), std::ios::out | std::ios::binary);

        if (i + count_block > count_files) {
            count_block = count_files - i;
        }

        std::vector<Heap*> heaps(count_block);

        for (uint64_t j = 0; j < count_block; ++j) {
            heaps[i] = new Heap(make_name(num_it, i + j).c_str());
        }

        uint64_t cur_idx = 0;
        uint64_t min = UINTMAX_MAX;
        while (true) {
            for (uint64_t j = 0; j < count_block; ++j) {
                min = std::min(min, heaps[i]->get_min());
            }

            if (min == UINTMAX_MAX) {
                break;
            }

            block[cur_idx++] = min;

            if (cur_idx == MEMORY_SIZE) {
                fout.write((char*) block, sizeof(uint64_t) * MEMORY_SIZE);
            }
        }
        if (cur_idx) {
            fout.write((char*) block, sizeof(uint64_t) * cur_idx);
        }


        for (uint64_t j = 0; j < count_block; ++j) {
            free(heaps[i]);
        }

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


int main() {

    uint64_t count_files = sort_blocks(), num_it = 1;
    while (count_files != 1) {
        count_files = merge(COUNT_BLOCK, count_files, num_it);
        num_it++;
    }

    return 0;
}