#!/bin/bash

# $1: calculation type: mpi or omp or mpi-omp

if ! [ -d build/bin ]; then
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
p_vals=(4 8 12 16 20)

num_iterations=4

# Syntax: mpi_run <m> <n> <k> <version=1,2> <iteration>
mpi_run() {
    for p in "${p_vals[@]}"; do
        echo -e "\tNumber of Processes (version $4): $p (m: ${1}, n: ${2}, k: ${3})"
        mpirun -n $p ./build/bin/scpa-mpi-v$4 $1 $2 $3 $5
    done
}

# Syntax: mpi_omp_run <m> <n> <k> <version=1,2> <iteration>
mpi_omp_run() {
    p=2
    for ((t = 4; t <= 10; t += 2)); do
        x=$((p * t))
        echo -e "\tNumber of Processes: $p; Number of Threads: $t (version $4, m: ${1}, n: ${2}, k: ${3}; x = ${x})"
        mpirun -n $p ./build/bin/scpa-mpi-omp-v$4 $1 $2 $3 $t $5
    done
    p=4
    for ((t = 2; t <= 5; t += 1)); do
        x=$((p * t))
        echo -e "\tNumber of Processes: $p; Number of Threads: $t (version $4, m: ${1}, n: ${2}, k: ${3}; x = ${x})"
        mpirun -n $p ./build/bin/scpa-mpi-omp-v$4 $1 $2 $3 $t $5
    done
}

# Syntax: omp_run <m> <n> <k> <iteration>
omp_run() {
    for t in "${p_vals[@]}"; do
        echo -e "\tNumber of Threads: $t (m: ${1}, n: ${2}, k: ${3})"
        ./build/bin/scpa-omp $1 $2 $3 $t $4
    done
}

# Syntax: run[ <type> <m> <n> <k> <iteration>]
run() {
    # Early return if there is no argument specified (just generate the matrices)
    if [ $1 == "" ]; then
        return
    fi
    if [ $1 == "mpi" ]; then
        mpi_run $2 $3 $4 1 $5
        mpi_run $2 $3 $4 2 $5
    elif [ $1 == "mpi-omp" ]; then
        mpi_omp_run $2 $3 $4 1 $5
        mpi_omp_run $2 $3 $4 2 $5
    elif [ $1 == "omp" ]; then
        omp_run $2 $3 $4 $5
    fi
}

# Syntax: main[ <calc_type>]
main() {
    echo "Square Matrices"
    for m in "${square_size_vals[@]}"; do
        echo -e "\tRunning (size: ${m})"
        for ((i = 0; i < $num_iterations; i++)); do
            run $1 $m $m $m $i
        done
    done
    echo "Rectangle Matrices"
    for m_n in "${m_n_vals[@]}"; do
        for k in "${k_vals[@]}"; do
            if [[ m -ne n ]] || [[ m -ne k ]]; then 
                echo -e "\tRunning (m = n: ${m_n}, k: ${k})"
                for ((i = 0; i < $num_iterations; i++)); do
                    run $1 $m_n $m_n $k $i
                done
            fi
        done
    done
}

main $1
