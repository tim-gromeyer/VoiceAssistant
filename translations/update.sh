#!/bin/bash

# Check if lupdate is installed
if ! command -v lupdate &> /dev/null; then
    echo "lupdate is not installed. Please install it and try again."
    exit 1
fi

# Navigate to the parent directory of the script
cd "$(dirname "$0")/../"

# Run lupdate for each translation file
for ts_file in translations/VoiceAssistant_*.ts; do
    lupdate src/*.cpp "$(find plugins/* -name '*.cpp')" plugins/*.h "$(find speechtotext/* -name '*.cpp')" ui/*.ui -ts "$ts_file" -noobsolete
done
