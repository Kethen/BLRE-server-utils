BLRevive module for changing server settings

## features

- override some server settings even the ones that could be overriden by playlist, see below for the currently supported values
- change the bot name pool
- disable intermission idle kick

## WIP

- querying server status from http

## runtime prerequisites

- on windows, vcrun 2015 x86 has to be installed (vcrun 140) for Proxy.dll to load correctly, you can find it at https://www.microsoft.com/en-us/download/details.aspx?id=52685
- recent versions of wine should be able to run it without installing msvc, do `winetricks vcrun2015` if that's not the case

## install and usage

first make sure proxy.dll is ready and is capable of loading modules: https://gitlab.com/blrevive/tools/proxy

install `server-utils.dll` to `<blr>/Binaries/Win32/Modules/`

change `<blr>/FoxGame/Config/BLRevive/default.json` to load server-utils on server

for example:
```
{
  "Proxy": {
    "Server": {
      "Host": "127.0.0.1",
      "Port": "+1"
    },
    "Modules": {
      "Server": [ "server-utils" ],
      "Client": [  ]
    }
  }
}
```

after first run, `<blr>/FoxGame/Config/BLRevive/server_utils/server_config.json` is generated

## supported parameters

| path | description |
| --- | --- |
| properties/RandomBotNames/[0-18] | (string) change one of the 19 possible bot names |
| properties/GameRespawnTime | (float) seconds until a player can click respawn |
| properties/GameForceRespawnTime | (float) seconds until a player is automatically respawned |
| properties/GameSpectatorSwitchDelayTime | (float) seconds until a player can switch back from spectator |
| properties/NumEnemyVotesRequiredForKick | (int) number of enemy votes required to kick a player |
| properties/NumFriendlyVotesRequiredForKick | (int) number of friendly votes required to kick a player |
| properties/VoteKickBanSeconds | (int) seconds until a kicked player can re-join |
| properties/MaxIdleTime | (float) seconds until a player is considerd idling and kicked |
| properties/MinRequiredPlayersToStart | (int) minimum player required to start a match, setting it to 1 allows starting with a single player |
| properties/PlayerSearchTime | (int) seconds until a round start after properties/MinRequiredPlayersToStart is fulfilled, setting it too low might cause client side issues during map change |
| properties/TimeLimit | (int) minutes a round should last, only applies when playlist is used, non playlist should use the launch parameter instead |
| properties/GoalScore | (int) score required to win, could mean very different things in different modes, for example it means the individual score to win in DM, and number of kills in TDM |
| hacks/disableOnMatchIdle | (int) set to non 0 to disable intermission idle kick |

## building prerequisites

In order to succesfully compile and run the module you need the following applications:

- Visual Studio 2019/2022 (with C++/MSVC)
- patched Blacklight: Retribution installation

## building
> Because Blacklight: Retribution itself is compiled for Win32, the modules target that platform too and only that one. It may be possible to compile the modules for another platform but there is no point in doing so.

1. start VS developer shell inside project
2. run `cmake -A Win32 -B build/` to generate build files for VS 2019/2022 / Win32
3. configure debugging feature with cmake options (see below)

### cmake options

| option | description | default |
|---|---|---|
| `BLR_INSTALL_DIR` | absolute path to Blacklight install directory | `C:\\Program Files (x86)\\Steam\\steamapps\\common\\blacklightretribution` |
| `BLR_EXECUTABLE` | filename of BLR applicaiton | `BLR.exe` |
| `BLR_SERVER_CLI` | command line params passed to server when debugging | `server HeloDeck` |
| `BLR_CLIENT_CLI` | command line params passed to client when debugging | `127.0.0.1?Name=superewald` |


## compiling

Use `build/server-utils.sln` to compile the module with VS 2019/2022.
The solution offers the following targets:

| target | description |
|---|---|
| Client | compile debug version and run BLR client |
| Server | compile debug version and run BLR server |
| Release | compile release version |
