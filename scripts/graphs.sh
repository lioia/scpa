#!/bin/bash
echo "Creating Python Virtual Environment"
python3 -m venv .venv > /dev/null

echo "Activating Python Virtual Environment"
source .venv/bin/activate > /dev/null

echo "Installing packages"
pip install -r requirements.txt > /dev/null

echo "Creating graphs"
python3 src/graphs/main.py ./output

echo "Cleaning up"
deactivate > /dev/null
rm -rf .venv > /dev/null
