import csv
import sys
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

import matplotlib.pyplot as plt


@dataclass
class PerfList:
    ms: List[int]
    ns: List[int]
    ks: List[int]
    times: List[float]
    keys: List[str]
    perfs: List[float]
    errors: List[float]


def csv_parser(filepath: str, matrix_type: str) -> Dict[Tuple[int, int], PerfList]:
    if matrix_type != "rectangle" and matrix_type != "square":
        print("Wrong parameter passed to matrix_type")
        return {}
    values: Dict[Tuple[int, int], PerfList] = {}
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
            t = int(row[4])
            time = float(row[5])
            error = float(row[6])
            perf = (2 * m * n * k) / time
            key = f"{m}x{k}x{n}"
            x = (p, t)
            if x not in values:
                values[x] = PerfList([], [], [], [], [], [], [])

            if key in values[x].keys:
                continue

            values[x].ms.append(m)
            values[x].ks.append(k)
            values[x].ns.append(n)
            values[x].times.append(time)
            values[x].keys.append(key)
            values[x].perfs.append(perf / 1e9)
            values[x].errors.append(error)
    return values


def get_label_from_calc_type(calc_type: str, process: Tuple[int, int]) -> str:
    if calc_type == "mpi":
        return f"Processes: {process[0]}"
    elif calc_type == "omp":
        return f"Threads: {process[1]}"
    elif calc_type == "mpi-omp":
        return f"Processes * Threads: {process[0]} * {process[1]} = {process[0] * process[1]}"
    else:
        raise


def square_plot(processes: Dict[Tuple[int, int], PerfList], calc_type: str):
    plt.figure(figsize=(20, 12))
    for process, values in processes.items():
        plt.plot(
            values.keys,
            values.perfs,
            label=get_label_from_calc_type(calc_type, process),
        )
        plt.xticks(values.keys, rotation=90)

    plt.xlabel("MxKxN")
    plt.ylabel("GFLOPS")
    plt.legend()
    plt.savefig(f"output/{calc_type}_square.png")


def speedup_square_plot(
    serial: PerfList,
    processes: Dict[Tuple[int, int], PerfList],
    calc_type: str,
):
    plt.figure(figsize=(20, 12))
    for process, values in processes.items():
        plt.plot(
            values.keys,
            [serial.times[i] / values.times[i] for i in range(len(values.times))],
            label=get_label_from_calc_type(calc_type, process),
        )
    plt.xlabel("MxKxN")
    plt.ylabel("Speed Up")
    plt.legend()
    plt.savefig(f"output/speedup_{calc_type}_square.png")


def main(folder: str, calc_type: Optional[str]):
    types = ["mpi", "omp", "mpi-omp"] if calc_type is None else [calc_type]
    serial_square = csv_parser(f"{folder}/serial.csv", "square")[(1, 1)]
    for calc_type in types:
        filepath = f"{folder}/{calc_type}.csv"
        square = csv_parser(filepath, "square")
        square_plot(square, calc_type)
        speedup_square_plot(serial_square, square, calc_type)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: python -m {sys.argv[0]} <output_folder>[, <calc_type>]")
        exit()
    main(sys.argv[1], sys.argv[2])
