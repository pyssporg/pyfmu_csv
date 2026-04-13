# Limitations

## Current Functional Limits

- Only CSV header parsing is implemented; signal samples are not consumed yet.
- The first CSV column must be named `time`.
- All exported signals are currently emitted as FMI `Real` outputs.
- Value references are assigned sequentially from the CSV column order.

## FMU Limits

- The generated FMU does not yet automatically embed the compiled runtime under `binaries/<platform>/`.
- The implemented FMI runtime currently supports the CSV path string parameter and real outputs only.
- OMSimulator or FMPy execution from the generated Python-built `.fmu` is not wired up yet because binary packaging is still pending.

## Scope Limits

- FMI 2.0 Model Exchange is not targeted.
- FMI 3.0 is not targeted.
- Interpolation, events, and runtime error policy are not implemented yet.
