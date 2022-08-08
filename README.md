
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/llde/xOBSE)](https://github.com/llde/xOBSE/releases/latest) [![Appveyor Build status](https://ci.appveyor.com/api/projects/status/github/llde/xOBSE?branch=master&svg=true)](https://ci.appveyor.com/project/llde/xOBSE/branch/master)

[![MSBuild](https://github.com/llde/xOBSE/actions/workflows/msbuild.yml/badge.svg?branch=master)](https://github.com/llde/xOBSE/actions/workflows/msbuild.yml)
[![Release](https://github.com/llde/xOBSE/actions/workflows/msbuild-relz.yml/badge.svg?branch=master)](https://github.com/llde/xOBSE/actions/workflows/msbuild-relz.yml)



This is a community maintained up-to-date fork of **Oblivion Script Extender** (OBSE). 

## Download

[Download the latest OBSE](https://github.com/llde/xOBSE/releases/latest)

Changelogs and all releases are provided in the [releases section](https://github.com/llde/xOBSE/releases).

## Installation

The instructions for installing and running OBSE differ based on whether you are using a retail or Steam version of the game.

IF YOU PURCHASED A RETAIL (NON-STEAM) VERSION OF OBLIVION:

1. Copy obse_1_2_416.dll, obse_editor_1_2.dll, obse_loader.exe and the Data folder to your Oblivion directory. This is usually in your Program Files folder, and should contain files called "Oblivion.exe" and "OblivionLauncher.exe".
3. Run oblivion by running obse_loader.exe from the Oblivion directory.

If you use a desktop shortcut to launch Oblivion normally, just update the shortcut to point to obse_loader.exe instead of oblivion.exe.

IF YOU ARE USING THE STEAM VERSION OF OBLIVION:

1. Copy obse_1_2_416.dll, obse_editor_1_2.dll, obse_steam_loader.dll and the Data folder  to your Oblivion directory. This is usually "C:\Program Files\Valve\Steam\SteamApps\common\oblivion".
2. Launch Oblivion via Steam or by running Oblivion.exe. OBSE will automatically be run along with Oblivion when launched. To disable this, rename or move obse_steam_loader.dll. You do not need to use obse_loader.exe unless you are running the editor.

IF USING STEAM PROTON ON LINUX (replace Point 2. from previous paragraph): 
1. Backup OblivionLauncher.exe and rename obse_loader.exe to OblivionLauncher.exe
2. Start the game from inside Steam 
Note this is needed only when using a Linux Steam and Proton. Using Windows Steam inside Wine should work out of the box,

IF YOU ARE USING MOD ORGANIZER 2:

Mod organizer user need to use special instruction to allow OBSE to function properly.
Follow the instruction on this page: https://github.com/ModOrganizer2/modorganizer/wiki/Running-Oblivion-OBSE-with-MO2

RUNNING TES:CONSTRUCTION SET WITH OBSE:

Scripts written with these new commands must be written via the TESConstructionSet launched with obse_loader. Open a command prompt window, navigate to your oblivion install folder, and type "obse_loader -editor". The normal editor can open plugins with these extended scripts fine, it just can't recompile them and will give errors if you try.

WARNING: It's unadvised to install Oblivion to *Program Files*

## Support
More information on running Oblivion and OBSE with Linux and Wine (or Proton) can be found on the [UESP Oblivion Linux](https://en.uesp.net/wiki/Oblivion:Linux) page. It should work out of the box for most people.

For support, contact us in the [xOBSE Discord server](https://discord.gg/efEzqa3). For bug reports and other problems, [create a new GitHub issue](https://github.com/llde/xOBSE/issues).

**As this is a community supported release don't report to the original programmers bugs that are present within this fork.**


## Description

**Oblivion Script Extender** (OBSE) is a modder's resource that expands the scripting capabilities of The Elder Scroll 4: Oblivion. It does so without modifying the executable files on disk, so there are no permanent side effects.


For documentation on how to write scripts utilizing OBSE , see [CS Wiki](https://cs.elderscrolls.com/index.php?title=Main_Page) and *obse_command_doc.html*  .

## Creating OBSE Plugins (for developers)
To create an OBSE plugin you may want to look the [example project](https://github.com/llde/xOBSE/tree/master/obse_plugin_example).
If the plugin you want to create define commands you need to request an opcode range to use. Reach us on the Discord server.

[Here](https://github.com/llde/xOBSE/wiki/Registered-Opcodes-for-OBSE-Plugins) there is a list of already allocated ranges. 

## FAQ / Troubleshooting

Can I update OBSE mid play-through?
- Yes, and you are encouraged to do so.

My antivirus says there's a virus in obse_loader.exe, what's that about?
- That's a false positive. We already contacted Microsoft about it but it will likely happen again in the future, so you need to add the file to exclusions of your AV.

Are old mods/plugins still compatible?
- Yes. We keep a focus on backwards compatibility with precedent mods and plugins. Despite that a beta may introduce unintendend errors that we will try to rectify as soon as possible.

Oblivion doesn't launch after running obse_loader.exe:

- Make sure you've copied the OBSE files to your oblivion folder. That folder should also contain oblivion.exe.
- Check the file obse_loader.log in your oblivion folder for errors.

obse_loader.log tells me it couldn't find a checksum:

- You may have a version of Oblivion that isn't supported. I have the english official patch v1.2.0.416.  Localized versions with different executables or different patches may not work, but many have been successful. If there's enough legitimate demand for it, I can add support for other versions in the future.
- Your Oblivion install may be corrupt. Hacks or no-cd patches may also change the checksum of the game, making it impossible to detect the installed version.
- In case this happen  [create a new GitHub issue](https://github.com/llde/xOBSE/issues)


The OBSE loader tells me I need to use the autopatcher:

- Go to to OBSE website (http://obse.silverlock.org) and download autopatcher, which will walk you through the update process. You will need the latest patch from Bethesda, as well as your original Oblivion DVD.

OBSE doesn't launch with the Direct2Drive version:

- The Direct2Drive version of the Oblivion executable has additional DRM applied, which would be illegal for us to bypass. We cannot support this version. If you own a retail version of Oblivion, please use the autopatcher to extract a usable executable.
- Note that if you are interested in buying a digitally distributed version of Oblivion, the GOG version and the Steam version are supported. (GOG Version preferred)

Crashes or other strange behavior:

- Let me know how you made it crash, and I'll see about fixing it.

Xbox 360 or PS3 version?

- Impossible.

Running OBSE and Oldblivion at the same time:

- Copy your oldblivion support files in to the Oblivion folder (oldblivion.dll, oldblivion.cfg, shaders.sdp), then run the loader with the -old command argument.

How do I change the script editor font?

- Hold F12 or F12 while opening the script editor. F12 will default to Lucida Console 9pt, and F11 will show a font picker dialog box.

Can I modify and release my own version of OBSE based on the released source code?

- This is highly discouraged. Each command must be assigned a unique and constant opcode from 0x1000-0x7FFF. Bethesda started adding commands at 0x1000, and OBSE started adding at 0x1400. If you add new commands yourself, they may conflict with later releases of OBSE. The suggested method for extending OBSE is to write a plugin. If this does not meet your needs, please email the contact addresses listed below. If you want a feature directly inside OBSE, or want to work improving OBSE, contribution can be done by forking the repository, and opening a Pull Request. Acceptance of the pull request is conditioned to a review.

How do I write a plugin for OBSE?

- Start with the obse_plugin_example project in the OBSE source distribution. Currently the documentation for the plugin API can be found in the source distribution under obse/obse/PluginAPI.h. Note that due to the opcode allocation issues discussed above, you will need to request an opcode range for your plugin by emailing the contact addresses at the bottom of the readme. Also note that plugins must have their source code available. The OBSE team has spent a very long time developing and documenting the interface to Oblivion's internals, so we request that plugin developers also share their findings with the public. This will also help us make sure that we keep Oblivion as stable as possible even when using third-party plugins.

How do I use OBSE with 3D Analyze?

- Run normal Oblivion with 3DA once, then quit. Set up any configuration options you need at this point. This should create a config_DX.ini file in the Oblivion folder. Then, copy dat3.000 from the 3DA folder in to the Oblivion folder as well, and rename it to d3d9.dll. This makes Oblivion use 3DA all the time, so now just use obse_loader.exe like normal. If you buy an actual video card and want to disable 3DA, delete the new d3d9.dll from the Oblivion folder.

I'm using the Steam version of Oblivion and OBSE doesn't seem to be working:

- Go to your Steam Settings page, pick the "In-game" tab, and make sure the "Enable Steam Community In-Game" box is checked.
- Sometimes even with the In Game overlay active the Steam version of Oblivion may fail to load the obse_steam_loader.dll. Starting steam as admin may resolve it. 
- A more reliable mechanism may be provided in the future


## Credits

OBSE was created and maintained by *Ian Patterson*, 
*Stephen Abel*, *Paul Connelly*, and *Madeesh Kannan*
(ianpatt, behippo, scruggsywuggsy the ferret, and shadeMe)

Additional contributions from *Timeslip*, *The J*, *DragoonWraith*, *SkyRanger-1*, *badhair*, *JRoush* and *kyoma*.

The home page for OBSE is http://obse.silverlock.org/


This version is maintained by *llde* and *shadeMe*

## Thanks
Thanks to the xNVSE people  *korri123* (aka *Kormákur*), *lStewieAl*, *jazzisparis*, *iranrmrf*, *maletsna* (*c6*), and *carxt* (aka *karut*)  that with their fork inspired me to take over this

Thanks to *Ershin* (Oblivion Display Tweaks, Loot Menu) that reported a serious bug in the Event Framework

Thanks to *Laulajatar* and *KatAwful* for documentation fixes and help with bug reports

Thanks to *EchoEclipseWolf* for decoding bits

Thanks to all people who reported bugs.

A special thanks to *ianpatt* that allowed us to mantain this fork.
