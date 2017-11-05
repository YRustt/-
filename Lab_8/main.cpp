//
// Created by yrustt on 5.11.17.
//

#include <fstream>
#include <sstream>

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <string>
#include <stdio.h>


typedef uint32_t index_type;

index_type BLOCK_SIZE = 200;
index_type MEMORY_SIZE = 10000;
index_type COUNT_BLOCK = MEMORY_SIZE / BLOCK_SIZE;

struct Node {
    uint32_t i;
    uint32_t next_i;
};


struct Node3 {
    uint32_t i;
    uint32_t next_i;
    uint32_t next_next_i;
};


template<class data_type>
data_type max_value() {
    return;
}

template<>
Node max_value<Node>() {
    static Node n;
    n.i = UINT32_MAX;
    n.next_i = UINT32_MAX;
    return n;
}

template<>
Node3 max_value<Node3>() {
    static Node3 n;
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
bool comp<Node>(const Node& d1, const Node& d2) {
    return d1.i < d2.i;
}

template<>
bool comp<Node3>(const Node3& d1, const Node3& d2) {
    return d1.i < d2.i;
}


template<class data_type>
bool equal(const data_type& d1, const data_type& d2) {
    return false;
}

template<>
bool equal<Node>(const Node& d1, const Node& d2) {
    return (d1.i == d2.i) && (d1.next_i == d2.next_i);
}

template<>
bool equal<Node3>(const Node3& d1, const Node3& d2) {
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
    index_type block_size, cur_idx;
    index_type file_size, cur_read_idx;
};


const std::string make_name(index_type num_it, index_type i) {
    std::ostringstream oss;
    oss << num_it << "_" << i << ".bin";
    return oss.str();
}


template<class data_type>
index_type sort_blocks(std::string filename) {
    std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
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

        for (index_type j = 0; j < count_block; ++j) {
            delete files[j];
        }

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


template<class data_type>
void make_output(std::string filename, index_type num_it, bool is_write_n=true) {
    std::ifstream fin(make_name(num_it, (index_type) 1).c_str(), std::ios::in | std::ios::binary);
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    fin.seekg(0, std::ios::end);
    index_type N = fin.tellg() / sizeof(data_type);
    fin.seekg(0);

    if (is_write_n) {
        fout.write((char *) &N, sizeof(index_type));
    }

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

template<class data_type>
void merge_sort(const std::string& in_filename, const std::string& out_filename, bool is_write_n=true) {
    index_type count_files = sort_blocks<data_type>(in_filename), num_it = 1;
    while (count_files != 1) {
        count_files = merge<data_type>(COUNT_BLOCK, count_files, num_it);
        num_it++;
    }
    make_output<data_type>(out_filename, num_it, is_write_n);
}


void reverse_pair() {
    std::ifstream fin("input.bin", std::ios::in | std::ios::binary);
    std::ofstream fout("reverse_input.bin", std::ios::out | std::ios::binary);

    index_type N;
    fin.read((char*) &N, sizeof(index_type));
    fout.write((char*) &N, sizeof(index_type));

    index_type read_size = MEMORY_SIZE;
    Node *block = new Node[MEMORY_SIZE];

    for (index_type i = 0; i < N; i += MEMORY_SIZE) {
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }
        fin.read((char*) block, sizeof(Node) * read_size);

        for (index_type j = 0; j < read_size; ++j) {
            std::swap(block[j].i, block[j].next_i);
        }

        fout.write((char*) block, sizeof(Node) * read_size);
    }

    free(block);
    fin.close();
    fout.close();
}

void join(const std::string& filename1, const std::string& filename2, const std::string& out_filename) {
    std::ifstream fin1(filename1.c_str(), std::ios::in | std::ios::binary);
    std::ifstream fin2(filename2.c_str(), std::ios::in | std::ios::binary);
    std::ofstream fout(out_filename.c_str(), std::ios::out | std::ios::binary);

    index_type N;
    fin1.read((char*) &N, sizeof(index_type));
    fin2.seekg(sizeof(index_type));

    fout.write((char*) &N, sizeof(index_type));

    index_type TMP_MEMORY_SIZE = MEMORY_SIZE / 5;
    index_type read_size = TMP_MEMORY_SIZE;
    Node *block1 = new Node[TMP_MEMORY_SIZE];
    Node *block2 = new Node[TMP_MEMORY_SIZE];
    Node3 *out_block = new Node3[TMP_MEMORY_SIZE];

    for (index_type i = 0; i < N; i += TMP_MEMORY_SIZE) {
        if (i + TMP_MEMORY_SIZE > N) {
            read_size = N - i;
        }
        fin1.read((char*) block1, sizeof(Node) * read_size);
        fin2.read((char*) block2, sizeof(Node) * read_size);

        for (index_type j = 0; j < read_size; ++j) {
            out_block[j].i = block2[j].next_i;
            out_block[j].next_i = block1[j].i;
            out_block[j].next_next_i = block1[j].next_i;
        }

        fout.write((char*) out_block, sizeof(Node3) * read_size);
    }

    fin1.close();
    fin2.close();
    fout.close();
    free(block1);
    free(block2);
    free(out_block);
}


int main() {
    reverse_pair();

    // sort pairs
    merge_sort<Node>("input.bin", "tmp1.bin");

    // sort reverse pairs
    merge_sort<Node>("reverse_input.bin", "tmp2.bin");

    // join
    join("tmp1.bin", "tmp2.bin", "tmp.bin");

    // sort result
    merge_sort<Node3>("tmp.bin", "output.bin", false);
    return 0;
}
