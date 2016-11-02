#!/bin/bash

python /opt/pyinstaller1.5/Build.py mutinfo_ex.spec
cp dist/mutinfo_ex ../../benchmark_runner/binary_tools/
