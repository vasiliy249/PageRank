import numpy as np


def power_iter_matrix(weights, graph, epsilon, d, max_iter):
    weights = [weight / weights.sum() for weight in weights]
    A = np.empty(graph.shape)
    for idx, col in enumerate(graph):
        A[idx] = (col / col.sum()) if col.sum() else col

    A = A.T
    old_weights = weights
    iter_count = max_iter
    for it in range(max_iter):
        weights = (1 - d) + d * (A @ old_weights)
        err = (np.absolute(weights - old_weights)).sum()
        print(err)
        if err < epsilon:
            iter_count = it
            break
        old_weights = weights

    weights = [weight * 100 / weights.sum() for weight in weights]

    return weights, iter_count

def power_iter_rel(weights, rels, epsilon, d, max_iter):
    weights = [weight / weights.sum() for weight in weights]
    weights = np.asarray(weights)
    count_out = np.zeros(len(weights) + 1)
    nodes_in = {}

    for rel in rels:
        count_out[rel[0]] += 1
        if rel[1] in nodes_in:
            (nodes_in[rel[1]]).append(rel[0])
        else:
            nodes_in[rel[1]] = [rel[0]]

    old_weights = weights.copy()
    iter_count = max_iter
    for it in range(max_iter):
        for index, weight in enumerate(weights):
            sum_rank = 0
            if index in nodes_in:
                for inner in nodes_in[index]:
                    sum_rank += old_weights[inner] / count_out[inner]
            weights[index] = (1 - d) + d * sum_rank

        err = (np.absolute(weights - old_weights)).sum()
        print(err)
        if err < epsilon:
            iter_count = it
            break
        old_weights = weights.copy()


    weights = [weight * 100 / weights.sum() for weight in weights]

    return weights, iter_count