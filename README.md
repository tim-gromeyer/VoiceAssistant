# Pre-Alpha!

Currently, only voice recognition and some basic commands like "how late" or "repeat after me" work.

# VoiceAssistant

Resource-efficient voice assistant that is still in the early stages of development.

## Table of Contents


## Customization

To customize the commands edit following file: `commands/<your_language>/default.json`.  
There are some default commands. More customization and documentation coming soon!

## Add commands

To add your own command edit this file: `commands/<your_language>/default.json`
Supported values:

| Name      | Type             | Description                                                |
|-----------|------------------|------------------------------------------------------------|
| commands  | Array of strings | Required. All commands it reacts to                        |
| funcName  | String           | The name of the function to call(`MainWindow::<funcName>`) |
| responses | Array of strings | It selects a random response from the Array                |
| program   | String           | A program to execute                                       |
| args      | Array of strings | Arguments passed to the executable `program`               |


## Credits

The project uses [Vosk](https://github.com/alphacep/vosk-api) which is licensed under the [Apache License 2.0](https://github.com/alphacep/vosk-api/blob/master/COPYING).
