from dataclasses import dataclass
from typing import List, Tuple


@dataclass
class PerfList:
    ms: List[int]
    ns: List[int]
    ks: List[int]
    times: List[float]
    keys: List[str]
    perfs: List[float]
    errors: List[float]


def get_label_from_calc_type(calc_type: str, process: Tuple[int, int]) -> str:
    if calc_type == "mpi":
        return f"Processes: {process[0]}"
    elif calc_type == "omp":
        return f"Threads: {process[1]}"
    elif calc_type == "mpi-omp":
        return f"Processes * Threads: {process[0]} * {process[1]} = {process[0] * process[1]}"
    else:
        raise
