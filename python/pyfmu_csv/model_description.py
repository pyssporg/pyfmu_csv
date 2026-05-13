from __future__ import annotations

from typing import cast
from xml.etree.ElementTree import Element, SubElement, tostring

from .model import CsvModelDescription

_DEFAULT_CSV_PATH_START = object()


def build_model_description_xml(
    model: CsvModelDescription,
    csv_path_start: object = _DEFAULT_CSV_PATH_START,
) -> str:
    if csv_path_start is _DEFAULT_CSV_PATH_START:
        csv_path_start = model.packaged_csv_path

    root = Element(
        "fmiModelDescription",
        {
            "fmiVersion": "2.0",
            "modelName": model.model_name,
            "guid": model.guid,
            "generationTool": "pyfmu-csv",
            "variableNamingConvention": "structured",
        },
    )
    SubElement(
        root,
        "CoSimulation",
        {
            "modelIdentifier": model.model_identifier,
            "canHandleVariableCommunicationStepSize": "true",
        },
    )

    model_variables = SubElement(root, "ModelVariables")
    csv_parameter = SubElement(
        model_variables,
        "ScalarVariable",
        {
            "name": model.csv_path_parameter,
            "valueReference": "0",
            "variability": "tunable",
            "causality": "parameter",
        },
    )
    if csv_path_start is not None:
        SubElement(csv_parameter, "String", {"start": cast(str, csv_path_start)})

    for signal in model.outputs:
        variability = "continuous" if signal.signal_type.value == "Real" else "discrete"
        variable = SubElement(
            model_variables,
            "ScalarVariable",
            {
                "name": signal.name,
                "valueReference": str(signal.value_reference),
                "variability": variability,
                "causality": "output",
            },
        )
        SubElement(variable, signal.signal_type.value)

    model_structure = SubElement(root, "ModelStructure")
    outputs = SubElement(model_structure, "Outputs")
    for output_index, _signal in enumerate(model.outputs, start=2):
        SubElement(outputs, "Unknown", {"index": str(output_index)})

    xml_text = tostring(root, encoding="unicode")
    return '<?xml version="1.0" encoding="UTF-8"?>\n' + xml_text
