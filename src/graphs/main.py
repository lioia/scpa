import sys
import os
from typing import Optional

import pandas as pd

from utils import mean_excluding_zeros
from rectangle import (
    rectangle_plot,
    rectangle_time_distribution_plot,
    speedup_rectangle_plot,
)
from square import square_speedup_plot, square_plot, square_time_distribution_plot


def main(calc_type: Optional[str]):
    types = (
        ["mpi-v1", "mpi-v2", "omp", "mpi-omp-v1", "mpi-omp-v2"]
        if calc_type is None
        else [calc_type]
    )
    for calc_type in types:
        if not os.path.exists(f"output/{calc_type}"):
            os.mkdir(f"output/{calc_type}")
        filepath = f"output/{calc_type}.csv"
        df = pd.read_csv(filepath)
        df = (
            df.groupby(["m", "n", "k", "processes", "threads"])
            .agg(
                {
                    "error": "mean",
                    "generation_time": "mean",
                    "first_communication_time": "mean",
                    "second_communication_time": "mean",
                    "parallel_time": "mean",
                    "serial_time": mean_excluding_zeros,
                }
            )
            .reset_index()
        )
        square = df[(df["m"] == df["n"]) & (df["n"] == df["k"])]
        square_plot(pd.DataFrame(square), calc_type)
        square_speedup_plot(pd.DataFrame(square), calc_type)
        rectangle = pd.concat([df, square]).drop_duplicates(keep=False)
        rectangle_plot(pd.DataFrame(rectangle), calc_type)
        speedup_rectangle_plot(pd.DataFrame(rectangle), calc_type)
        if calc_type == "mpi-v1" or calc_type == "mpi-v2":
            square_time_distribution_plot(pd.DataFrame(square), calc_type)
            rectangle_time_distribution_plot(pd.DataFrame(rectangle), calc_type)


if __name__ == "__main__":
    if len(sys.argv) < 1:
        print(f"Usage: python -m {sys.argv[0]} [<calc_type>]")
        exit()
    calc_type = None
    if len(sys.argv) == 2:
        calc_type = sys.argv[1]
    main(calc_type)
