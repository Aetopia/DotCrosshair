# DotCrosshair
Adds in a dot crosshair overlay.

## Usage
> [!NOTE]
> Check it out in action: https://youtu.be/iM_xArwz_fw

1. Download the latest release from [GitHub Releases](https://github.com/Aetopia/DotCrosshair/releases/latest).
2. Extract the `.zip` file.
3. Run `DotCrosshair.exe`.
    This will spawn in a small square onto your display.
    This square will snap onto the foreground window and will be placed in the middle.

To adjust the size of the crosshair, do the following:
1. Create a new file called `DotCrosshair.ini` where `DotCrosshair.exe` and `DotCrosshair.dll` are located.
2. Add in the following content to the file:
    ```ini
    [Settings]
    Size = 3
    ```
    The default size is 3 and minimum is 2.

## Building
1. Install [gcc](https://github.com/brechtsanders/winlibs_mingw/releases/latest) (and or [UPX](https://upx.github.io/) for optional compression).
2. Run `Build.bat`.
