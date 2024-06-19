from typing import Callable
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib import colormaps
import numpy as np

from utils import (
    default_time_calculation,
    get_label_from_calc_type,
    get_unique_p_t,
    get_title_from_calc_type,
)


# Line Chart
def rectangle_plot(
    df: pd.DataFrame,
    calc_type: str,
    subtitle: str = "",
    time_calculation: Callable = default_time_calculation,
    suffix: str = "",
):
    ps = get_unique_p_t(df)
    m_values = df["m"].unique().tolist()
    k_values = df["k"].unique().tolist()
    divisors = list()
    for i in range(1, len(m_values)):
        if len(m_values) % i == 0:
            divisors.append(i)
    n_rows = divisors[len(divisors) // 2]
    n_cols = len(m_values) // n_rows
    colors = colormaps["viridis"](np.linspace(0.2, 1, len(ps)))
    fig, axs = plt.subplots(n_rows, n_cols, figsize=(16, 12), layout="constrained")
    for ax, m in zip(axs.flat, m_values):
        n_df = df[(df["m"] == m) & (df["n"] == m)]
        ax.set_title(f"M = N={m}")
        ax.set_xlabel("K")
        ax.set_ylabel("GFLOPS")
        handles, labels = ax.get_legend_handles_labels()
        for i, (p, t) in enumerate(ps):
            values = pd.DataFrame(
                n_df[(n_df["processes"] == p) & (n_df["threads"] == t)]
            )
            perfs = values.apply(
                lambda r: (
                    ((2 * r["m"] * r["k"] * r["n"]) / time_calculation(r)) / 1e9
                ),
                axis=1,
            ).tolist()
            ax.plot(
                k_values,
                perfs,
                label=get_label_from_calc_type(calc_type, (p, t)),
                color=colors[i],
            )
            ax.set_xticks(k_values)
        handles, labels = axs.flat[0].get_legend_handles_labels()
        fig.suptitle(
            f"{get_title_from_calc_type(calc_type)}{subtitle}: Rectangle Matrices"
        )
        fig.legend(handles, labels)
        fig.savefig(f"output/{calc_type}_rectangle{suffix}.png")
    plt.close()


def speedup_rectangle_plot(df: pd.DataFrame, calc_type: str):
    ps = get_unique_p_t(df)
    m_values = df["m"].unique().tolist()
    k_values = df["k"].unique().tolist()
    divisors = list()
    for i in range(1, len(m_values)):
        if len(m_values) % i == 0:
            divisors.append(i)
    n_rows = divisors[len(divisors) // 2]
    n_cols = len(m_values) // n_rows
    colors = colormaps["viridis"](np.linspace(0.2, 1, len(ps)))
    fig, axs = plt.subplots(n_rows, n_cols, figsize=(16, 12), layout="constrained")
    for ax, m in zip(axs.flat, m_values):
        n_df = df[(df["m"] == m) & (df["n"] == m)]
        ax.set_title(f"M = N = {m}")
        ax.set_xlabel("K")
        ax.set_ylabel("Speed Up")
        handles, labels = ax.get_legend_handles_labels()
        for i, (p, t) in enumerate(ps):
            ys = list()
            values = pd.DataFrame(
                n_df[(n_df["processes"] == p) & (n_df["threads"] == t)]
            )
            ys = values.apply(
                lambda r: r["serial_time"] / r["parallel_time"], axis=1
            ).tolist()

            ax.plot(
                k_values,
                ys,
                label=get_label_from_calc_type(calc_type, (p, t)),
                color=colors[i],
            )
            ax.set_xticks(k_values)
        handles, labels = axs.flat[0].get_legend_handles_labels()
        fig.suptitle(f"{get_title_from_calc_type(calc_type)}: Rectangle Matrices")
        fig.legend(handles, labels)
        fig.savefig(f"output/speedup_{calc_type}_rectangle.png")
    plt.close()
