import os
import time
import argparse


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
        ar = []
        for i in range(N, 0, -1):
            ar.extend([i, i - 1])
        ar[-1] = N

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
            N = int.from_bytes(file.read(4), byteorder='little')
            ar = [int.from_bytes(file.read(4), byteorder='little') for _ in range(2 * N)]
    
            print("N=%d" % N)
            print(ar)

