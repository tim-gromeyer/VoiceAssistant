# Commands

In this folder, you can create subfolders named after the language codes of the speech-to-text (STT) engine used by the voice assistant. 

When the voice assistant is launched, it will loop over all system languages and check if the STT engine has a language model for the system language. If a language model is found, the voice assistant will try to load the commands from the folder with the corresponding language code. For example, if the system language is English and the STT engine uses the language code `en-US`, the voice assistant will look for commands in the folder `commands/en-US`.

The commands for each language should be defined in a file named `default.json`. In this file, you must define an array of `commands`.

In addition to the `commands` array, the `default.json` file may also include the following fields:

| Field Name | Type             | Description                                                                                                                       |
|------------|------------------|-----------------------------------------------------------------------------------------------------------------------------------|
| `funcName` | String           | The name of the function to be called when the command is detected. Example: `say` calls `MainWindow::say`                        |
| `responses`| Array of strings | An array of responses to be randomly selected from when the command is detected.                                                  |
| `program`  | String           | The path to a program or script to be executed when the command is detected.                                                      |
| `args`     | Array of strings | An array of arguments to be passed to the program defined in the `program` field. `${TEXT}` will be replaced with the voice input |
| `sound`    | String           | The path or url to a sound file to be played when the command is detected. Volume can be controlled via voice/GUI.                |

Note that not all fields are required, and some fields may not be applicable to certain commands. 

The volume information in the previous version is not accurate and has been removed.

Please refer to the [project's main README](../README.md#add-commands) file for more information on customization and adding commands. 
