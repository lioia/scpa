import matplotlib.pyplot as plt
import pandas as pd
from matplotlib import colormaps
import numpy as np

from utils import get_label_from_calc_type, get_unique_p_t, get_title_from_calc_type


# Line Chart
def rectangle_plot(df: pd.DataFrame, calc_type: str):
    ps = get_unique_p_t(df)
    m_values = df["m"].unique().tolist()
    n_values = df["n"].unique().tolist()
    k_values = df["k"].unique().tolist()
    divisors = list()
    for i in range(1, len(n_values)):
        if len(n_values) % i == 0:
            divisors.append(i)
    n_rows = divisors[len(divisors) // 2]
    n_cols = len(n_values) // n_rows
    colors = colormaps["viridis"](np.linspace(0.2, 1, len(ps)))
    for m in m_values:
        fig, axs = plt.subplots(n_rows, n_cols, figsize=(20, 12), layout="constrained")
        m_df = df[df["m"] == m]
        for ax, n in zip(axs.flat, n_values):
            n_df = m_df[m_df["n"] == n]
            ax.set_title(f"N={n}")
            ax.set_xlabel("K")
            ax.set_ylabel("GFLOPS")
            handles, labels = ax.get_legend_handles_labels()
            for i, (p, t) in enumerate(ps):
                values = pd.DataFrame(n_df[(n_df["p"] == p) & (n_df["t"] == t)])
                perfs = values.apply(
                    lambda r: (((2 * r["m"] * r["k"] * r["n"]) / r["time"]) / 1e9),
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
            f"{get_title_from_calc_type(calc_type)}: Rectangle Matrices, M = {m}"
        )
        fig.legend(handles, labels)
        fig.savefig(f"output/{calc_type}_rectangle_{m}.png")


# Bar Chart
# def rectangle_plot(df: pd.DataFrame, calc_type: str):
#     processes = get_unique_p_t(df)
#     arbitrary_value = pd.DataFrame(
#         df[(df["p"] == processes[0][0]) & (df["t"] == processes[0][1])]
#     )
#     keys = df[["m", "n"]].drop_duplicates().apply(tuple, axis=1).tolist()
#     width = 0.2
#     multiplier = 0
#     x = np.arange(len(keys))
#     fig, ax = plt.subplots(layout="constrained", figsize=(20, 12))
#     k_values = df["k"].unique().tolist()
#     for k in k_values:
#         offset = width * multiplier
#         perfs = arbitrary_value[(arbitrary_value["k"] == k)].apply(
#             lambda r: (((2 * r["m"] * r["k"] * r["n"]) / r["time"]) / 1e9),
#             axis=1,
#         )
#         ax.bar(x + offset, perfs, width, label=k)
#         multiplier += 1
#
#     ax.set_title("Rectangle Matrices")
#     ax.set_ylabel("GFLOPS")
#     ax.set_xlabel("MxKxN")
#     ax.set_xticks(x + width, [f"{k[0]}x{k[1]}" for k in keys], rotation=90)
#     ax.legend()
#     fig.savefig(f"output/{calc_type}_rectangle.png")
