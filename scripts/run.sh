#!/bin/bash

# $1: calculation type: mpi or omp or mpi-omp
# $2: generation type: random or index

if ! [ -f ./build/scpa-matrix-generator ] || ! [ -f ./build/mpi/scpa-mpi ] || ! [ -f ./build/mpi/scpa-mpi-omp ] || ! [ -f ./build/omp/scpa-omp ]; then
    echo "The project was not built correctly. Building now..."
    module load gnu mpich # or module load mpi in local development environment
    cmake -B build && cmake --build build
fi

k_vals=(32 64 128 156)
size_vals=(32 64 128 256 512 1024 2048 4096 8192)
p_vals=(2 4 8 16 24 32)

# Syntax: mpi_run <m> <n> <k>
mpi_run() {
    for p in "${p_vals[@]}"; do
        echo -e "\tNumber of Processes: $p (m: ${1}, n: ${2}, k: ${3})"
        mpirun -n $p ./build/mpi/scpa-mpi $1 $2 $3
    done
}

# Syntax: mpi_omp_run <m> <n> <k>
mpi_omp_run() {
    length=${#p_vals[@]}
    for ((i = 0; i < length; i++)); do
        p=${p_vals[i]}
        for ((j = i + 1; j < length; j++)); do
            t=${p_vals[j]}
            x=$((p * t))
            echo -e "\tNumber of Processes: $p; Number of Threads: $t (m: ${1}, n: ${2}, k: ${3}; x = ${x})"
            mpirun -n $p ./build/mpi/scpa-mpi-omp $1 $2 $3 $t
        done
    done
}

# Syntax: omp_run <m> <n> <k>
omp_run() {
    for t in "${p_vals[@]}"; do
        echo -e "\tNumber of Threads: $t (m: ${1}, n: ${2}, k: ${3})"
        ./build/omp/scpa-omp $1 $2 $3 $t
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

# Syntax: main[ <calc_type> <gen_type>]
main() {
    echo "Square Matrices"
    for m in "${size_vals[@]}"; do
        echo -e "\tGenerating Matrix (size: ${m})"
        ./build/scpa-matrix-generator $m $m $m $2
        run $1 $m $m $m
    done
    echo "Rectangle Matrices"
    for m in "${size_vals[@]}"; do
        for n in "${size_vals[@]}"; do
            for k in "${k_vals[@]}"; do
                if [[ m -ne n ]] && [[ m -ne n ]] && [[ n -ne k ]]; then 
                    echo -e "\tGenerating Matrix (m: ${m}, n: ${n}, k: ${k})"
                    ./build/scpa-matrix-generator $m $n $k $2
                    run $1 $m $n $k
                fi
            done
        done
    done
}

main $1 $2
