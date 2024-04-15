#!/bin/bash

# Remove temp folders
rm -rf build .cache .venv output
# Remove valgrind output
rm valgrind-out* callgrind.*
