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
#include <random>
#include <time.h>
#include <stdio.h>


typedef uint64_t index_type;

index_type MAX_VALUE = UINT64_MAX;

index_type BLOCK_SIZE = 80;
index_type MEMORY_SIZE = 400;
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
    File(const std::string& filename, bool has_write_n=false) {
        this->cur_idx = 0;
        this->cur_read_idx = 0;
        this->block_size = BLOCK_SIZE;
        this->filename = filename;
        this->has_write_n = has_write_n;

        this->fin.open(this->filename.c_str(), std::ios::in | std::ios::binary);

        if (!this->has_write_n) {
            // get file size
            this->fin.seekg(0, std::ios::end);
            this->file_size = fin.tellg() / sizeof(Element<size>);
            this->fin.seekg(0);
        } else {
            this->fin.read((char*) &this->file_size, sizeof(index_type));
            this->file_size--;
        }

        this->block = new Element<size>[this->block_size];
        this->read_block();
    }
    ~File() {
        fin.close();
        delete this->block;
    }

    void reopen() {
        this->fin.close();
        this->fin.open(this->filename.c_str(), std::ios::in | std::ios::binary);
        if (this->has_write_n) {
            this->fin.seekg(sizeof(index_type));
        }

        delete this->block;
        this->block = new Element<size>[this->block_size];
        this->cur_idx = 0;
        this->cur_read_idx = 0;
        this->read_block();
    }

    Element<size> read() {
        if (this->cur_idx == this->block_size) {
            this->read_block();
            this->cur_idx = 0;
        }

        return this->block[this->cur_idx];
    }

    void next() {
        if (this->block[0][0] == MAX_VALUE) {
            return;
        }

        this->cur_idx++;
    }
private:
    void read_block() {
        if (this->cur_read_idx == this->file_size) {
            for (index_type i = 0; i < size; ++i) {
                this->block[0][i] = MAX_VALUE;
            }
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
    std::string filename;
    bool has_write_n;
};


template<index_type size>
class WriteFile {
public:
    WriteFile(const std::string& filename, bool is_write_n=true): cur_idx(0), file_size(0), is_write_n(is_write_n) {
        this->out.open(filename.c_str(), std::ios::out | std::ios::binary);
        if (this->is_write_n) {
            this->out.seekp(sizeof(index_type));
        }
        this->block = new Element<size>[MEMORY_SIZE];
    }
    ~WriteFile() {
        this->flush();
        if (this->is_write_n) {
            this->write_size();
        }
        this->out.close();
        delete this->block;
    }
    void close() {
        this->flush();
        if (this->is_write_n) {
            this->write_size();
        }
        this->out.close();
    }

    void write(const Element<size>& obj) {
        if (this->cur_idx >= MEMORY_SIZE) {
            this->flush();
        }

        this->block[this->cur_idx] = obj;
        this->cur_idx++;
    }
private:
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
            this->cur_idx = 0;
        }
    }

    std::ofstream out;
    Element<size> *block;
    index_type cur_idx, file_size;
    bool is_write_n;
};


template<index_type size>
class Stack {
public:
    Stack() {
        this->cur_idx = 0;
        this->cur_file_idx = 0;
        this->left_border = BLOCK_SIZE / 2;
        this->right_border = 3 / 2 * BLOCK_SIZE;
        this->block = new Element<size>[2 * BLOCK_SIZE];
    }
    ~Stack() {
        delete this->block;
    }
    Element<size>& top() {
        if ((this->cur_idx == 0) && this->cur_file_idx) {
            this->read();
        }

//        std::cout << this->block[this->cur_idx - 1][0] << std::endl;

//        std::cout << this->cur_idx << std::endl;

        return this->block[this->cur_idx - 1];
    }
    void pop() {
        if ((this->cur_idx < this->left_border) && this->cur_file_idx) {
            this->read();
        }
        if (cur_idx == 0) {
            return;
        }
        this->cur_idx--;

//        for (index_type i = 0; i < this->cur_idx; ++i) {
//            std::cout << this->block[i][0] << " ";
//        }
//        std::cout << std::endl;
    }
    void push(const Element<size>& obj) {
        if (this->cur_idx > this->right_border) {
//            std::cout << "write " << this->cur_file_idx << std::endl;
            this->write();
        }
        this->block[this->cur_idx] = obj;
        this->cur_idx++;

//        for (index_type i = 0; i < this->cur_idx; ++i) {
//            std::cout << this->block[i][0] << " ";
//        }
//        std::cout << std::endl;
    }

private:
    void write() {
        std::ostringstream oss;
        oss << "stack_" << this->cur_file_idx++ << ".bin";
        std::ofstream out(oss.str().c_str(), std::ios::out | std::ios::binary);
        out.write((char*) this->block, sizeof(Element<size>) * BLOCK_SIZE);
        out.close();

        for (index_type i = BLOCK_SIZE; i < this->cur_idx; ++i) {
            this->block[i - BLOCK_SIZE] = this->block[i];
        }
        this->cur_idx -= BLOCK_SIZE;
    }

