import random
import time

SIZE = 1024

def MatrixMultiply(A, B, C, sanity=False):
    for i in range(len(C)):
        if(sanity):
            print("Multiplying i = ", i)
        for j in range(len(C[0])):
            for k in range(len(A[0])):
                C[i][j] += A[i][k] * B[k][j]

A = []
B = []
C = []
for i in range(SIZE):
    A.append([])
    B.append([])
    C.append([0.0]*SIZE)
    for j in range(SIZE):
        A[i].append(random.random())
        B[i].append(random.random())

start = time.perf_counter()
MatrixMultiply(A, B, C)
end = time.perf_counter()
print("Multiplication time (N=", SIZE, "):", end-start)
