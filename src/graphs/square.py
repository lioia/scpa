from typing import Dict, Tuple

import matplotlib.pyplot as plt

from utils import PerfList, get_label_from_calc_type


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
