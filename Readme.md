BLRevive Module Skeleton providing quick start environment setup to develop custom BLRevive Modules.

## features

- modern preconfigured cmake script for windows environment
- basic skeleton for BLRevive Module
- uses [CPM]() for easy package management
  
## prerequisites

In order to succesfully compile and run the module you need the following applications:

- Visual Studio 2019 (with C++/MSVC)
- patched Blacklight: Retribution installation

## setup

In order to setup a new BLRevive Module project follow these steps:

1. clone this repository 
   > `git clone https://gitlab.com/blrevive/modules/skeleton.git <module-name>`
2. run setup script
   > `./setup.ps1 <ModuleName>`
3. remove setup script
   > `rm setup.ps1`

*Don't forget to initialize your git repository before making changes!*

## building
> Because Blacklight: Retribution itself is compiled for Win32, the modules target that platform too and only that one. It may be possible to compile the modules for another platform but there is no point in doing so.

1. change `GIT_TAG main` and `set(PROXY_VERSION "0.0.1" ...` in cmake/packages.cmake to the version of Proxy you are targetting
2. start VS developer shell inside project
3. run `cmake -A Win32 -B build/` to generate build files for VS 2019 / Win32
4. configure debugging feature with cmake options (see below)

### cmake options

| option | description | default |
|---|---|---|
| `BLR_INSTALL_DIR` | absolute path to Blacklight install directory | `C:\\Program Files (x86)\\Steam\\steamapps\\common\\blacklightretribution` |
| `BLR_EXECUTABLE` | filename of BLR applicaiton | `BLR.exe` |
| `BLR_SERVER_CLI` | command line params passed to server when debugging | `server HeloDeck` |
| `BLR_CLIENT_CLI` | command line params passed to client when debugging | `127.0.0.1?Name=superewald` |


## compiling

Use `build/<ModuleName>.sln` to compile the module with VS 2019.
The solution offers the following targets:

| target | description |
|---|---|
| Client | compile debug version and run BLR client |
| Server | compile debug version and run BLR server |
| Release | compile release version |
