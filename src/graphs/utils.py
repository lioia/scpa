from typing import List, Tuple

import pandas as pd


def get_unique_p_t(df: pd.DataFrame) -> List[Tuple[int, int]]:
    return df[["processes", "threads"]].drop_duplicates().apply(tuple, axis=1).tolist()


def get_title_from_calc_type(calc_type: str) -> str:
    if calc_type == "mpi-v1":
        return "MPI v1"
    elif calc_type == "mpi-v2":
        return "MPI v2"
    elif calc_type == "omp":
        return "OpenMP"
    elif calc_type == "mpi-omp-v1":
        return "MPI v1 + OpenMP"
    elif calc_type == "mpi-omp-v2":
        return "MPI v2 + OpenMP"
    else:
        raise


def get_label_from_calc_type(calc_type: str, process: Tuple[int, int]) -> str:
    if calc_type == "mpi-v1" or calc_type == "mpi-v2":
        return f"Processes: {process[0]}"
    elif calc_type == "omp":
        return f"Threads: {process[1]}"
    elif calc_type == "mpi-omp-v1" or calc_type == "mpi-omp-v2":
        return f"Processes * Threads: {process[0]} * {process[1]} = {process[0] * process[1]}"
    else:
        raise


def default_time_calculation(df):
    return df["parallel_time"]


def mpi_time_calculation(df):
    return (
        df["parallel_time"]
        + df["first_communication_time"]
        + df["second_communication_time"]
    )


def mean_excluding_zeros(series):
    non_zero_values = series[series != 0.0]
    if len(non_zero_values) == 0:
        return 0
    return non_zero_values.mean()
