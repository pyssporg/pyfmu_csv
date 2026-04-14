# Testing

## Test Levels

### Runtime smoke tests

Covers the reusable C++ artifact:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

This currently covers:

- runtime-core CSV loading and interpolation
- FMI instantiate/setup/init/doStep/getReal/getInteger/getBoolean/getString/setString smoke behavior

### Python unit tests

Covers:

- CSV validation
- signal extraction
- type annotation parsing
- model description generation

```bash
pytest tests/unit
```

### Python integration tests

Covers:

- package skeleton creation
- `.fmu` archive assembly
- runtime binary placement into the FMU layout
- CLI command behavior

```bash
pytest tests/integration
```

### End-to-end validation

Covers:

- generation of a final `.fmu`
- structural inspection with FMPy
- execution of the generated FMU through FMPy using `csv_path`
- mixed-type output behavior for `Real`, `Integer`, and `Boolean`

```bash
pytest tests/e2e
```

### Full Python suite

```bash
pytest tests
```
