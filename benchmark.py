"""
 To run as-is, put weight.dat and any input.dat file in a folder called benchmark_files, as shown below

dir/
 - benchmark.py
 - benchmark_files
     - input1.dat
     - input2.dat
     - input3.dat
     - weights.dat
 - benchmark_TIMESTAMP.csv (will be generated)
"""

# -------- Config --------
# Change as required

executable = ".//x64//Release//SE751_filter.exe"
repeats = 3;
filters = ["btd", "ocltd", "oclctd", "oclfft", "oclcfft"];
benchmark_files = "./benchmark_files/"
weights = "weights.dat"
inputs = [] # insert files to override running all
# -----------

import os
import subprocess
from datetime import datetime

if not inputs:
    for entry in os.scandir(benchmark_files):
            if not entry.name.startswith('.') and not entry.name in weights and not entry.name in "output.dat":
                inputs.append(entry.name)

print(f"Benchmarking with:\n\tFilters: {filters}\n\tInputs: {benchmark_files}{inputs}\n\tWeights: {benchmark_files}{weights}\n")

with open(f"benchmark_{datetime.now().strftime('%H-%M-%S')}.csv", 'a') as csv:
    csv.write("filter,input,time\n")
    for filter in filters:
        for inputFile in inputs:
            for i in range(0, repeats):
                print(f"Run #{i+1} - {filter} on {inputFile}")

                p = subprocess.Popen([executable, filter, benchmark_files + inputFile, benchmark_files + weights, benchmark_files + "output.dat"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                stdout, stderr = p.communicate()

                if p.wait() != 0:
                    print("Last run had non-zero exit code")
                    exit()

                for line in stdout.splitlines():
                    line = line.decode('ascii')
                    if line.startswith("Time: "):
                        time = int(line[6:])
                        csv.write(f"{filter},{inputFile},{time}\n")