#!/bin/bash
if [ -f "/bf16.c" ]; then
    echo "BF detected in root"
else
    echo -e "Downloading BF... /n/n"
    wget https://github.com/p2r3/bf16/archive/refs/heads/main.zip
    mkdir bf16
    cd bf16
    unzip bf16-main.zip
fi

package_name="build-essential"

if apt list --installed 2>/dev/null | grep -q "^${package_name}/"; then
    echo "Package GCC detected"
    sleep 6
    
else
    echo "Package GCC is not detected, Installing now"
    sudo apt install -y -q --show-progress build-essential
fi

echo "Setting up BF Runtime..."
gcc bf16.c -o bf16 -lSDL2 -lm
gcc bf16_grayscale.c -o bf16_grayscale -lSDL2 -lm