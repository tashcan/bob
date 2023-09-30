# Contributing

## Building

First clone and initialise the repository:

```bash
git clone https://github.com/tashcan/bob.git
cd bob
git submodules update --recursive --init
```

This will download three submodules and further submodules that they have, so this will take some time to complete.

## Installing

Please note that when this project compiles, it will create a DLL called `stfc-communty-patch.dll`.  This 
file must be either copied to the `C:\Games\Star Trek Fleet Command\Star Trek Fleet Command\default\game` 
folder as `version.dll` or create a symbolic link to the file using an elevated (administrator) command 
prompt:

```console
cd C:\Games\Star Trek Fleet Command\Star Trek Fleet Command\default\game
mklink [output folder]\stfc-communty-patch.dll version.dll
```

If you do link the file, please note you will need to close the game to recompile.

### Visual Studio

#### SDK Warning

You may see the following error, and this normally occurs if your SDK is below the minimum required.

```
The <experimental/coroutine> and <experimental/resumable> headers are only supported with /await and implement pre-C++20 coroutine support. Use <coroutine> for standard C++20 coroutines
```

If you have Visual Studio 2022, make sure to have the Windows 11 SDK selected as a minimum, even if you 
are running the Visual Studio IDE on Windows 10.  The Windows 11 SDK will also create targets that work 
for Windows 10.

You can do this via the Visual Studio Installer.  The Visual Studio Installer will show any other versions 
of Visual Studio, so please make sure to use the `Modify` button on the correct one.  You can launch the 
Visual Studio Installer from within Visual Studio 2022 via the `Tools -> Get Tools and Features` menu 
option, or through `Settings -> Apps -> Visual Studio -> Modify`.  

Once you are modifying the correct Visual Studio 2022 instance, select the `Individual Components` tab and 
click into the Filter textbox.  Enter the type `SDK` which should provided a minimal filtered list of SDK's 
available.  You should see Windows 11 SDK listed and has a tick.  If not, please click the tick box and then 
apply the changes to have the SDK downloaded and installed.  This will take around 2-3GB of space.

#### Configure and building the project

Create or enter the `.vs/` folder and place the following in `launch.vs.json`:

```json
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "exe": "C:\\Games\\Star Trek Fleet Command\\Star Trek Fleet Command\\default\\game\\prime.exe",
      "project": "CMakeLists.txt",
      "projectTarget": "stfc-communty-patch.dll",
      "name": "Prime",
      "cwd": "C:\\Games\\Star Trek Fleet Command\\Star Trek Fleet Command\\default\\game"
    }
  ]
}
```

Once they are installed, either standalone or via Visual Studio, you can open the `bob` folder inside 
Visual Studio or by right clicking in a Windows Explorer via and selecting `Open with Visual Studio`.  
When it first opens, it should automatically start a build to configure the project.  You can 
reconfigure the project by right clicking on the `CMakeLists.txt` file and selecting 
`Configure STFC Community Patch`.  

Once the project configuration has finished, you can build the project by pressing `F6` or right clicking 
on the `CMakeLists.txt` file and selecting `Build`.  

**IMPORTANT**: To reset the build, you can remove the `out/` folder and all items beneath it.  Visual 
Studio will then rebuild the project.

**IMPORTANT**: To fully reset the project, also remove the `.vs/` folder.  If you do this, please remember 
to recreate the `.vs/launch.vs.json` file.

### Visual Studio Code

If you want to use Visual Studio Code, you may still need to make sure the various SDK's and MSVC runtimes 
are available.

Once they are installed, either standalone or via Visual Studio, you can open the `bob` folder inside 
Visual Studio or by right clicking in a Windows Explorer via and selecting `Open with Visual Studio Code`.  
When it first opens, it should ask you to install the CMake extensions bundle.  Once the extensions are 
installed, you can build the project by pressing `F7` or right clicking on the `CMakeLists.txt` file and
selecting `Build All Projects`.  This will ask you to configure the project and pick various items to use
in the build on the first build only.

**IMPORTANT**: To reset the build, you can remove the `build/` folder and all items beneath it.  Visual 
Studio will then rebuild the project.

### Command Line

If you do not have Visual Studio Code, this project uses CMake, so the simplest way to build it on Windows:

```ps1
mkdir build
cd build
cmake ../
cmake --build .
```
