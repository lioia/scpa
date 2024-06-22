#!/bin/bash

# Remove temp folders
rm -rf build .cache .venv 2&> /dev/null

# Remove valgrind output
rm valgrind-out* callgrind.* 2&> /dev/null
