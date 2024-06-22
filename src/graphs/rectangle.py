import matplotlib.pyplot as plt
import pandas as pd
from matplotlib import colormaps
import numpy as np

from utils import (
    get_label_from_calc_type,
    get_unique_p_t,
    get_title_from_calc_type,
)


def rectangle_plot(df: pd.DataFrame, calc_type: str):
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
                lambda r: (((2 * r["m"] * r["k"] * r["n"]) / r["parallel_time"]) / 1e9),
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
        fig.suptitle(f"{get_title_from_calc_type(calc_type)}: Rectangle Matrices")
        fig.legend(handles, labels)
        fig.savefig(f"output/{calc_type}/rectangle.png")
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
        fig.savefig(f"output/{calc_type}/speedup_rectangle.png")
    plt.close()


def rectangle_time_distribution_plot(df: pd.DataFrame, calc_type: str):
    m_values = pd.Series(df[df["m"] == 5000]["m"]).unique().tolist()
    k_values = pd.Series(df["k"]).unique().tolist()
    processes = get_unique_p_t(df)

    _, ax = plt.subplots(figsize=(12, 7))
    bar_width = 0.4 / len(processes)
    group_spacing = 0.05
    x_pos = np.arange(len(k_values))

    first_comm_bars = []
    parallel_bars = []
    second_comm_bars = []

    for m in m_values:
        for j, k in enumerate(k_values):
            for i, (p, _) in enumerate(processes):
                df_kp = df[
                    (df["k"] == k)
                    & (df["m"] == m)
                    & (df["n"] == m)
                    & (df["processes"] == p)
                ]
                first_time = pd.Series(df_kp["first_communication_time"]).values[0]
                parallel_time = pd.Series(df_kp["parallel_time"]).values[0]
                second_time = pd.Series(df_kp["second_communication_time"]).values[0]
                total_time = first_time + parallel_time + second_time

                first_time = first_time / total_time * 100
                parallel_time = parallel_time / total_time * 100
                second_time = second_time / total_time * 100

                bar_offset = i - (len(processes) - 1) / 2
                bar_center = x_pos[j] + bar_offset * (bar_width + group_spacing)
                first_comm_bar = ax.bar(
                    bar_center,
                    first_time,
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
                    bottom=first_time,
                )
                second_comm_bar = ax.bar(
                    bar_center,
                    second_time,
                    bar_width,
                    label="Second Communication Time" if not second_comm_bars else "",
                    color="C2",
                    bottom=first_time + parallel_time,
                )

                first_comm_bars.append(first_comm_bar)
                parallel_bars.append(parallel_bar)
                second_comm_bars.append(second_comm_bar)

                ax.text(
                    bar_center,
                    first_time + parallel_time + second_time + 1,
                    str(p),
                    ha="center",
                    va="bottom",
                    fontsize=8,
                )

        ax.set_xlabel("K")
        ax.set_ylabel("Percentage of Time")
        ax.set_title(f"Percentage of Time Spent in Each Phase M = N = {m}")
        ax.set_xticks(x_pos)
        ax.set_xticklabels(k_values)
        ax.legend(loc="lower left")

        plt.savefig(f"output/{calc_type}/time_distribution_rectangle_{m}.png")
    plt.close()
