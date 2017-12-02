import os
import time
import argparse
import random


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-g', action='store_true')
    parser.add_argument('-n', type=int)
    parser.add_argument('-o', action='store_true')
    args = parser.parse_args()

    if not args.g:
        N = int(input("Enter N:\n"))

        ar = [int(x) for x in input("Enter 2 * N numbers:\n").split()]
    else:
        N = args.n
        nodes = list(range(1, N + 1))
        random.shuffle(nodes)

        edges = []
        for i in range(N):
            edges.append([nodes[i], nodes[(i + 1) % N]])

        random.shuffle(edges)

        ar = []
        for e in edges:
            ar.extend(e)

    with open('input.bin', 'wb') as file:
        file.write(N.to_bytes(4, byteorder='little'))
        for a in ar:
            file.write(a.to_bytes(4, byteorder='little'))

    if args.o:
        with open('input.bin', 'rb') as file:
            N = int.from_bytes(file.read(4), byteorder='little')
            ar = [int.from_bytes(file.read(4), byteorder='little') for _ in range(2 * N)]
    
            print("N=%d" % N)
            print(ar)

    begin_time = time.time()
    os.system('./main')
    end_time = time.time()

    print(end_time - begin_time)

    if args.o:
        with open('output.bin', 'rb') as file:
            ar = [int.from_bytes(file.read(4), byteorder='little') for _ in range(N)]
    
            print("N=%d" % N)
            print(ar)

