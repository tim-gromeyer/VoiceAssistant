#!/bin/bash
# Script for creating icons in different sizes and formats

# Check if Inkscape is installed, otherwise use Imagemagick
if command -v inkscape &> /dev/null
then
    CONVERT_COMMAND="inkscape -e"
    echo "Inkscape found. Using Inkscape for PNG conversion."
else
    if ! command -v convert &> /dev/null
    then
        echo "Imagemagick is not installed. Please install it to proceed."
        exit
    fi
    CONVERT_COMMAND="convert -background none -density 90"
    echo "Inkscape not found. Using Imagemagick for PNG conversion."
fi

# Create PNG files in different sizes
for x in 16 24 32 48 64 128 256 512 1024; do
    ${CONVERT_COMMAND} -resize ${x}x${x} Icon.svg "${x}-apps-VoiceAssistant.png" &&
    echo "PNG file ${x}-apps-VoiceAssistant.png was created successfully."
done

# Convert SVG file to ICO
if ! command -v convert &> /dev/null
then
    echo "Imagemagick is not installed. Please install it to proceed."
    exit
fi
convert -background none Icon.svg -define icon:auto-resize Icon.ico &&
echo "ICO file Icon.ico was created successfully."

# Convert PNG file to ICNS
if ! command -v png2icns &> /dev/null
then
    echo "png2icns is not installed. Please install it to proceed."
    exit
fi
if png2icns Icon.icns 1024-apps-VoiceAssistant.png &> /dev/null
then
    echo "ICNS file Icon.icns was created successfully."
else
    echo "Error converting PNG file to ICNS."
fi

