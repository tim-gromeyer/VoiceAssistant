# Commands

In this folder you should create subfolders named exactly like the model folders.

After the model is loaded, it loads commands from the folder named like the model folder.  
It will look for a `default.json` file.  
You have to define `funcName` and `commands`.
After one of the commands is detected, it calls the function `MainWindow::funcName`, where funcName is the name of the function.
