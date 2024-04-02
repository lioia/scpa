#!/bin/bash
echo "Creating Python Virtual Environment"
python3 -m venv .venv > /dev/null

echo "Activating Python Virtual Environment"
source .venv/bin/activate > /dev/null

echo "Installing packages"
pip install -r requirements.txt > /dev/null

echo "Creating graphs for MPI"
python3 graphs/main.py ./output/mpi.csv mpi > /dev/null
echo "Creating graphs for OpenMP"
python3 graphs/main.py ./output/mpi-omp.csv mpi-omp > /dev/null
echo "Creating graphs for MPI+OpenMP"
python3 graphs/main.py ./output/omp.csv omp > /dev/null

echo "Cleaning up"
deactivate > /dev/null
rm -rf .venv > /dev/null
