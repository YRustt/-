//
// Created by yrustt on 26.11.17.
//

#include <fstream>
#include <sstream>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <utility>


uint32_t BLOCK_SIZE = 200;
uint32_t MEMORY_SIZE = 10000;
uint32_t COUNT_BLOCK = MEMORY_SIZE / BLOCK_SIZE;


struct Pair {
    uint32_t i;
    uint32_t r_i;
    uint32_t next_i;
    uint32_t r_next_i;
};

struct Triple {
    uint32_t i;
    uint32_t r_i;
    uint32_t next_i;
    uint32_t r_next_i;
    uint32_t next_next_i;
    uint32_t r_next_next_i;
};


template<class data_type>
data_type max_value() {
    return;
}

template<>
Pair max_value<Pair>() {
    static Pair n;
    n.i = UINT32_MAX;
    n.next_i = UINT32_MAX;
    return n;
}

template<>
Triple max_value<Triple>() {
    static Triple n;
    n.i = UINT32_MAX;
    n.next_i = UINT32_MAX;
    n.next_next_i = UINT32_MAX;
    return n;
}

template<class data_type>
bool comp(const data_type& d1, const data_type& d2) {
    return false;
}

template<>
bool comp<Pair>(const Pair& d1, const Pair& d2) {
    return d1.i < d2.i;
}

template<>
bool comp<Triple>(const Triple& d1, const Triple& d2) {
    return d1.next_i < d2.next_i;
}


template<class data_type>
bool equal(const data_type& d1, const data_type& d2) {
    return false;
}

template<>
bool equal<Pair>(const Pair& d1, const Pair& d2) {
    return (d1.i == d2.i) && (d1.next_i == d2.next_i);
}

