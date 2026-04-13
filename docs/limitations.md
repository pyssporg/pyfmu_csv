# Limitations

## Current Functional Limits

- Only CSV header parsing is implemented; signal samples are not consumed yet.
- The first CSV column must be named `time`.
- All exported signals are currently emitted as FMI `Real` outputs.
- Value references are assigned sequentially from the CSV column order.

## FMU Limits

- The generated FMU is packaged for inspection, not execution.
- The reusable runtime binary is not yet placed in `binaries/<platform>/`.
- FMI C API functions are not implemented yet.
- OMSimulator or FMPy simulation execution is not supported yet.

## Scope Limits

- FMI 2.0 Model Exchange is not targeted.
- FMI 3.0 is not targeted.
- Interpolation, events, and runtime error policy are not implemented yet.
