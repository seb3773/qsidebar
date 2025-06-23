#!/bin/bash
wid=$(xdotool search --onlyvisible --class "networkmanager")&&eval $(xdotool getwindowgeometry --shell $wid)&&xdotool mousemove $((X+10)) $((Y+10)) && xdotool click --window $wid 1
