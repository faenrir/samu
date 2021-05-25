# SAMU

`samu` is my personal fork of Dylan Araps's [sowm](https://github.com/dylanaraps/sowm) with many fixes and new features. It's a tiny wm for the Xorg Server. Works on any GNU/Linux or BSD distro with X11.

```bash
date +"%Y is the year of the `uname -o` desktop"
```

## Features

`samu` is getting ewmh support! Combine it with other applications following the UNIX philosophy to create a full desktop system.

- multi monitor support
- window borders with active/inactive colors
  <a href="https://user-images.githubusercontent.com/81267840/112312925-a1635200-8ca7-11eb-8505-dcab972b15b1.png"><img src="https://user-images.githubusercontent.com/81267840/112312925-a1635200-8ca7-11eb-8505-dcab972b15b1.png" width="43%" align="right"></a>
- alternatively no window borders
- awesome keyboard & mouse workflow
- full keyboard-only control possible
- focus with cursor
- always open windows centered
- floating only
- tiling support through scripts
- window teleportation
- properly exit applications
- No ICCCM
- No EWMH
- no bloat, no distractions
- etc etc etc

<a href="https://user-images.githubusercontent.com/81267840/112312944-a6280600-8ca7-11eb-8d0d-d44e48c944bc.png"><img src="https://user-images.githubusercontent.com/81267840/112312944-a6280600-8ca7-11eb-8d0d-d44e48c944bc.png" width="43%" align="right"></a>

## Default Keybindings

See [config.h](config.h)

## Dependencies

- `xlib` (_usually `libX11`_).
- `lXinerama` (_usually `libXinerama`_).
- `lXext` (_usually `libXext`_).

## Experiments

Check out the [branch](https://github.com/faenrir/samu/tree/experiments)

## Installation

### Manually

1. Run `make` to build `samu`.
2. Copy it to your bin path or run `make install`.
3. Launch `samu` with `X`

Important! `samu` does not automatically run as dbus-session!

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

## Thanks

based on the great [sowm](https://github.com/dylanaraps/sowm)

and thanks to [bunnywm](https://github.com/kiedtl/bunnywm)
