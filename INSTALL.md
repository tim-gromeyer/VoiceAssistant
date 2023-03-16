# Install

~Download VoiceAssistant from [here](https://github.com/tim-gromeyer/VoiceAssistant/releases/latest)~

Not available yet! Follow the build instruction down below and run `cmake --install .`

## Build

Needed Qt modules: base, multimedia, network and speech.

### Linux

1. Install nessasary Qt modules.<br><br>
On Ubuntu run this command: `apt-get install qtbase5-dev qtdeclarative5-dev qttools5-dev qtmultimedia5-dev libqt5texttospeech5-dev libqt5svg5-dev qt5ct`  
On Fedora run this command: `dnf install qt5-qtbase-devel qt5-qtdeclarative-devel qt5-qttools-devel qt5-qtmultimedia-devel qt5-qtspeech-devel qt5-qtsvg-devel`  
On Arch run this command: `pacman -S qt5-base qt5-declarative qt5-tools qt5-multimedia qt5-speech qt5-svg`

2. Clone the repo (requires git)<br><br>
    `git clone https://github.com/tim-gromeyer/VoiceAssistant --depth=1 --recurse-submodules`

3. Build it (requires CMake 3.13 or above)<br>

    ```bash
    cd VoiceAssistant
    mkdir -p build && cd build
    cmake ..
    cmake --build . -j3
    cmake --build . --target package
    ```

### MacOS

1. Install Qt by downloading the installer from the official website and following the installation instructions.

2. Install CMake 3.13 or above, either by downloading the installer from the official website or by using a package manager such as Homebrew.

3. Clone the repo (requires git)<br><br>
    `git clone https://github.com/tim-gromeyer/VoiceAssistant --depth=1 --recurse-submodules`

3. Build it (requires CMake 3.13 or above)<br>

    ```bash
    cd VoiceAssistant
    mkdir -p build && cd build
    cmake ..
    cmake --build . -j3
    cmake --build . --target package
    ```

### Windows

1. Install Visual Studio with the Desktop Development with C++ workload, and select the CMake tools for Windows component.

2. Install Qt by downloading the installer from the official website and following the installation instructions.

3. Clone the repo (requires git)  

    `git clone https://github.com/tim-gromeyer/VoiceAssistant --depth=1 --recurse-submodules`

4. Build it

    ```bash
    cd VoiceAssistant
    mkdir build && cd build
    cmake ..
    cmake --build . --config Release
    cmake --build . --target package
    ```
