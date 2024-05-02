import pandas as pd
import matplotlib.pyplot as plt

from utils import get_label_from_calc_type, get_title_from_calc_type, get_unique_p_t


def square_plot(df: pd.DataFrame, calc_type: str):
    plt.figure(figsize=(20, 12))
    processes = get_unique_p_t(df)
    for p, t in processes:
        values = df[(df["p"] == p) & (df["t"] == t)]
        keys = values.apply(
            lambda r: f"{int(r['m'])}x{int(r['k'])}x{int(r['n'])}",
            axis=1,
        ).tolist()
        perfs = values.apply(
            lambda r: (((2 * r["m"] * r["k"] * r["n"]) / r["time"]) / 1e9),
            axis=1,
        ).tolist()
        plt.plot(keys, perfs, label=get_label_from_calc_type(calc_type, (p, t)))
        plt.xticks(keys, rotation=90)

    plt.title(f"{get_title_from_calc_type(calc_type)}: Square Matrices")
    plt.xlabel("MxKxN")
    plt.ylabel("GFLOPS")
    plt.legend()
    plt.savefig(f"output/{calc_type}_square.png")


def speedup_square_plot(serial: pd.DataFrame, df: pd.DataFrame, calc_type: str):
    plt.figure(figsize=(20, 12))
    processes = get_unique_p_t(df)
    for p, t in processes:
        keys = list()
        ys = list()
        values = df[(df["p"] == p) & (df["t"] == t)]
        for i in range(len(values)):
            v = values.iloc[i]
            s = serial.iloc[i]
            keys.append(f"{v['m']}x{v['k']}x{v['n']}")
            ys.append(s["time"] / v["time"])

        plt.plot(keys, ys, label=get_label_from_calc_type(calc_type, (p, t)))
        plt.xticks(keys, rotation=90)
    plt.xlabel("MxKxN")
    plt.ylabel("Speed Up")
    plt.legend()
    plt.savefig(f"output/speedup_{calc_type}_square.png")
