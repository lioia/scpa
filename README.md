# scpa

## Building

```bash
module load gnu mpich cuda # or module load load in local development
cmake -B build
cmake --build build
```

## Running

**MPI**

```bash
mpirun -n <p> ./build/mpi/scpa-mpi <m> <n> <k> <initial_seed> <type>
```

**CUDA**

```cuda
./build/cuda/scpa-cuda
```
