//
// Created by yrustt on 26.11.17.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <cstdlib>


typedef uint32_t index_type;

index_type MAX_VALUE = UINT32_MAX;

index_type BLOCK_SIZE = 1000;
index_type MEMORY_SIZE = 10000;
index_type COUNT_BLOCK = MEMORY_SIZE / BLOCK_SIZE;

template<index_type size>
class Element {
public:
    index_type& operator[](index_type i) {
        return this->array[i];
    }

    index_type operator[](index_type i) const {
        return this->array[i];
    }

    void operator=(const Element<size>& obj) {
        for (index_type i = 0; i < size; ++i) {
            this->array[i] = obj[i];
        }
    }

    index_type array[size];
};


template<index_type size, index_type order_by>
bool comp(const Element<size>& obj1, const Element<size>& obj2) {
    return obj1[order_by] < obj2[order_by];
}


template<index_type size>
class File {
public:
    File(const std::string& filename): cur_idx(0), cur_read_idx(0), block_size(BLOCK_SIZE) {
        this->fin.open(filename.c_str(), std::ios::in | std::ios::binary);

        // get file size
        this->fin.seekg(0, std::ios::end);
        this->file_size = fin.tellg() / sizeof(Element<size>);
        this->fin.seekg(0);

        this->block = new Element<size>[this->block_size];
        this->read_block();
    }
    ~File() {
        fin.close();
        delete this->block;
    }

    Element<size> get_min() {
        if (this->cur_idx == this->block_size) {
            this->read_block();
            this->cur_idx = 0;
        }

        return this->block[this->cur_idx];
    }

    void pop_min() {
        if (this->block[0][0] == MAX_VALUE) {
            return;
        }

        this->cur_idx++;
    }
private:
    void read_block() {
        if (this->cur_read_idx == this->file_size) {
            this->block[0][0] = MAX_VALUE;
            return;
        }

        if (this->cur_read_idx + this->block_size > this->file_size) {
            this->block_size = this->file_size - this->cur_read_idx;
        }
        this->cur_read_idx += this->block_size;

        this->fin.read((char*) block, sizeof(Element<size>) * this->block_size);
    }

    std::ifstream fin;
    Element<size> *block;
    index_type block_size, cur_idx;
    index_type file_size, cur_read_idx;
};


template<index_type size>
class WriteFile {
public:
    WriteFile(const std::string& filename): cur_idx(0), file_size(0) {
        this->out.open(filename.c_str(), std::ios::out | std::ios::binary);
        this->out.seekp(sizeof(index_type));
        this->block = new Element<size>[BLOCK_SIZE];
    }
    ~WriteFile() {
        this->flush();
        this->write_size();
        this->out.close();
        delete this->block;
    }
    void write(const Element<size>& obj) {
        if (this->cur_idx < BLOCK_SIZE) {
            this->block[this->cur_idx] = obj;
            this->cur_idx++;
        } else {
            this->flush();
        }
    }
    void write_size() {
        if (this->file_size != 0) {
            this->out.seekp(0);
            this->out.write((char *) &this->file_size, sizeof(index_type));
        }
    }
    void flush() {
        if (this->cur_idx != 0) {
            out.write((char *) this->block, sizeof(Element<size>) * this->cur_idx);
            this->file_size += this->cur_idx;
        }
    }
private:
    std::ofstream out;
    Element<size> *block;
    index_type cur_idx, file_size;
};


const std::string make_name(index_type num_it, index_type i) {
    std::ostringstream oss;
    oss << num_it << "_" << i << ".bin";
    return oss.str();
}


template<index_type size, index_type order_by>
index_type sort_blocks(const std::string& filename) {
    std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    index_type N;
    fin.read((char*) &N, sizeof(index_type));

    Element<size> *block = new Element<size>[MEMORY_SIZE];
    index_type count_files = 0, num_file = 1;

    for (index_type i = 0; i < N; i += MEMORY_SIZE) {
        index_type read_size = MEMORY_SIZE;
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }

        fin.read((char*) block, sizeof(Element<size>) * read_size);
        std::sort(block, block + read_size, comp<size, order_by>);

        std::ofstream fout(make_name((index_type) 1, num_file).c_str(), std::ios::out | std::ios::binary);
        fout.write((char*) block, sizeof(Element<size>) * read_size);
        fout.close();

        count_files++;
        num_file++;
    }

    free(block);
    return count_files;
}


