from typing import Callable

import pandas as pd
import matplotlib.pyplot as plt

from utils import (
    default_time_calculation,
    get_label_from_calc_type,
    get_title_from_calc_type,
    get_unique_p_t,
)


def square_plot(
    df: pd.DataFrame,
    calc_type: str,
    subtitle: str = "",
    time_calculation: Callable = default_time_calculation,
    suffix: str = "",
):
    plt.figure(figsize=(20, 12))
    processes = get_unique_p_t(df)
    for p, t in processes:
        values = df[(df["processes"] == p) & (df["threads"] == t)]
        keys = values.apply(
            lambda r: f"{int(r['m'])}x{int(r['k'])}x{int(r['n'])}",
            axis=1,
        ).tolist()
        perfs = values.apply(
            lambda r: (((2 * r["m"] * r["k"] * r["n"]) / time_calculation(r)) / 1e9),
            axis=1,
        ).tolist()
        plt.plot(keys, perfs, label=get_label_from_calc_type(calc_type, (p, t)))
        plt.xticks(keys, rotation=90)

    plt.title(f"{get_title_from_calc_type(calc_type)}{subtitle}: Square Matrices")
    plt.xlabel("MxKxN")
    plt.ylabel("GFLOPS")
    plt.legend()
    plt.savefig(f"output/{calc_type}_square{suffix}.png")
    plt.close()


def speedup_square_plot(df: pd.DataFrame, calc_type: str):
    plt.figure(figsize=(20, 12))
    processes = get_unique_p_t(df)
    for p, t in processes:
        keys = list()
        ys = list()
        values = df[(df["processes"] == p) & (df["threads"] == t)]
        for i in range(len(values)):
            v = values.iloc[i]
            keys.append(f"{v['m']}x{v['k']}x{v['n']}")
            ys.append(v["serial_time"] / v["parallel_time"])

        plt.plot(keys, ys, label=get_label_from_calc_type(calc_type, (p, t)))
        plt.xticks(keys, rotation=90)
    plt.xlabel("MxKxN")
    plt.ylabel("Speed Up")
    plt.legend()
    plt.savefig(f"output/speedup_{calc_type}_square.png")
    plt.close()
