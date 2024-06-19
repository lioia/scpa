import sys
from typing import Optional
import pandas as pd

from utils import mpi_time_calculation, mean_excluding_zeros
from rectangle import rectangle_plot, speedup_rectangle_plot
from square import speedup_square_plot, square_plot


def main(folder: str, calc_type: Optional[str]):
    types = (
        ["mpi-v1", "mpi-v2", "omp", "mpi-omp-v1", "mpi-omp-v2"]
        if calc_type is None
        else [calc_type]
    )
    for calc_type in types:
        filepath = f"{folder}/{calc_type}.csv"
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
        speedup_square_plot(pd.DataFrame(square), calc_type)
        rectangle = pd.concat([df, square]).drop_duplicates(keep=False)
        rectangle_plot(pd.DataFrame(rectangle), calc_type)
        speedup_rectangle_plot(pd.DataFrame(rectangle), calc_type)
        if calc_type != "omp":
            square_plot(
                pd.DataFrame(square),
                calc_type,
                " (with communication time)",
                mpi_time_calculation,
                "_comm",
            )
            rectangle_plot(
                pd.DataFrame(rectangle),
                calc_type,
                " (with communication time)",
                mpi_time_calculation,
                "_comm",
            )
            # stacked bar chart for times


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: python -m {sys.argv[0]} <output_folder>[, <calc_type>]")
        exit()
    calc_type = None
    if len(sys.argv) == 3:
        calc_type = sys.argv[2]
    main(sys.argv[1], calc_type)
