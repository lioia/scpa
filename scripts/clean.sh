#!/bin/bash

# Remove temp folders
rm -rf build .cache .venv 2&> /dev/null
find output ! -name "*.csv" -type f -delete -print

# Remove valgrind output
rm valgrind-out* callgrind.* 2&> /dev/null
