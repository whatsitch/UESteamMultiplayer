# UESteamMultiplayer

# Work In Progress!

# Guide

## Enabled Plugins

- Online Subsystem
- Online Subsystem NULL
- Online Subsystem Utils
- Online Subsystem Steam
- Steam Shared Module
- Steam Sockets

## DefaultEngine.ini configuration

````ini
[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SteamSockets.SteamSocketsNetDriver",DriverClassNameFallback="/Script/SteamSockets.SteamNetSocketsNetDriver")

[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480
bUseSteamNetworking=true
bAllowP2PPacketRelay=true

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"
````
## MSVC Version
- MSVC v143 - VS 2022 c++ x64/x86 build tools (v14.44-17.14)

# TODO
- SteamMultiplayer.Build.cs?

- SteamMultiplayerGameInstance


## Main Menu

### C++
- Menu/GM_Menu
- Menu/PC_Menu

### Blueprint
- Menu/BP_GM_Menu
- Menu/BP_PC_Menu
  - UI, select Main Menu Widget, Multiplayer Menu Widget
- 
