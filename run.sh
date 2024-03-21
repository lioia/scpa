#!/bin/bash

if ! [ -f ./build/scpa-matrix-generator ] || ! [ -f ./build/mpi/scpa-mpi ] || ! [ -f ./build/mpi/scpa-mpi-omp ] || ! [ -f ./build/omp/scpa-omp ]; then
    echo "The project was not built correctly. Building now..."
    cmake -B build && cmake --build build
fi

k_vals=(32 64 128 156)
size_vals=(32 64 128 256 512 1024 2048 4096 8192)
p_vals=(2 4 8 16 24)

# Syntax: mpi_run <m> <n> <k>
mpi_run() {
    for p in "${p_vals[@]}"; do
        mpirun -n $p ./build/mpi/scpa-mpi $1 $2 $3
    done
}

# Syntax: mpi_omp_run <m> <n> <k>
mpi_omp_run() {
    for p in "${p_vals[@]}"; do
        for t in "${p_vals[@]}"; do
            mpirun -n $p ./build/mpi/scpa-mpi-omp $1 $2 $3 $t
        done
    done
}

# Syntax: omp_run <m> <n> <k>
omp_run() {
    for t in "${p_vals[@]}"; do
        ./build/omp/scpa-omp $1 $2 $3 $t
    done
}

# Syntax: run[ <type> <m> <n> <k>]
run() {
    # Early return if there is no argument specified (just generate the matrices)
    if [ $1 == "" ]; then
        return
    fi
    # Repeat computation for the matrix 64 times to get valid results
    for ((i = 0; i < 64; i++)); do
        if [ $1 == "mpi" ]; then
            mpi_run $2 $3 $4
        elif [ $1 == "mpi-omp" ]; then
            mpi_omp_run $2 $3 $4
        elif [ $1 == "omp" ]; then
            omp_run $2 $3 $4
        fi
    done
}

# Syntax: main[ <type> <gen_type>]
main() {
    for m in "${size_vals[@]}"; do
        for n in "${size_vals[@]}"; do
            for k in "${k_vals[@]}"; do
                echo "Running size $m $n $k"
                ./build/scpa-matrix-generator $m $n $k $2
                run $1 $m $n $k
            done
        done
    done
}

main $1 $2
