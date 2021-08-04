# SAMU

`samu` is my personal fork of cwm with opinionated changes. I previously used `sowm` as base, you can check out the sowm branch.

```bash
date +"%Y is the year of the `uname -o` desktop"
```

## Features

Changes were made to make cwm act more like sowm.

- no coordinates when moving windows
- fix transparency on borders when using picom
- slight changes to mouse behavior when opening new windows
- added window-center option
- added autostart

## Autostart

Samu will attempt to run the `autostart.sh` file located by default in `XDG_CONFIG_HOME/samu/autostart.sh`.

## Installation

### Manually

1. Run `make` to build `cwm`.
2. Copy it to your bin path or run `make install`.
3. Launch `cwm` with `X`

## Three-Finger-Drag Window Movement

Install libinput-gestures with 3-finger drag support. More infos [here](https://github.com/bulletmark/libinput-gestures/issues/10)

Create ~/.config/libinput-gestures.conf

```bash
# Swipe threshold (0-100)
swipe_threshold 0

gesture swipebegin all 3 xdotool keydown super mousedown 1 keyup super
gesture swipeend all 3 xdotool mouseup 1
gesture swipeupdate all 3 xdotool mousemove_relative -- x y

gesture swipe left 4 xdotool key XF86Forward
gesture swipe right 4 xdotool key XF86Back
```

### Recommended touchpad config

/etc/X11/xorg.conf.d/20-touchpad.conf

```bash
Section "InputClass"
        Identifier "libinput touchpad"
        Driver "libinput"
        MatchIsTouchpad "on"
        MatchDevicePath "/dev/input/event*"
        Option "Tapping" "on"
        Option "ClickMethod" "clickfinger"
        Option "NaturalScrolling" "true"
	Option "TappingDragLock" "1"
	Option "AccelProfile" "flat"
	Option "AccelSpeed" "0.4"
EndSection
```
