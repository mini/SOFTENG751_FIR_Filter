# SE751 Project #1: Convolution in OpenCL
Various implementations of an FIR filter combined into a single package.

## Usage

`./firfilter <filter> <input> <weights> [<output> [<expected>]]`

filter = one of {btd, ocltd, oclfft, oclctd, oclcfft}  
input/weights = a file containing floats  
output = where the output should be saved  
expected = an existing file to compare output against, output must be defined  
  
All file arguments support reading/writing to binary or text formats
 
 ---
`./random-wave-gen <output> <random points> [<interpolated points>]`

output = output file

random points = integer, how many random numbers to generate

interpolated points = integer, default = 10, how many points to sample between two random points (smoother waveform)

---

`./manual <output>`

ouput = output file 

Keep typing numbers in, one per line, submit empty line to finish

---

`python benchmark.py`

No arguments, check the config section in the script

## Building

Main project dependencies:
- OpenCL SDK (provided by Intel/AMD/NVIDIA)
- [clFFT](https://github.com/clMathLibraries/clFFT/)

The individual files in `./generators/` are standalone tools. No dependancies there.
