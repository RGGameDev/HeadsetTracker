# HeadsetTracker

A lightweight Linux system tray app that shows your wireless headset's battery level at a glance.

HeadsetTracker wraps the [`headsetcontrol`](https://github.com/Sapd/HeadsetControl) command-line tool and displays a colored icon in the system tray:

- 🟢 **Green** — battery level 25% or higher
- 🟡 **Yellow** — battery level between 10% and 24%
- 🔴 **Red** — battery level below 10%
- ⚪ **Gray** — no headset detected

Hovering over the icon shows a tooltip with the device name and exact battery percentage. A right-click menu offers **Scan Now** (force an immediate refresh) and **Exit**.

## How it works

On startup, and then once a minute thereafter, HeadsetTracker runs:

```
headsetcontrol -b -o JSON
```

It parses the JSON output to find the first connected device that reports battery status, then updates the tray icon and tooltip accordingly.

## Requirements

- Linux with a system tray host (e.g. KDE Plasma, GNOME with an extension, XFCE)
- [`headsetcontrol`](https://github.com/Sapd/HeadsetControl) installed and available on `PATH`
- CMake 3.16+
- A C++20 compiler
- SDL3 and SDL3_gfx (prebuilt shared libraries are vendored under `lib/`, with matching headers under `include/`)
- [nlohmann/json](https://github.com/nlohmann/json) (header vendored under `include/nlohmann/`)

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The build step copies the required SDL3/SDL3_gfx shared libraries alongside the built executable.

## Installing

The `dobuild` script performs a full release build, installs the binary to `/usr/local/bin/HeadsetTracker`, and registers it to autostart on login (KDE Plasma):

```bash
./dobuild
```

This runs, in order:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local/bin/HeadsetTracker
cmake --build build
sudo cmake --install build
mkdir -p ~/.config/autostart
cp HeadsetTracker.desktop ~/.config/autostart/HeadsetTracker.desktop
```

### HeadsetTracker.desktop

This is a [freedesktop `.desktop` entry](https://specifications.freedesktop.org/desktop-entry-spec/latest/) that tells the desktop environment how to launch HeadsetTracker as a background application. It points `Exec` at the installed binary (`/usr/local/bin/HeadsetTracker/HeadsetTracker`), runs it without a terminal window, and, via `X-KDE-autostart-after=panel`, tells KDE Plasma to start it only after the panel (and therefore the system tray) is ready — otherwise the tray icon could fail to appear.

`dobuild` copies this file into `~/.config/autostart/` so HeadsetTracker launches automatically every time you log in. If you'd rather not autostart it, skip that copy step and launch the binary manually instead; you can also drop the file into `/etc/xdg/autostart/` to autostart it for all users on the machine.

## Usage

Once installed and running, HeadsetTracker sits in the system tray. Click the icon for tooltip status, or right-click for the **Scan Now** / **Exit** menu.
