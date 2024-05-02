from typing import List, Tuple

import pandas as pd


def get_unique_p_t(df: pd.DataFrame) -> List[Tuple[int, int]]:
    return df[["p", "t"]].drop_duplicates().apply(tuple, axis=1).tolist()


def get_title_from_calc_type(calc_type: str) -> str:
    if calc_type == "mpi":
        return "MPI"
    elif calc_type == "omp":
        return "OpenMP"
    elif calc_type == "mpi-omp":
        return "MPI + OpenMP"
    else:
        raise


def get_label_from_calc_type(calc_type: str, process: Tuple[int, int]) -> str:
    if calc_type == "mpi":
        return f"Processes: {process[0]}"
    elif calc_type == "omp":
        return f"Threads: {process[1]}"
    elif calc_type == "mpi-omp":
        return f"Processes * Threads: {process[0]} * {process[1]} = {process[0] * process[1]}"
    else:
        raise
