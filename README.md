[![Build Status](https://github.com/tim-gromeyer/VoiceAssistant/actions/workflows/build.yml/badge.svg)](https://github.com/tim-gromeyer/VoiceAssistant/actions/workflows/build.yml)
[![Translated using weblate](https://img.shields.io/badge/Translated%20using%20weblate-✅-green)](https://weblate.org/en/)


# Pre-Alpha!

# VoiceAssistant

Resource-efficient and customizable voice assistant that is still in the early stages of development but already functional.

## Table of Contents

- [Goal](#goal)
- [Supported Platforms](#supported-platforms)
- [To-Do's](#to-dos)
- [Customization](#customization)
- [Add Commands](#add-commands)
- [Build](#build)
- [Translation Status](#translation-status)
- [Credits](#credits)

## Goal

The main goal of VoiceAssistant is to provide a fully customizable and extendable voice assistant that can be controlled entirely using voice commands. 

Users can define the commands in the `commands/<your_language>/default.json` file. See [Add Commands](#add-commands).

## Supported Platforms

The project has been tested on x86_64 Linux, but it should theoretically work on the following platforms:

- [x] Aarch64/Arm64 Linux/Android/Raspberry Pi
- [x] Armv7l Linux/Android
- [x] Riscv64 Linux
- [x] x86 Linux/Android
- [x] x86_64 Linux/Android
- [x] 32-Bit Windows (not suggested and untested)
- [x] 64-Bit Windows
- [x] MacOS (Intel)
- [ ] MacOS (M1)
- [ ] iOS
- [ ] WebAssembly

## To-Do's

- [ ] Full customizability
- [ ] Cross-platform compatibility
- [ ] Implement Whisper.cpp backend (this will add WebAssembly, iOS and ARM MacOS support)

## Customization

**In the near future I'll add settings to the app!**  
*I'm struggling with a plugin system for the settings and the GUI.*

To customize the commands, edit the `commands/<your_language>/default.json` file. 

There are some default commands. See [Add Commands](#add-commands).

## Add Commands

To add your own command, edit the `commands/<your_language>/default.json` file. The supported values are:

| Name        | Type             | Description                                                 | Notes                                           |
|-------------|------------------|-------------------------------------------------------------|-------------------------------------------------|
| `commands`  | Array of strings | All commands that the voice assistant should react to       | Required                                        |
| `funcName`  | String           | The name of the function to call (`MainWindow::<funcName>`) |                                                 |
| `responses` | Array of strings | It selects a random response from the array                 |                                                 |
| `program`   | String           | A program to execute                                        |                                                 |
| `args`      | Array of strings | Arguments passed to `program`                               | `${TEXT}` will be replaced with the voice input |
| `sound`     | String           | Path to local or remote file that gets played               | Volume can be controlled via voice/GUI          |

## Build

See the build section in [INSTALL.md](INSTALL.md) ([direct link](INSTALL.md#build))

## Translation Status

You can check the translation status of VoiceAssistant on Weblate (click the image below):

[ ![Translation Status](https://hosted.weblate.org/widgets/voiceassistant/-/multi-auto.svg)](https://hosted.weblate.org/engage/voiceassistant/)


## Credits

| Name                                         | License                                                                | What it's used for      |
|----------------------------------------------|------------------------------------------------------------------------|------------------------|
| [Vosk](https://github.com/alphacep/vosk-api) | [Apache 2.0](https://github.com/alphacep/vosk-api/blob/master/COPYING) | Voice recognition      |
| [11Zip](https://github.com/Sygmei/11Zip)     | [MIT](https://github.com/Sygmei/11Zip/blob/master/LICENSE)             | Unzipping voice models |
| [JokeAPI](https://jokeapi.dev)               | [MIT](https://github.com/Sv443/JokeAPI/blob/master/LICENSE.txt)        | Telling jokes          |