template<>
bool equal<Triple>(const Triple& d1, const Triple& d2) {
    return (d1.i == d2.i) && (d1.next_i == d2.next_i) && (d1.next_next_i == d2.next_next_i);
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
        free(this->block);
    }

    data_type get_min() {
        if (this->cur_idx == this->block_size) {
            this->read_block();
            this->cur_idx = 0;
        }

        return this->block[this->cur_idx];
    }

    void pop_min() {
        if (equal<data_type>(this->block[0], max_value<data_type>())) {
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
    uint32_t block_size, cur_idx;
    uint32_t file_size, cur_read_idx;
};


const std::string make_name(uint32_t num_it, uint32_t i) {
    std::ostringstream oss;
    oss << num_it << "_" << i << ".bin";
    return oss.str();
}


template<class data_type>
uint32_t sort_blocks(std::string filename) {
    std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    uint32_t N;
    fin.read((char*) &N, sizeof(uint32_t));

    data_type *block = new data_type[MEMORY_SIZE];
    uint32_t count_files = 0, num_file = 1;

    for (uint32_t i = 0; i < N; i += MEMORY_SIZE) {
        uint32_t read_size = MEMORY_SIZE;
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }

        fin.read((char*) block, sizeof(data_type) * read_size);
        std::sort(block, block + read_size, comp<data_type>);

        std::ofstream fout(make_name((uint32_t) 1, num_file).c_str(), std::ios::out | std::ios::binary);
        fout.write((char*) block, sizeof(data_type) * read_size);
        fout.close();

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


template<class data_type>
uint32_t merge(uint32_t count_block, uint32_t prev_count_files, uint32_t num_it) {
    uint32_t count_files = 0, num_file = 1;
    data_type *block = new data_type[MEMORY_SIZE];

    for (uint32_t i = 0; i < prev_count_files; i += count_block) {
        std::ofstream fout(make_name(num_it + 1, num_file).c_str(), std::ios::out | std::ios::binary);

        if (i + count_block > prev_count_files) {
            count_block = prev_count_files - i;
        }

        std::vector<File<data_type>*> files(count_block);

        for (uint32_t j = 0; j < count_block; ++j) {
            files[j] = new File<data_type>(make_name(num_it, i + j + 1));
        }

        uint32_t cur_idx = 0;

        while (true) {
            uint32_t min_j = 0;
            data_type min = max_value<data_type>();
            for (uint32_t j = 0; j < count_block; ++j) {
                if (comp<data_type>(files[j]->get_min(), min)) {
                    min = files[j]->get_min();
                    min_j = j;
                }
            }
            files[min_j]->pop_min();

            if (equal<data_type>(min, max_value<data_type>())) {
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

        for (uint32_t j = 0; j < count_block; ++j) {
            delete files[j];
        }

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


template<class data_type>
void make_output(std::string filename, uint32_t num_it, bool is_write_n=true) {
    std::ifstream fin(make_name(num_it, (uint32_t) 1).c_str(), std::ios::in | std::ios::binary);
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    fin.seekg(0, std::ios::end);
    uint32_t N = fin.tellg() / sizeof(data_type);
    fin.seekg(0);

    if (is_write_n) {
        fout.write((char *) &N, sizeof(uint32_t));
    }

    uint32_t read_size = MEMORY_SIZE;
    data_type *block = new data_type[MEMORY_SIZE];

    for (uint32_t i = 0; i < N; i += MEMORY_SIZE) {
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

template<class data_type>
void merge_sort(const std::string& in_filename, const std::string& out_filename, bool is_write_n=true) {
    uint32_t count_files = sort_blocks<data_type>(in_filename), num_it = 1;
    while (count_files != 1) {
        count_files = merge<data_type>(COUNT_BLOCK, count_files, num_it);
        num_it++;
    }
    make_output<data_type>(out_filename, num_it, is_write_n);
}


void reverse_pair() {
    std::ifstream fin("input.bin", std::ios::in | std::ios::binary);
    std::ofstream fout("reverse_input.bin", std::ios::out | std::ios::binary);

    uint32_t N;
    fin.read((char*) &N, sizeof(uint32_t));
    fout.write((char*) &N, sizeof(uint32_t));

    uint32_t read_size = MEMORY_SIZE;
    Pair *block = new Pair[MEMORY_SIZE];

    for (uint32_t i = 0; i < N; i += MEMORY_SIZE) {
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }
        fin.read((char*) block, sizeof(Pair) * read_size);

        for (uint32_t j = 0; j < read_size; ++j) {
            std::swap(block[j].i, block[j].next_i);
            std::swap(block[j].r_i, block[j].r_next_i);
        }

        fout.write((char*) block, sizeof(Pair) * read_size);
    }

    free(block);
    fin.close();
    fout.close();
}

void join(const std::string& filename1, const std::string& filename2, const std::string& out_filename) {
    std::ifstream fin1(filename1.c_str(), std::ios::in | std::ios::binary);
    std::ifstream fin2(filename2.c_str(), std::ios::in | std::ios::binary);
    std::ofstream fout(out_filename.c_str(), std::ios::out | std::ios::binary);

    uint32_t N;
    fin1.read((char*) &N, sizeof(uint32_t));
    fin2.seekg(sizeof(uint32_t));

    fout.write((char*) &N, sizeof(uint32_t));

    uint32_t TMP_MEMORY_SIZE = MEMORY_SIZE / 5;
    uint32_t read_size = TMP_MEMORY_SIZE;
    Pair *block1 = new Pair[TMP_MEMORY_SIZE];
    Pair *block2 = new Pair[TMP_MEMORY_SIZE];
    Triple *out_block = new Triple[TMP_MEMORY_SIZE];

    for (uint32_t i = 0; i < N; i += TMP_MEMORY_SIZE) {
        if (i + TMP_MEMORY_SIZE > N) {
            read_size = N - i;
        }
        fin1.read((char*) block1, sizeof(Pair) * read_size);
        fin2.read((char*) block2, sizeof(Pair) * read_size);

        for (uint32_t j = 0; j < read_size; ++j) {
            out_block[j].i = block2[j].next_i;
            out_block[j].next_i = block1[j].i;
            out_block[j].next_next_i = block1[j].next_i;
        }

        fout.write((char*) out_block, sizeof(Triple) * read_size);
    }

    fin1.close();
    fin2.close();
    fout.close();
    free(block1);
    free(block2);
    free(out_block);
}


void join(const std::string& in_filename, const std::string& out_filename) {
    reverse_pair();

    // sort pairs
    merge_sort<Pair>(in_filename.c_str(), "tmp1.bin");

    // sort reverse pairs
    merge_sort<Pair>("reverse_input.bin", "tmp2.bin");

    // join
    join("tmp1.bin", "tmp2.bin", "tmp.bin");

    // sort result
    merge_sort<Triple>("tmp.bin", out_filename.c_str(), false);
}



void init(const std::string& in_filename, const std::string& out_filename) {
    std::ifstream in(in_filename.c_str(), std::ios::in | std::ios::binary);
    std::ofstream out(out_filename.c_str(), std::ios::out | std::ios::binary);

    uint32_t n, read_size;
    std::pair<uint32_t, uint32_t> in_block[BLOCK_SIZE];
    Pair out_block[BLOCK_SIZE];

    in.read((char*) &n, sizeof(uint32_t));
    out.write((char*) &n, sizeof(uint32_t));

    for (uint32_t i = 0; i < n; i += BLOCK_SIZE) {
        read_size = BLOCK_SIZE;
        if (i + BLOCK_SIZE >= n) {
            read_size = n - i;
        }

        in.read((char*) in_block, read_size);
        for (uint32_t j = 0; j < read_size; ++j) {
            out_block[j].i = in_block[j].first;
            out_block[j].next_i = in_block[j].second;
            out_block[j].r_i = 1;
            out_block[j].r_next_i = 1;
        }

        out.write((char*) out_block, read_size);
    }

    in.close();
    out.close();
}


void gen_d(const std::string& pairs, const std::string& d) {
    std::ifstream in(pairs.c_str(), std::ios::in | std::ios::binary);
    std::ofstream out(d.c_str(), std::ios::out | std::ios::binary);



    in.close();
    out.close();
}

uint32_t forward_iter(const std::string& pairs, const std::string& triples, const std::string& ds, const std::string& dels) {
    gen_d(pairs, ds);
    join(pairs, triples);


    return 0;
}


int main() {
    init("input.bin", "pairs.bin");

    uint32_t i = 1, n;
    while (true) {
        std::ostringstream oss;
        oss << "del_" << i << ".bin";
        n = forward_iter("pairs.bin", "triples.bin", "d.bin", oss.str());

        if (n < MEMORY_SIZE) {
            break;
        } else {
            i++;
        }
    }

    return 0;
}