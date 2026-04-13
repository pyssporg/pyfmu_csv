# Limitations

## Current Functional Limits

- Only CSV header parsing is implemented; signal samples are not consumed yet.
- The first CSV column must be named `time`.
- All exported signals are currently emitted as FMI `Real` outputs.
- Value references are assigned sequentially from the CSV column order.

## FMU Limits

- The implemented FMI runtime currently supports the CSV path string parameter and real outputs only.
- Model Exchange is not implemented.
- FMI state serialization and advanced status/query APIs are not implemented.
- Engine validation is currently centered on FMPy; OMSimulator coverage is still pending.

## Scope Limits

- FMI 2.0 Model Exchange is not targeted.
- FMI 3.0 is not targeted.
- Interpolation, events, and runtime error policy are not implemented yet.
