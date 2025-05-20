#!/bin/bash
wid=$(xdotool search --onlyvisible --class "networkmanager")&&eval $(xdotool getwindowgeometry --shell $wid)&&xdotool mousemove $X $Y && xdotool click --window $wid 1
