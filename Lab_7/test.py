import os
import time
import argparse


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-g', action='store_true')
    parser.add_argument('-n', type=int)
    args = parser.parse_args()

    if not args.g:
        N = int(input("Enter N:\n"))

        ar = [int(x) for x in input("Enter N numbers:\n").split()]
    else:
        N = args.n
        ar = [0] * N

    with open('input.bin', 'wb') as file:
        file.write(N.to_bytes(8, byteorder='little'))
        for a in ar:
            file.write(a.to_bytes(8, byteorder='little'))

    begin_time = time.time()
    os.system('./main')
    end_time = time.time()

    print(end_time - begin_time)

    # with open('output.bin', 'rb') as file:
    #     N = int.from_bytes(file.read(8), byteorder='little')
    #     ar = [int.from_bytes(file.read(8), byteorder='little') for _ in range(N)]
    #
    #     print("N=%d" % N)
    #     print(ar)
