import os
import time
import argparse


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-n', type=int)
    parser.add_argument('-m', type=int)

    args = parser.parse_args()

    with open('input.bin', 'wb') as file:
        n, m = args.n, args.m
        file.write(n.to_bytes(4, byteorder='little'))
        file.write(m.to_bytes(4, byteorder='little'))
        file.write(b'\x00' * (n * m))

    begin_time = time.time()
    os.system('./main')
    end_time = time.time()

    print(end_time - begin_time)
