#!/bin/bash

# $1: calculation type: mpi or omp or mpi-omp

if ! [ -f ./build/bin/scpa-mpi ] || ! [ -f ./build/bin/scpa-mpi-omp ] || ! [ -f ./build/bin/scpa-omp ]; then
    echo "The project was not built. Building now..."
    module load gnu mpich # or module load mpi in local development environment
    cmake -B build && cmake --build build
fi

if [[ "$1" != "mpi" && "$1" != "omp" && "$1" != "mpi-omp" ]]; then
    echo "Incorrect argument $1: expecting mpi, omp or mpi-omp"
    exit 1
fi

square_size_vals=(32 64 128 256 512 1024 2500 5000 10000)
m_n_vals=(2500 5000 10000)
k_vals=(32 64 128 156)
p_vals=(2 4 8 16 24 32)

num_iterations=20

# Syntax: mpi_run <m> <n> <k>
mpi_run() {
    for p in "${p_vals[@]}"; do
        echo -e "\tNumber of Processes: $p (m: ${1}, n: ${2}, k: ${3})"
        mpirun -n $p ./build/bin/scpa-mpi $1 $2 $3
    done
}

# Syntax: mpi_omp_run <m> <n> <k>
mpi_omp_run() {
    p=2
    for ((t = 4; t <= 16; t += 4)); do
        x=$((p * t))
        echo -e "\tNumber of Processes: $p; Number of Threads: $t (m: ${1}, n: ${2}, k: ${3}; x = ${x})"
        mpirun -n $p ./build/bin/scpa-mpi-omp $1 $2 $3 $t
    done
    p=4
    for ((t = 4; t <= 8; t += 2)); do
        x=$((p * t))
        echo -e "\tNumber of Processes: $p; Number of Threads: $t (m: ${1}, n: ${2}, k: ${3}; x = ${x})"
        mpirun -n $p ./build/bin/scpa-mpi-omp $1 $2 $3 $t
    done
}

# Syntax: omp_run <m> <n> <k>
omp_run() {
    for t in "${p_vals[@]}"; do
        echo -e "\tNumber of Threads: $t (m: ${1}, n: ${2}, k: ${3})"
        ./build/bin/scpa-omp $1 $2 $3 $t
    done
}

# Syntax: run[ <type> <m> <n> <k>]
run() {
    # Early return if there is no argument specified (just generate the matrices)
    if [ $1 == "" ]; then
        return
    fi
    if [ $1 == "mpi" ]; then
        mpi_run $2 $3 $4
    elif [ $1 == "mpi-omp" ]; then
        mpi_omp_run $2 $3 $4
    elif [ $1 == "omp" ]; then
        omp_run $2 $3 $4
    fi
}

# Syntax: main[ <calc_type>]
main() {
    echo "Square Matrices"
    for m in "${square_size_vals[@]}"; do
        echo -e "\tRunning (size: ${m})"
        run $1 $m $m $m
    done
    echo "Rectangle Matrices"
    for m in "${m_n_vals[@]}"; do
        for n in "${m_n_vals[@]}"; do
            for k in "${k_vals[@]}"; do
                if [[ m -ne n ]] || [[ m -ne k ]]; then 
                    echo -e "\tRunning (m: ${m}, n: ${n}, k: ${k})"
                    for ((i=1; i<=$num_iterations; i++)); do
                        run $1 $m $n $k
                    done
                fi
            done
        done
    done
}

main $1
