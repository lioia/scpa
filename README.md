# scpa

## Building

```bash
module load gnu mpi cuda # only on department server
cmake -B build
cmake --build build
```

## Running

**MPI**

```bash
mpirun -n <n_processes> ./build/mpi/scpa-mpi
```

**CUDA**

```cuda
./build/cuda/scpa-cuda
```
