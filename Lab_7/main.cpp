//
// Created by rustam on 29.10.17.
//

#include <fstream>
#include <sstream>

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <string>
#include <stdio.h>


typedef uint64_t index_type;

index_type BLOCK_SIZE = 200;
index_type MEMORY_SIZE = 10000;
index_type COUNT_BLOCK = MEMORY_SIZE / BLOCK_SIZE;


template<class data_type>
data_type max_value() {
    return;
}

template<>
uint64_t max_value<uint64_t>() {
    return UINT64_MAX;
}


template<class data_type>
class File {
public:
    File(std::string filename):
            cur_idx(0),
            cur_read_idx(0),
            block_size(BLOCK_SIZE)
    {
        this->fin.open(filename.c_str(), std::ios::in | std::ios::binary);

        // get file size
        this->fin.seekg(0, std::ios::end);
        this->file_size = fin.tellg() / sizeof(data_type);
        this->fin.seekg(0);

        this->block = new data_type[this->block_size];
        this->read_block();
    }
    ~File() {
        fin.close();
        free(block);
    }

    data_type get_min() {
        if (this->cur_idx == this->block_size) {
            this->read_block();
            this->cur_idx = 0;
        }

        return this->block[this->cur_idx];
    }

    void pop_min() {
        if (this->block[0] == max_value<data_type>()) {
            return;
        }

        this->cur_idx++;
    }
private:
    void read_block() {
        if (this->cur_read_idx == this->file_size) {
            this->block[0] = max_value<data_type>();
            return;
        }

        if (this->cur_read_idx + this->block_size > this->file_size) {
            this->block_size = this->file_size - this->cur_read_idx;
        }
        this->cur_read_idx += this->block_size;

        this->fin.read((char*) block, sizeof(data_type) * this->block_size);
    }

    std::ifstream fin;
    data_type *block;
    index_type block_size, cur_idx;
    index_type file_size, cur_read_idx;
};


const std::string make_name(index_type num_it, index_type i) {
    std::ostringstream oss;
    oss << num_it << "_" << i << ".bin";
    return oss.str();
}

template<class data_type>
bool comp(const data_type& d1, const data_type& d2) {
    return false;
}

template<>
bool comp<uint64_t>(const uint64_t& d1, const uint64_t& d2) {
    return d1 < d2;
}


template<class data_type>
index_type sort_blocks() {
    std::ifstream fin("input.bin", std::ios::in | std::ios::binary);
    index_type N;
    fin.read((char*) &N, sizeof(index_type));

    data_type *block = new data_type[MEMORY_SIZE];
    index_type count_files = 0, num_file = 1;

    for (index_type i = 0; i < N; i += MEMORY_SIZE) {
        index_type read_size = MEMORY_SIZE;
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }

        fin.read((char*) block, sizeof(data_type) * read_size);
        std::sort(block, block + read_size, comp<data_type>);

        std::ofstream fout(make_name((index_type) 1, num_file).c_str(), std::ios::out | std::ios::binary);
        fout.write((char*) block, sizeof(data_type) * read_size);
        fout.close();

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


template<class data_type>
index_type merge(index_type count_block, index_type prev_count_files, index_type num_it) {
    index_type count_files = 0, num_file = 1;
    data_type *block = new data_type[MEMORY_SIZE];

    for (index_type i = 0; i < prev_count_files; i += count_block) {
        std::ofstream fout(make_name(num_it + 1, num_file).c_str(), std::ios::out | std::ios::binary);

        if (i + count_block > prev_count_files) {
            count_block = prev_count_files - i;
        }

        std::vector<File<data_type>*> files(count_block);

        for (index_type j = 0; j < count_block; ++j) {
            files[j] = new File<data_type>(make_name(num_it, i + j + 1));
        }

        index_type cur_idx = 0;
	
        while (true) {
            index_type min_j = 0;
            data_type min = max_value<data_type>();
            for (index_type j = 0; j < count_block; ++j) {
                if (comp<data_type>(files[j]->get_min(), min)) {
                    min = files[j]->get_min();
                    min_j = j;
                }
            }
            files[min_j]->pop_min();

            if (min == max_value<data_type>()) {
                break;
            }

            block[cur_idx] = min;
	        cur_idx++;

            if (cur_idx == MEMORY_SIZE) {
                fout.write((char*) block, sizeof(data_type) * MEMORY_SIZE);
                cur_idx = 0;
            }
        }
        if (cur_idx) {
            fout.write((char*) block, sizeof(data_type) * cur_idx);
        }

        for (index_type j = 0; j < count_block; ++j) {
            free(files[j]);
        }

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


template<class data_type>
void make_output(index_type num_it) {
    std::ifstream fin(make_name(num_it, (index_type) 1).c_str(), std::ios::in | std::ios::binary);
    std::ofstream fout("output.bin", std::ios::out | std::ios::binary);

    fin.seekg(0, std::ios::end);
    uint64_t N = fin.tellg() / sizeof(data_type);
    fin.seekg(0);

    fout.write((char*) &N, sizeof(index_type));
    
    index_type read_size = MEMORY_SIZE;
    data_type *block = new data_type[MEMORY_SIZE];

    for (index_type i = 0; i < N; i += MEMORY_SIZE) {
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }        
        fin.read((char*) block, sizeof(data_type) * read_size);
        fout.write((char*) block, sizeof(data_type) * read_size);
    }

    free(block);
    fin.close();
    fout.close();
}


int main() {
    index_type count_files = sort_blocks<uint64_t>(), num_it = 1;
    while (count_files != 1) {
        count_files = merge<uint64_t>(COUNT_BLOCK, count_files, num_it);
        num_it++;
    }
    make_output<uint64_t>(num_it);
    return 0;
}
