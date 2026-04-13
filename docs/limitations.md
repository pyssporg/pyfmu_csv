# Limitations

## Current Functional Limits

- The first CSV column must be named `time`.
- Output types are declared through CSV header annotations, with `Real` fallback.
- Supported output types are `Real`, `Integer`, `Boolean`, and `String`.
- Value references are assigned sequentially from the CSV column order.
- `Real` outputs are linearly interpolated.
- `Integer`, `Boolean`, and `String` outputs are piecewise constant.

## FMU Limits

- The implemented FMI runtime currently supports the `csv_path` string parameter plus typed outputs only.
- Model Exchange is not implemented.
- FMI state serialization and advanced status/query APIs are not implemented.
- Engine validation is currently centered on FMPy; OMSimulator coverage is still pending.
- Typed inputs and additional non-`csv_path` parameters are not implemented.

## Scope Limits

- FMI 2.0 Model Exchange is not targeted.
- FMI 3.0 is not targeted.
- Interpolation, events, and runtime error policy are not implemented yet.
