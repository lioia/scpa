#!/bin/bash

if ! [ -f ./build/scpa-matrix-generator ] || ! [ -f ./build/mpi/scpa-mpi ] || ! [ -f ./build/mpi/scpa-mpi-omp ] || ! [ -f ./build/omp/scpa-omp ]; then
    echo "The project was not built correctly. Building now..."
    cmake -B build && cmake --build build
fi

k_vals=(32 64 128 156)
size_vals=(32 64 128 256 512 1024 2048 4096 8192)
p_vals=(2 4 8 16 24 32)

# Syntax: mpi_run <m> <n> <k>
mpi_run() {
    for p in "${p_vals[@]}"; do
        echo -e "\t\tNumber of Processes: $p"
        mpirun -n $p ./build/mpi/scpa-mpi $1 $2 $3
    done
}

# Syntax: mpi_omp_run <m> <n> <k>
mpi_omp_run() {
    for p in "${p_vals[@]}"; do
        for t in "${p_vals[@]}"; do
            echo -e "\t\tNumber of Processes: $p; Number of Threads: $t"
            mpirun -n $p ./build/mpi/scpa-mpi-omp $1 $2 $3 $t
        done
    done
}

# Syntax: omp_run <m> <n> <k>
omp_run() {
    for t in "${p_vals[@]}"; do
        echo -e "\t\tNumber of Threads: $t"
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

# Syntax: main[ <type> <gen_type>]
main() {
    echo "Square Matrices"
    for m in "${size_vals[@]}"; do
        echo -e "\tRunning size $m $m $m"
        echo -e "\t\tGenerating Matrix"
        ./build/scpa-matrix-generator $m $m $m $2
        run $1 $m $m $m
    done
    echo "Rectangle Matrices"
    for m in "${size_vals[@]}"; do
        for n in "${size_vals[@]}"; do
            for k in "${k_vals[@]}"; do
                if [[ m -ne n ]] && [[ m -ne n ]] && [[ n -ne k ]]; then 
                    echo -e "\tRunning size $m $n $k"
                    echo -e "\t\tGenerating Matrix"
                    ./build/scpa-matrix-generator $m $n $k $2
                    run $1 $m $n $k
                fi
            done
        done
    done
}

main $1 $2