    void read() {
        this->cur_file_idx--;
        std::ostringstream oss;
        oss << "stack_" << this->cur_file_idx << ".bin";
        std::ifstream in(oss.str().c_str(), std::ios::in | std::ios::binary);

        for (index_type i = 0; i < this->cur_idx; ++i) {
            this->block[i + BLOCK_SIZE] = this->block[i];
        }
        in.read((char*) this->block, sizeof(Element<size>) * BLOCK_SIZE);
        this->cur_idx += BLOCK_SIZE;

//        std::cout << "read" << std::endl;
    }

    Element<size> *block;
    index_type left_border, right_border;
    index_type cur_idx;
    index_type cur_file_idx;
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

    delete block;
    return count_files;
}


template<index_type size, index_type order_by>
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
                if (files[j]->read()[order_by] < min) {
                    min = files[j]->read()[order_by];
                    min_el = files[j]->read();
                    min_j = j;
                }
            }
            files[min_j]->next();

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
    for (index_type i = 0; i < prev_count_files; ++i) {
        remove(make_name(num_it, i + 1).c_str());
    }
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
        count_files = merge<size, order_by>(COUNT_BLOCK, count_files, num_it);
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

    index_type TMP_MEMORY_SIZE = MEMORY_SIZE / 2;
    index_type read_size = TMP_MEMORY_SIZE;
    Element<4> *block1 = new Element<4>[TMP_MEMORY_SIZE];
    Element<4> *block2 = new Element<4>[TMP_MEMORY_SIZE];
    Element<7> *out_block = new Element<7>[TMP_MEMORY_SIZE];

    for (index_type i = 0; i < N; i += TMP_MEMORY_SIZE) {
        if (i + TMP_MEMORY_SIZE > N) {
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
    merge_sort<4, 0>(in_filename, "tmp1.bin", true);

    // sort reverse pairs
    merge_sort<4, 1>(in_filename, "tmp2.bin", true);

    // join
    join("tmp1.bin", "tmp2.bin", "tmp.bin");

    // sort result
    merge_sort<7, 0>("tmp.bin", out_filename, true);
}



index_type init(const std::string& in_filename, const std::string& out_filename) {
    srand(time(NULL));

    std::ifstream in(in_filename.c_str(), std::ios::in | std::ios::binary);
    std::ofstream out(out_filename.c_str(), std::ios::out | std::ios::binary);

    index_type n, read_size;
    Element<2> in_block[MEMORY_SIZE];
    Element<4> out_block[MEMORY_SIZE];

    in.read((char*) &n, sizeof(index_type));
    out.write((char*) &n, sizeof(index_type));

    for (index_type i = 0; i < n; i += MEMORY_SIZE) {
        read_size = MEMORY_SIZE;
        if (i + MEMORY_SIZE > n) {
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
    srand(time(NULL));

    make_join("pairs.bin", "triples.bin");
    std::ifstream triples("triples.bin", std::ios::in | std::ios::binary);

    WriteFile<4> pairs("pairs.bin");
    WriteFile<3> d(deleted, false);
    Element<4> el_pairs;
    Element<3> el_d;

    Element<7> *block = new Element<7>[MEMORY_SIZE];
    index_type cur_n = 0, n, read_size;

    triples.read((char*) &n, sizeof(index_type));

    for (index_type i = 0; i < n; i += MEMORY_SIZE) {
        read_size = MEMORY_SIZE;
        if (i + MEMORY_SIZE >= n) {
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
                    el_d[2] = block[j][3];
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
                    el_pairs[0] = block[j][1];
                    el_pairs[1] = block[j][2];
                    el_pairs[2] = block[j][4];
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

void expand(index_type idx) {
    std::ifstream in("pairs.bin", std::ios::in | std::ios::binary);

    index_type n;
    in.read((char*) &n, sizeof(index_type));

    Element<4> pairs[n];

    std::ostringstream oss;
    oss << "result_" << idx << ".bin";
    WriteFile<2> result(oss.str());

    in.read((char*) pairs, sizeof(Element<4>) * n);

    index_type min = MAX_VALUE, min_idx;
    for (index_type i = 0; i < n; ++i) {
        if (pairs[i][0] < min) {
            min = pairs[i][0];
            min_idx = i;
        }
    }

    index_type cur_idx, rank;
    Element<2> tmp;
//    // std::cout << "expand" << std::endl;
    for (index_type i = 0; i < n; ++i) {
//        // std::cout << i << std::endl;
        rank = 0;
        cur_idx = min_idx;
        while (true) {
            if(pairs[i][0] == pairs[cur_idx][0]) {
                break;
            }
            for (index_type j = 0; j < n; ++j) {
                if (pairs[j][0] == pairs[cur_idx][1]) {
                    rank += pairs[cur_idx][2];
                    cur_idx = j;
                    break;
                }
            }
        }
        tmp[0] = pairs[i][0];
        tmp[1] = rank;
        result.write(tmp);
    }
    result.close();

    bool is_write_n = (idx)? false : true;
    merge_sort<2, 0>(oss.str(), oss.str(), is_write_n);

    in.close();
}

void backward_iter(index_type i) {
    std::ostringstream result_name, d_name, new_result_name;
    result_name << "result_" << i << ".bin";
    d_name << "del_" << i << ".bin";
    new_result_name << "result_" << i - 1 << ".bin";
    File<2> result(result_name.str());
    File<3> d(d_name.str());

    WriteFile<2> new_result(new_result_name.str());

    Element<2> cur_res, prev_res;
    Element<3> cur_d;

    while (result.read()[0] != MAX_VALUE) {
        cur_res = result.read();
        cur_d = d.read();
        if(cur_res[0] == cur_d[0]) {
            prev_res[0] = cur_d[1];
            prev_res[1] = cur_res[1] + cur_d[2];
            new_result.write(prev_res);
            new_result.write(cur_res);
            result.next();
            d.next();
        } else {
            new_result.write(cur_res);
            result.next();
        }
    }
    new_result.close();

    remove(result_name.str().c_str());
    remove(d_name.str().c_str());

    bool is_write_n = (i - 1)? false : true;
    merge_sort<2, 0>(new_result_name.str(), new_result_name.str(), is_write_n);
}

void make_result(const std::string& out_filename) {
    merge_sort<2, 1>("result_0.bin", "result.bin", false);

    File<2> in("result.bin");
    WriteFile<1> out("tmp.bin", false);
    Element<1> el;
    while (true) {
        if(in.read()[0] == MAX_VALUE) {
            break;
        }

        el[0] = in.read()[0];
        in.next();
        out.write(el);
    }
    out.close();

    File<1> tmp("tmp.bin");
    WriteFile<1> output(out_filename.c_str(), false);
    index_type offset, min = MAX_VALUE, idx=0;
    while (true) {
        if (tmp.read()[0] == MAX_VALUE) {
            break;
        }

        if (tmp.read()[0] < min) {
            min = tmp.read()[0];
            offset = idx;
        }

        tmp.next();
        idx++;
    }
    // std::cout << offset << " " << min << std::endl;

    tmp.reopen();
    for (index_type i = 0; i < offset; ++i) {
        tmp.read();
        tmp.next();
    }

    while (true) {
        if (tmp.read()[0] == MAX_VALUE) {
            break;
        }
        // std::cout << tmp.read()[0] << std::endl;
        output.write(tmp.read());
        tmp.next();
    }

    tmp.reopen();
    for (index_type i = 0; i < offset; ++i) {
        // std::cout << tmp.read()[0] << std::endl;
        output.write(tmp.read());
        tmp.next();
    }
}


void make_list_ranking(const std::string& in_filename, const std::string& out_filename) {
    index_type n = init(in_filename.c_str(), "pairs.bin");

    index_type i = 0;
    while (n > MEMORY_SIZE) {
        i++;
        std::ostringstream oss;
        oss << "del_" << i << ".bin";

//        // std::cout << "forward " << n << std::endl;

        n = forward_iter(oss.str());
    }

    expand(i);

    while (i > 0) {

//        // std::cout << "backward " << i << std::endl;
        backward_iter(i);
        i--;
    }

    make_result(out_filename);
}

void prepare_32_to_64(const std::string& in_filename, const std::string& out_filename) {
    std::ifstream in(in_filename.c_str(), std::ios::in | std::ios::binary);
    in.seekg(0, std::ios::end);
    uint32_t file_size = in.tellg() / sizeof(uint32_t);
    in.seekg(0);

    WriteFile<1> out(out_filename, false);
    Element<1> el;
    uint32_t block[MEMORY_SIZE];
    uint32_t read_size = MEMORY_SIZE;

    for (uint32_t i = 0; i < file_size; i += MEMORY_SIZE) {
        if (i + MEMORY_SIZE > file_size) {
            read_size = file_size - i;
        }
        in.read((char*) block, sizeof(uint32_t) * read_size);
        for (uint32_t j = 0; j < read_size; ++j) {
            el[0] = block[j];
            out.write(el);
        }
    }

    in.close();
    out.close();
}

void step_1(const std::string& in_filename, const std::string& out_filename) {
    prepare_32_to_64(in_filename, "new_input.bin");
    File<2> in("new_input.bin", true);
    WriteFile<2> out(out_filename, true);
    Element<2> out_el;
    index_type cur_idx = 0;
    while (in.read()[0] != MAX_VALUE) {
//        std::cout << in.read()[0] << " " << in.read()[1] << std::endl;
        out_el[0] = in.read()[0];
        out_el[1] = in.read()[1];
        out.write(out_el);
        out_el[0] = in.read()[1];
        out_el[1] = in.read()[0];
        out.write(out_el);
        in.next();
    }
    out.close();
    merge_sort<2, 0>(out_filename, out_filename, false);
}

index_type edge_to_num(const Element<2>& edge) {
//    std::cout << edge[0] << " " << edge[1] << " " << (edge[0] << 32) + edge[1] << std::endl;
    return (edge[0] << 32) + edge[1];
}


void step_2(const std::string& in_filename, const std::string& out_filename) {
    File<2> in(in_filename, false);
    WriteFile<2> out(out_filename, true);
    Element<2> first_in, cur_in, next_in, tmp_in, tmp_out;
    cur_in = in.read();
    first_in = cur_in;
    while (cur_in[0] != MAX_VALUE) {
        in.next();
        next_in = in.read();

        if (cur_in[0] == next_in[0]) {
            tmp_out[0] = edge_to_num(cur_in);
            tmp_in[0] = next_in[1];
            tmp_in[1] = next_in[0];
            tmp_out[1] = edge_to_num(tmp_in);
//            std::cout << cur_in[0] << " " << cur_in[1] << " - " << tmp_in[0] << " " << tmp_in[1] << std::endl;
            out.write(tmp_out);
        } else {
            tmp_out[0] = edge_to_num(cur_in);
            tmp_in[0] = first_in[1];
            tmp_in[1] = first_in[0];
            tmp_out[1] = edge_to_num(tmp_in);
//            std::cout << cur_in[0] << " " << cur_in[1] << " - " << tmp_in[0] << " " << tmp_in[1] << std::endl;
            out.write(tmp_out);
            first_in = next_in;
        }
        cur_in = next_in;
    }
}

void make_unique_node(index_type idx, const std::string& in_filename, const std::string& out_filename) {
    File<2> in(in_filename);
    WriteFile<1> out(out_filename, false);
    Element<1> cur;
    cur[0] = in.read()[idx];
    in.next();
    out.write(cur);

    while (in.read()[0] != MAX_VALUE) {
        if (in.read()[idx] != cur[0]) {
            cur[0] = in.read()[idx];
            out.write(cur);
        }
        in.next();
    }
}

index_type find_root() {
    File<2> in("new_input.bin", true);
    WriteFile<2> out("tmp.bin", true);
    while (in.read()[0] != MAX_VALUE) {
        out.write(in.read());
        in.next();
    }
    out.close();

    merge_sort<2, 0>("tmp.bin", "tmp1.bin", false);
    merge_sort<2, 1>("tmp.bin", "tmp2.bin", false);
    make_unique_node(0, "tmp1.bin", "unique_tmp1.bin");
    make_unique_node(1, "tmp2.bin", "unique_tmp2.bin");

    File<1> in_1("unique_tmp1.bin");
    File<1> in_2("unique_tmp2.bin");

    while (in_1.read()[0] != MAX_VALUE) {
        if (in_1.read()[0] == in_2.read()[0]) {
            in_2.next();
        }
        in_1.next();
    }

    return in_2.read()[0];
}

index_type get_first_node(index_type edge_num) {
    return edge_num & (((index_type) 1 << 32) - 1);
}

index_type get_second_node(index_type edge_num) {
    return edge_num >> 32;
}


void transform(index_type root, const std::string& in_filename, const std::string& out_filename) {
    File<1> in(in_filename);
    WriteFile<1> out(out_filename, false);

    while (get_first_node(in.read()[0]) != root) {
//        std::cout << get_first_node(in.read()[0]) << " " << get_second_node(in.read()[0]) << std::endl;
        in.next();
    }

    while (in.read()[0] != MAX_VALUE) {
        out.write(in.read());
        in.next();
    }
    in.reopen();
    while (get_first_node(in.read()[0]) != root) {
        out.write(in.read());
        in.next();
    }
}

void make_magic(const std::string& in_filename, const std::string& out_filename) {
    File<1> in(in_filename);
    WriteFile<2> out(out_filename);
    Element<2> out_el;
    index_type cur_depth = 0;
    index_type first_node, second_node, cur_first_node, cur_second_node;
    first_node = get_first_node(in.read()[0]);
    second_node = get_second_node(in.read()[0]);
    in.next();
    out_el[0] = first_node;
    out_el[1] = cur_depth;
    out.write(out_el);
    out_el[0] = second_node;
    out_el[1] = ++cur_depth;
    out.write(out_el);

    Stack<1> stack;
    Element<1> stack_el;
    stack_el[0] = first_node;
    stack.push(stack_el);

    while (in.read()[0] != MAX_VALUE) {
        cur_first_node = get_first_node(in.read()[0]);
        cur_second_node = get_second_node(in.read()[0]);

        if (stack.top()[0] != cur_second_node) {
//            std::cout << stack.top()[0] << " " << cur_second_node << std::endl;
            out_el[0] = cur_second_node;
            out_el[1] = ++cur_depth;
            out.write(out_el);
            stack_el[0] = second_node;
            stack.push(stack_el);
        } else {
//            std::cout << stack.top()[0] << " " << cur_second_node << std::endl;
            cur_depth--;
            stack.pop();
        }
        first_node = cur_first_node;
        second_node = cur_second_node;
        in.next();
    }
}


void prepare_64_to_32(const std::string& in_filename, const  std::string& out_filename) {
    std::ofstream out(out_filename.c_str(), std::ios::out | std::ios::binary);
    File<1> in(in_filename);

    uint32_t block[MEMORY_SIZE];
    uint32_t cur_idx = 0;

    while (in.read()[0] != MAX_VALUE) {
        block[cur_idx++] = in.read()[0];
        if (cur_idx == MEMORY_SIZE) {
            out.write((char*) block, sizeof(uint32_t) * MEMORY_SIZE);
            cur_idx = 0;
        }

        in.next();
    }
    if (cur_idx) {
        out.write((char*) block, sizeof(uint32_t) * cur_idx);
    }

    out.close();
}


void make_traversal(const std::string& in_filename, const std::string& out_filename) {
    step_1(in_filename, "step_1.bin");
    step_2("step_1.bin", "step_2.bin");
    make_list_ranking("step_2.bin", "step_3.bin");

    index_type root = find_root();

    transform(root, "step_3.bin", "step_4.bin");
    make_magic("step_4.bin", "tmp_output.bin");
    merge_sort<2, 0>("tmp_output.bin", "tmp_output.bin", false);
    prepare_64_to_32("tmp_output.bin", "output.bin");
}



int main() {
    srand(time(NULL));

    make_traversal("input.bin", "output.bin");

    return 0;
}