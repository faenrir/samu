#!/bin/sh
export _JAVA_AWT_WM_NONREPARENTING=1
wmname LG3D

hsetroot -solid "#3d5976"
xset s off -dpms
sxhkd &
batsignal &
ewmh &
pipewire &
pipewire-pulse &
