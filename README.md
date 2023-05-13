[![Build Status](https://github.com/tim-gromeyer/VoiceAssistant/actions/workflows/build.yml/badge.svg)](https://github.com/tim-gromeyer/VoiceAssistant/actions/workflows/build.yml)
[![Translated using Weblate](https://img.shields.io/badge/Translated%20using%20Weblate-✅-green)](https://weblate.org/en/)

# Pre-Alpha!

# VoiceAssistant

VoiceAssistant is a resource-efficient and customizable voice assistant that is still in the early stages of development but already functional.

Privacy is our top priority, and VoiceAssistant ensures that all operations are performed offline on your local device, keeping your data secure and protected.

## Goal

The main goal of VoiceAssistant is to provide a fully customizable and extendable voice assistant that can be controlled entirely using voice commands.

## Features

- Fully customizable and extendable (I'm still working on this)
- Offline functionality for enhanced privacy and security
- Voice recognition for executing predefined commands
- Privacy-first approach with everything offline and local on the device

## Supported Platforms

The project has been tested on x86_64 Linux, but it should theoretically work on the following platforms:

- [x] Aarch64/Arm64 Linux/Android/Raspberry Pi
- [x] Armv7l Linux/Android
- [x] Riscv64 Linux
- [x] x86 Linux/Android
- [x] x86_64 Linux/Android
- [x] 32-bit Windows (not suggested and untested)
- [x] 64-bit Windows
- [x] macOS (Intel)
- [ ] macOS (M1)
- [ ] iOS
- [ ] WebAssembly

Note: The Android build works and starts, but I can't load the speech-to-text plugin.

## To-Do's

- [ ] Full customizability
- [ ] Implement Whisper.cpp backend (this will add WebAssembly, iOS, and M1 macOS support)

## Customization

**In the near future, I'll add settings to the app!**

To customize the commands, edit the `commands/<your_language>/default.json` file.

There are some default commands. See [Add Commands](#add-commands).

## Add Commands

To add your command, edit the `commands/<your_language>/default.json` file. The supported values are:

| Name        | Type             | Description                                                 | Notes                                             |
|-------------|------------------|-------------------------------------------------------------|---------------------------------------------------|
| `commands`  | Array of strings | All commands that the voice assistant should react to       | Required                                          |
| `funcName`  | String           | The name of the function to call (`MainWindow::<funcName>`) |                                                   |
| `responses` | Array of strings | It selects a random response from the array                 | Use `[wait <number of ms>]` to wait between words |
| `program`   | String           | A program to execute                                        |                                                   |
| `args`      | Array of strings | Arguments passed to `program`                               | `${TEXT}` will be replaced with the voice input   |
| `sound`     | String           | Path to local or remote file that gets played               | Volume can be controlled via voice/GUI            |

## Build

See the build section in [INSTALL.md](INSTALL.md) ([direct link](INSTALL.md#build)).

## Translation Status

You can check the translation status of VoiceAssistant on Weblate (click the image below):

[![Translation Status](https://hosted.weblate.org/widgets/voiceassistant/-/multi-auto.svg)](https://hosted.weblate.org/engage/voiceassistant/)

## Credits

VoiceAssistant uses the following open-source projects:

- [Vosk](https://github.com/alphacep/vosk-api) for voice recognition
- [11Zip](https://github.com/Sygmei/11Zip) for unzipping voice models
- [JokeAPI](https://jokeapi.dev) for telling jokes

Thank you to these amazing projects and their contributors for making VoiceAssistant possible!
