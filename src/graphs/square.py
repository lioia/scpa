from __future__ import print_function
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pandas.plotting import parallel_coordinates

from utils import (
    get_label_from_calc_type,
    get_title_from_calc_type,
    get_unique_p_t,
)


def square_plot(df: pd.DataFrame, calc_type: str):
    plt.figure(figsize=(20, 12))
    processes = get_unique_p_t(df)
    for p, t in processes:
        values = df[(df["processes"] == p) & (df["threads"] == t)]
        keys = values.apply(
            lambda r: str(r["k"]),
            axis=1,
        ).tolist()
        perfs = values.apply(
            lambda r: (((2 * r["m"] * r["k"] * r["n"]) / r["parallel_time"]) / 1e9),
            axis=1,
        ).tolist()
        plt.plot(keys, perfs, label=get_label_from_calc_type(calc_type, (p, t)))
        plt.xticks(keys)

    plt.title(f"{get_title_from_calc_type(calc_type)}: Square Matrices")
    plt.xlabel("M = N = K")
    plt.ylabel("GFLOPS")
    plt.legend()
    plt.savefig(f"output/{calc_type}/square.png")
    plt.close()


def square_speedup_plot(df: pd.DataFrame, calc_type: str):
    plt.figure(figsize=(20, 12))
    processes = get_unique_p_t(df)
    for p, t in processes:
        keys = list()
        ys = list()
        values = df[(df["processes"] == p) & (df["threads"] == t)]
        for i in range(len(values)):
            v = values.iloc[i]
            keys.append(str(v["k"]))
            ys.append(v["serial_time"] / v["parallel_time"])

        plt.plot(keys, ys, label=get_label_from_calc_type(calc_type, (p, t)))
        plt.xticks(keys)
    plt.title(f"{get_title_from_calc_type(calc_type)} Speed Up: Square Matrices")
    plt.xlabel("M = N = K")
    plt.ylabel("Speed Up")
    plt.legend()
    plt.savefig(f"output/{calc_type}/speedup_square.png")
    plt.close()


def square_time_distribution_plot(df: pd.DataFrame, calc_type: str):
    k_values = pd.Series(df[df["k"] > 2000]["k"]).unique().tolist()
    processes = get_unique_p_t(df)

    _, ax = plt.subplots(figsize=(12, 7))
    bar_width = 0.4 / len(processes)
    group_spacing = 0.05
    x_pos = np.arange(len(k_values))

    first_comm_bars = []
    parallel_bars = []
    second_comm_bars = []

    for j, k in enumerate(k_values):
        for i, (p, _) in enumerate(processes):
            df_kp = df[(df["k"] == k) & (df["processes"] == p)]
            first_comm_time = pd.Series(df_kp["first_communication_time"]).values[0]
            parallel_time = pd.Series(df_kp["parallel_time"]).values[0]
            second_comm_time = pd.Series(df_kp["second_communication_time"]).values[0]
            total_time = first_comm_time + parallel_time + second_comm_time

            first_comm_time = first_comm_time / total_time * 100
            parallel_time = parallel_time / total_time * 100
            second_comm_time = second_comm_time / total_time * 100

            bar_offset = i - (len(processes) - 1) / 2
            bar_center = x_pos[j] + bar_offset * (bar_width + group_spacing)
            first_comm_bar = ax.bar(
                bar_center,
                first_comm_time,
                bar_width,
                label="First Communication Time" if not first_comm_bars else "",
                color="C0",
            )
            parallel_bar = ax.bar(
                bar_center,
                parallel_time,
                bar_width,
                label="Parallel Time" if not parallel_bars else "",
                color="C1",
                bottom=first_comm_time,
            )
            second_comm_bar = ax.bar(
                bar_center,
                second_comm_time,
                bar_width,
                label="Second Communication Time" if not second_comm_bars else "",
                color="C2",
                bottom=first_comm_time + parallel_time,
            )

            first_comm_bars.append(first_comm_bar)
            parallel_bars.append(parallel_bar)
            second_comm_bars.append(second_comm_bar)

            ax.text(
                bar_center,
                first_comm_time + parallel_time + second_comm_time + 1,
                str(p),
                ha="center",
                va="bottom",
                fontsize=8,
            )

    ax.set_xlabel("K")
    ax.set_ylabel("Percentage of Time")
    ax.set_title("Percentage of Time Spent in Each Phase")
    ax.set_xticks(x_pos)
    ax.set_xticklabels(k_values)
    ax.legend(loc="lower left")

    plt.savefig(f"output/{calc_type}/time_distribution_square.png")
    plt.close()
