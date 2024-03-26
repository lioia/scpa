import matplotlib.pyplot as plt
import sys
import csv
from typing import List, Tuple, Dict


def csv_parser(
    filepath: str, matrix_type: str
) -> Dict[int, Tuple[List[str], List[float], List[float]]]:
    if matrix_type != "rectangle" and matrix_type != "square":
        print("Wrong parameter passed to matrix_type")
        return {}
    # processes -> Tuple[List(x), List(perf), Listerror)]
    processes: Dict[int, Tuple[List[str], List[float], List[float]]] = {}
    with open(filepath) as csvfile:
        reader = csv.reader(csvfile)
        next(reader, None)
        for row in reader:
            m = int(row[0])
            n = int(row[1])
            k = int(row[2])
            if matrix_type == "rectangle" and m == n and m == k and k == n:
                continue
            elif matrix_type == "square" and (m != n or m != k or k != n):
                continue
            p = int(row[3])
            time = float(row[4])
            error = float(row[5])
            perf = 2 * m * n * k / time
            key = f"{m}x{k}x{n}"
            if p not in processes:
                processes[p] = ([], [], [])
            processes[p][0].append(key)
            processes[p][1].append(perf / 1e9)
            processes[p][2].append(error)
    return processes


def get_label_from_calc_type(calc_type: str, process: int) -> str:
    if calc_type == "mpi":
        return f"Processes: {process}"
    elif calc_type == "omp":
        return f"Threads: {process}"
    elif calc_type == "mpi-omp":
        return f"Processes * Threads: {process}"
    else:
        raise


def create_plot(
    processes: Dict[int, Tuple[List[str], List[float], List[float]]],
    calc_type: str,
    graph_type: str,
    matrix_type: str,
):
    if graph_type not in ["perf", "error"]:
        print("Incorrect usage of create_plot")
        exit()
    plt.figure(figsize=(10, 8))
    for process, values in processes.items():
        x = values[0]
        if graph_type == "perf":
            y = values[1]
        else:
            y = values[2]
        plt.plot(x, y, label=get_label_from_calc_type(calc_type, process))
        if len(x) > 30:
            subset_indices = range(0, len(x), len(x) // 30)
            subset_labels = [x[i] for i in subset_indices]
            plt.xticks(subset_indices, subset_labels, rotation=45)
        else:
            plt.xticks(x, rotation=45)

    plt.xlabel("MxKxN")
    if graph_type == "perf":
        plt.ylabel("GFLOPS")
    elif graph_type == "error":
        plt.ylabel("Error")
    plt.legend()
    plt.savefig(f"output/{calc_type}_{graph_type}_{matrix_type}.png")


def main(filepath: str, calc_type: str):
    p = csv_parser(filepath, "square")
    create_plot(p, calc_type, "perf", "square")
    create_plot(p, calc_type, "error", "square")
    p = csv_parser(filepath, "rectangle")
    create_plot(p, calc_type, "perf", "rectangle")
    create_plot(p, calc_type, "error", "rectangle")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: python -m {sys.argv[0]} <path/to/file.csv> <calc_type>")
        exit()
    if sys.argv[2] not in ["mpi", "omp", "mpi-omp"]:
        print("Unrecognized calc_type")
        exit()
    main(sys.argv[1], sys.argv[2])
