# Pre-Alpha!

Currently, only basics work. See [`Add Commands`](#add-commands) for more.

# VoiceAssistant

Resource-efficient voice assistant that is still in the early stages of development.

## Table of Contents

- [Customization](#customization)
- [Add commands](#add-commands)
- [Credits](#credits)

## Customization

To customize the commands edit following file: `commands/<your_language>/default.json`.  
There are some default commands. More customization and documentation coming soon!

## Add commands

To add your own command edit this file: `commands/<your_language>/default.json`
Supported values:

| Name        | Type             | Description                                                |
|-------------|------------------|------------------------------------------------------------|
| `commands`  | Array of strings | Required. All commands it reacts to                        |
| `funcName`  | String           | The name of the function to call(`MainWindow::<funcName>`) |
| `responses` | Array of strings | It selects a random response from the Array                |
| `program`   | String           | A program to execute                                       |
| `args`      | Array of strings | Arguments passed to the executable `program`               |
| `sound`     | String           | Path to local or remote file which gets played             |


## Credits

| Name                                         | License                                                                | What is's used for     |
|----------------------------------------------|------------------------------------------------------------------------|------------------------|
| [Vosk](https://github.com/alphacep/vosk-api) | [Apache 2.0](https://github.com/alphacep/vosk-api/blob/master/COPYING) | Voice recognition      |
| [11Zip](https://github.com/Sygmei/11Zip)     | [MIT](https://github.com/Sygmei/11Zip/blob/master/LICENSE)             | Unzipping voice models |
| [JokeAPI](https://jokeapi.dev)               | [MIT](https://github.com/Sv443/JokeAPI/blob/master/LICENSE.txt)        | Telling jokes          |

