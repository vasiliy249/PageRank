import PageRank
import numpy as np

test_matrix = False

stanford_file = 'web-Stanford.txt'
stanford_node_count = 281903
google_file = 'web-Google.txt'
google_node_count = 875713

node_count = stanford_node_count
file_name = stanford_file

if test_matrix:
    P = np.array([[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0],
                  [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                  [1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                  [0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0],
                  [0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0],
                  [0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0],
                  [0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0],
                  [0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0],
                  [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]])
    node_count = P.shape[0]
    x = np.ones(node_count)
    weights, iter_count = PageRank.power_iter_matrix(x, graph=P, epsilon=1e-20, d=0.85, max_iter=1000)

    print("Weights:")
    print(weights)
    print("Iteration count:")
    print(iter_count)
else:

    P = None
    # P = {(1,2), (2,1), (3,0), (3,1), (4,3), (4,1), (4,5), (5,1), (5,4), (6,1), (6,4), (7,1), (7,4), (8,1), (8,4), (9,4), (10,4)}
    # node_count = 11
    with open(file_name, 'r') as f:
        line_count = sum(1 for line in f)
        P = np.zeros(line_count, dtype='int32, int32')
        f.seek(0)
        for i, line in enumerate(f):
            P[i] = tuple([int(x) - 1 for x in line.split()])

    x = np.ones(node_count)
    print("parsing completed, start computing")
    weights, iter_count = PageRank.power_iter_rel(x, rels=P, epsilon=1e-5, d=0.85, max_iter=1000)

    print("Weights:")
    print(weights)
    print("Iteration count:")
    print(iter_count)