template<index_type size>
index_type merge(index_type count_block, index_type prev_count_files, index_type num_it) {
    index_type count_files = 0, num_file = 1;
    Element<size> *block = new Element<size>[MEMORY_SIZE];

    for (index_type i = 0; i < prev_count_files; i += count_block) {
        std::ofstream fout(make_name(num_it + 1, num_file).c_str(), std::ios::out | std::ios::binary);

        if (i + count_block > prev_count_files) {
            count_block = prev_count_files - i;
        }

        std::vector<File<size>*> files(count_block);

        for (index_type j = 0; j < count_block; ++j) {
            files[j] = new File<size>(make_name(num_it, i + j + 1));
        }

        index_type cur_idx = 0;

        while (true) {
            index_type min_j = 0, min = MAX_VALUE;
            Element<size> min_el;
            for (index_type j = 0; j < count_block; ++j) {
                if (files[j]->get_min()[0] < min) {
                    min = files[j]->get_min()[0];
                    min_el = files[j]->get_min();
                    min_j = j;
                }
            }
            files[min_j]->pop_min();

            if (min == MAX_VALUE) {
                break;
            }

            block[cur_idx] = min_el;
            cur_idx++;

            if (cur_idx == MEMORY_SIZE) {
                fout.write((char*) block, sizeof(Element<size>) * MEMORY_SIZE);
                cur_idx = 0;
            }
        }
        if (cur_idx) {
            fout.write((char*) block, sizeof(Element<size>) * cur_idx);
        }

        for (index_type j = 0; j < count_block; ++j) {
            delete files[j];
        }

        count_files++;
        num_file++;
    }

    delete block;
    return count_files;
}


template<index_type size>
void make_output(const std::string& filename, index_type num_it, bool is_write_n=true) {
    std::ifstream fin(make_name(num_it, (index_type) 1).c_str(), std::ios::in | std::ios::binary);
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    fin.seekg(0, std::ios::end);
    index_type N = fin.tellg() / sizeof(Element<size>);
    fin.seekg(0);

    if (is_write_n) {
        fout.write((char *) &N, sizeof(index_type));
    }

    index_type read_size = MEMORY_SIZE;
    Element<size> *block = new Element<size>[MEMORY_SIZE];

    for (index_type i = 0; i < N; i += MEMORY_SIZE) {
        if (i + MEMORY_SIZE > N) {
            read_size = N - i;
        }
        fin.read((char*) block, sizeof(Element<size>) * read_size);
        fout.write((char*) block, sizeof(Element<size>) * read_size);
    }

    delete block;
    fin.close();
    fout.close();
}

template<index_type size, index_type order_by>
void merge_sort(const std::string& in_filename, const std::string& out_filename, bool is_write_n=true) {
    index_type count_files = sort_blocks<size, order_by>(in_filename), num_it = 1;
    while (count_files != 1) {
        count_files = merge<size>(COUNT_BLOCK, count_files, num_it);
        num_it++;
    }
    make_output<size>(out_filename, num_it, is_write_n);
}


void join(const std::string& filename1, const std::string& filename2, const std::string& out_filename) {
    std::ifstream fin1(filename1.c_str(), std::ios::in | std::ios::binary);
    std::ifstream fin2(filename2.c_str(), std::ios::in | std::ios::binary);
    std::ofstream fout(out_filename.c_str(), std::ios::out | std::ios::binary);

    index_type N;
    fin1.read((char*) &N, sizeof(index_type));
    fin2.seekg(sizeof(index_type));

    fout.write((char*) &N, sizeof(index_type));

    index_type read_size = BLOCK_SIZE;
    Element<4> *block1 = new Element<4>[BLOCK_SIZE];
    Element<4> *block2 = new Element<4>[BLOCK_SIZE];
    Element<7> *out_block = new Element<7>[BLOCK_SIZE];

    for (index_type i = 0; i < N; i += BLOCK_SIZE) {
        if (i + BLOCK_SIZE > N) {
            read_size = N - i;
        }
        fin1.read((char*) block1, sizeof(Element<4>) * read_size);
        fin2.read((char*) block2, sizeof(Element<4>) * read_size);

        for (index_type j = 0; j < read_size; ++j) {
            out_block[j][0] = block2[j][0];
            out_block[j][1] = block1[j][0];
            out_block[j][2] = block1[j][1];
            out_block[j][3] = block2[j][2];
            out_block[j][4] = block1[j][2];
            out_block[j][5] = block2[j][3];
            out_block[j][6] = block1[j][3];
        }

        fout.write((char*) out_block, sizeof(Element<7>) * read_size);
    }

    fin1.close();
    fin2.close();
    fout.close();
    delete block1;
    delete block2;
    delete out_block;
}


