# Chocolate Source :chocolate_bar:
Custom Source Engine branch based on Source Engine 2007 currently available on Windows in x86. Forked from [SE2007Tweaked](https://gitlab.com/Juesto/SE2007_src-tweaked) and [Quiver](https://github.com/quiverteam/Engine).

# :hammer: Building

1. Download and install [Visual Studio 2013](https://go.microsoft.com/fwlink/?LinkId=532495&clcid=0x409).
2. Download and install the [Multibyte MFC Library](https://www.microsoft.com/en-gb/download/details.aspx?id=40770).
3. Run first `createlibfiles.bat` and build it this contains all of the source engine libs need.

* For SDKLess builds (like old sdk base 2007):
  * Run `createsdklessbuild.bat` and build it.

* For Full SDK build:
  * Run `createworkingprojects.bat` and build it.

 :pushpin: **Note:** Currently the engine is not able to run/build in Linux/x64

## :heavy_check_mark: Features

* 32 slots for all games
* 10 weapon slots. (no more unused slot commands)
* built-in MP3 Player enabled
* Scenes.image enforcement removed (able to load both from image and VCDs)
* Remove bug report.
* Multi-instance support (Aka - running multiple engine windows at the same time)
* Mod base GameUI.dll like leaked engine from 2003.
* Remove unfinished dx10 api.
* Remove Dx7 and 6 Support.
* Protocol Version is 15.
* L4D2 "Subtitles" system ( now the engine and game reads subtitles_%lang% instead of the old way )
* Valve_Avi **=>** VideoServices
* Remove P4 Support
* Alpha Chanel Support For Bink Files

## :bookmark_tabs: Notes

* The engine is on NO_STEAM, it means that steam is not need for running the engine (Temp until the steam api is update)

## :mega: Contributing

Feel free to contribute to the project with pull requests. They will be reviewed and merged as fast as possible.
