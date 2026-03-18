#!/bin/bash
# ONVIF Compliance Tester - Wrapper Script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_PATH="$SCRIPT_DIR/onvif-env"
PYTHON="$VENV_PATH/bin/python"

if [ ! -f "$PYTHON" ]; then
    echo "Error: Python virtual environment not found at $VENV_PATH"
    echo "Please run: python3 -m venv $VENV_PATH && $VENV_PATH/bin/pip install onvif-zeep"
    exit 1
fi

# Run the ONVIF tester with all arguments
$PYTHON "$SCRIPT_DIR/onvif_tester.py" "$@"
