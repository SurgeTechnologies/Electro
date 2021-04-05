# Electro

This repository contains the source code of Electro Engine  

## Cooking Electro

- Building on Windows is very simple! You need to clone the repository first! Its upto you how you clone the repository, you can clone it via CLI(Command Line Interface) or if you are very dumb(like me) and love GUI(like me), you can clone the repository via Gitkraken or Github Desktop. I personally prefer cloning it via Gitkraken, but other ways will work absolutely fine!  

- So now **you should** have cloned the repository in your local system(A Computer)! If yes, you can Safely proceed to the next steps!

- Make a folder named **build** in the root directory(the directory where this README.md exists)

- Enter the newly created fresh directory

- Open a command prompt in that directory. Run CMake via `cmake ..` in that directory

- Okay, if you are a PRO you should have succeeded and get the `Electro.sln` in the `build` directory. Simply open the `Electro.sln` and hit the play button in Visual Studio!

- NOTE: You can use other CMake commands to generate for other IDEs, Visual Studio is just an example. Be sure to set the working directory to the root directory in other IDEs manually before hitting the play button!(Who uses another IDE when Visual Studio exists)

- If you are Dumb(like me) and you got `cmake is not recognized as an internal or external command` after typing `cmake ..` that means you don't have cmake installed (or you have indtalled it and forgot to add it in the environment variables). The solution to this: Click [this](https://github.com/Kitware/CMake/releases/download/v3.20.0/cmake-3.20.0-windows-x86_64.msi) link to install Cmake, be sure to delete Cmake if you have installed it already. **When the installer runs, check the `Add to PATH` option! Now you have Cmake. Re-run `cmake ..` in the build directory and you should be fine**

Pull requests are very WELCOME, we badly need a decent logo for the engine(Current logo, which is not posted here. is a piece trash which I made in 10 mins in Photoshop)  

### Current Features

The screenshots of the engine and its rendering capabilities will be given here soon. {I am too lazy to take screenshots of them(Dumb me)}. I promise I will write them here after some days  

Anything you imagine (I am too lazy to write them down here, again, I promise I will write them here after some days)

Finally, Sorry to all Linux and Mac users, we don't support this platforms yet :(  

[Trello](https://trello.com/b/XWRZuD3R)