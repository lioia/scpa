import sys
from typing import Optional
import pandas as pd

from rectangle import rectangle_plot
from square import square_plot, speedup_square_plot


def main(folder: str, calc_type: Optional[str]):
    types = ["mpi", "omp", "mpi-omp"] if calc_type is None else [calc_type]
    serial = pd.read_csv(f"{folder}/serial.csv")
    serial_square = serial[(serial["m"] == serial["n"]) & (serial["n"] == serial["k"])]
    # serial_rectangle = pd.concat([serial, serial_square]).drop_duplicates(keep=False)
    for calc_type in types:
        filepath = f"{folder}/{calc_type}.csv"
        df = pd.read_csv(filepath)
        square = df[(df["m"] == df["n"]) & (df["n"] == df["k"])]
        square_plot(pd.DataFrame(square), calc_type)
        speedup_square_plot(
            pd.DataFrame(serial_square),
            pd.DataFrame(square),
            calc_type,
        )
        rectangle = pd.concat([df, square]).drop_duplicates(keep=False)
        rectangle_plot(pd.DataFrame(rectangle), calc_type)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: python -m {sys.argv[0]} <output_folder>[, <calc_type>]")
        exit()
    main(sys.argv[1], sys.argv[2])