void make_join(const std::string& in_filename, const std::string& out_filename) {
    // sort pairs
    merge_sort<4, 0>(in_filename, "tmp1.bin");

    // sort reverse pairs
    merge_sort<4, 1>(in_filename, "tmp2.bin");

    // join
    join("tmp1.bin", "tmp2.bin", "tmp.bin");

    // sort result
    merge_sort<7, 0>("tmp.bin", out_filename, true);
}



index_type init(const std::string& in_filename, const std::string& out_filename) {
    std::ifstream in(in_filename.c_str(), std::ios::in | std::ios::binary);
    std::ofstream out(out_filename.c_str(), std::ios::out | std::ios::binary);

    index_type n, read_size;
    Element<2> in_block[BLOCK_SIZE];
    Element<4> out_block[BLOCK_SIZE];

    in.read((char*) &n, sizeof(index_type));
    out.write((char*) &n, sizeof(index_type));

    for (index_type i = 0; i < n; i += BLOCK_SIZE) {
        read_size = BLOCK_SIZE;
        if (i + BLOCK_SIZE >= n) {
            read_size = n - i;
        }

        in.read((char*) in_block, sizeof(Element<2>) * read_size);
        for (index_type j = 0; j < read_size; ++j) {
            out_block[j][0] = in_block[j][0];
            out_block[j][1] = in_block[j][1];
            out_block[j][2] = 1;
            out_block[j][3] = rand() % 2;
        }

        out.write((char*) out_block, sizeof(Element<4>) * read_size);
    }

    in.close();
    out.close();

    return n;
}


index_type forward_iter(const std::string& deleted) {
    make_join("pairs.bin", "triples.bin");
    std::ifstream triples("triples.bin", std::ios::in | std::ios::binary);

    WriteFile<4> pairs("pairs.bin");
    WriteFile<3> d(deleted);
    Element<4> el_pairs;
    Element<3> el_d;

    Element<7> *block = new Element<7>[BLOCK_SIZE];
    index_type cur_n = 0, n, read_size;

    triples.read((char*) &n, sizeof(index_type));

    for (index_type i = 0; i < n; i += BLOCK_SIZE) {
        read_size = BLOCK_SIZE;
        if (i + BLOCK_SIZE >= n) {
            read_size = n - i;
        }
        triples.read((char*) block, sizeof(Element<7>) * read_size);

        for (index_type j = 0; j < read_size; ++j) {
            if (block[j][5] == 0) {
                if (block[j][6] == 0) {
                    cur_n++;
                    el_pairs[0] = block[j][0];
                    el_pairs[1] = block[j][1];
                    el_pairs[2] = block[j][3];
                    el_pairs[3] = rand() % 2;
                    pairs.write(el_pairs);
                } else {
                    el_d[0] = block[j][0];
                    el_d[1] = block[j][1];
                    el_d[2] = block[j][4];
                    d.write(el_d);

                    cur_n++;
                    el_pairs[0] = block[j][0];
                    el_pairs[1] = block[j][2];
                    el_pairs[2] = block[j][3] + block[j][4];
                    el_pairs[3] = rand() % 2;
                    pairs.write(el_pairs);
                }
            } else {
                if (block[j][6] == 1) {
                    cur_n++;
                    el_pairs[0] = block[j][0];
                    el_pairs[1] = block[j][1];
                    el_pairs[2] = block[j][3];
                    el_pairs[3] = rand() % 2;
                    pairs.write(el_pairs);
                }
            }
        }
    }

    delete block;
    triples.close();

    return cur_n;
}

void expand() {
    std::ifstream in("pairs.bin", std::ios::in | std::ios::binary);
}

void make_list_ranking(const std::string& in_filename, const std::string& out_filename) {
    index_type n = init(in_filename.c_str(), "pairs.bin");

    index_type i = 0;
    while (n > MEMORY_SIZE) {
        std::ostringstream oss;
        oss << "del_" << i << ".bin";

        n = forward_iter(oss.str());

        std::cout << n << std::endl;

        if (n >= MEMORY_SIZE) {
            i++;
        }
    }


}


int main() {
    srand(time(0));

    make_list_ranking("input2.bin", "output.bin");

    return 0;
}