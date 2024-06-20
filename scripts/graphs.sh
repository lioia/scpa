#!/bin/bash

if ! [ -d .venv] ; then
    echo "Creating Python Virtual Environment"
    python3 -m venv .venv > /dev/null

    echo "Activating Python Virtual Environment"
    source .venv/bin/activate > /dev/null

    echo "Installing packages"
    pip install -r requirements.txt > /dev/null
fi

echo "Creating graphs"
python3 src/graphs/main.py

echo "Cleaning up"
deactivate > /dev/null
