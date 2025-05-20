# qsidebar: A Customizable Notification and Quick Action Center for Linux

**qsidebar** is a lightweight and efficient desktop utility for Linux, written in pure C using the GTK3 toolkit.  It provides a feature-rich sidebar that emulates the functionality of the Windows 10 notification center (with much more configuration options and features).  
-Key features include:

* **Customizable Quick Action Buttons:** Define up to 16 custom buttons with names, icons, types (oneshot/toggle), and associated commands. Supports predefined buttons for common actions like Wi-Fi, Bluetooth, Airplane mode, Night Light, Project (display management), and Focus Assist. Buttons can also have confirmation dialogs.
* **Enhanced Display Management ("Project" Panel):** A dedicated panel for managing dual displays with a custom implementation that aims to be more reliable than `xrandr`. Unlike traditional tools like `xrandr`, it employs a complete rewrite of the display configuration mechanism, prioritizing reliability and preventing potential display issues. Offers options for PC screen only, duplicate, extend, and second screen only modes. Users can also define custom scripts to handle display configurations.  
*The application implements robust logic to accurately detect the primary display in multi-monitor setups. It iterates through display outputs and CRTCs using the Xrandr library to reliably identify the correct primary monitor, addressing a common challenge in multi-display environments. (Note: While qsidebar uses the Xrandr library, it implements its own display management logic rather than directly relying on the `xrandr` command-line tool.)*
* **Integrated Night Light:** qsidebar includes a built-in night light feature, similar to the one found in Windows. This feature adjusts the color temperature of the display, reducing blue light emission to minimize eye strain during evening or nighttime use. The intensity of the effect is customizable, allowing users to fine-tune the warmth of the screen to their preference.
* **Notification Management:** Full notify-daemon with the ability to filter notifications based on title, body, title+body, or application name. Filters can be configured to ignore notifications, accept them silently, or set their urgency. You can also execute custom commands upon matching a filter. Options for notification popup position, colors, opacity, and timeouts are available. You can also hide notification icons and control the tray icon's notification number indicator.
* **Flexible Integration:** `qsidebar` can be used as a standalone application with its own system tray icon.  Alternatively, it offers full integration with **Trinity Desktop environment** through a dedicated Kicker applet, providing seamless control of the sidebar.
* **Optimized Performance:** The program is designed for efficiency, featuring optimized code that results in a small binary size ( < 100KB ) and minimal memory footprint, allowing it to run smoothly even on low-end systems.
* **Extensive Customization:** Highly customizable appearance with options for:
    * Panel tint (default, dark, TDE, or custom RGB).
    * Panel opacity.
    * Background image (with an option for a solid background behind the image).
    * Panel opening/closing animations (slide, fade, slide+fade, none) with optional easing.
    * Custom panel title.
    * Dark mode with night light intensity adjustment.
    * Rounded quick action buttons.
    * Transparent click-through behavior with configurable modes (ALL, DESKTOP) and window types.
    * Bottom margin for the panel.
    * Notification sound (predefined or custom).
    * Customizable fonts for various elements (panel title, text, quick actions, project, notifications).




*Originally, it was designed exclusively for the Trinity Desktop environment (I'm a proud user of Q4OS ;-) ), and the initial development used TQt3. However, my limited experience with Qt development :p and the broader compatibility of GTK3 (allowing it to run on other desktop environments) led to the decision to rewrite the code with GTK3, utilizing a dedicated Kicker applet for improved Trinity integration. The outcome is, in my opinion, a successful compromise, providing seamless Trinity integration while also supporting standalone usage.*
  
*It's worth noting that there's also a GTK2 version of qsidebar (file: qsidebar_gtk2.c). It maintains the same visual appearance as the GTK3 version and offers the same core functionality, albeit with slightly fewer configuration options. This GTK2 version served both as a "style exercise" (the code was initially written in GTK3 to avoid visual limitations, and then adapted for GTK2) and as a response to the need for lower resource consumption on certain systems, such as the Raspberry Pi. The GTK2 version uses somewhat less RAM and potentially slightly fewer CPU resources. However, the difference is often minimal and imperceptible on most systems, so you're encouraged to choose the version that best suits your specific system requirements.
  
* Only X11 : no wayland support for now (and probably never, I'm not interested at all by wayland).  
* Not gtk4 version, as I don't really see the reason to do that, gtk4 is f***** bloated and I profondly dislike desktop apps made in gtk4. So nothing like that planned for now.
  
  -----------------------------------------------------------

**Dependencies:**

qsidebar relies on the following key libraries:

* GTK+ 3.x (`gtk+-3.0`) / ( gtk2 version: GTK+ 2.x (`gtk+-2.0`) )
* GLib (`glib-2.0`) and GIO (`gio-2.0`)
* X11 (`x11`, `xrandr`)
* D-Bus (`dbus-1`)
* libcanberra-gtk3 (`libcanberra-gtk3`)
* NetworkManager (`libnm`)
* Qt (`tqt-mt`)
* C++ Standard Library (`stdc++`)
* / ( gtk2 version: Xinerama )  

Standard C libraries are also required.

 -----------------------------------------------------------
* **Building from sources:**  
  


1) First you need to build the DCOP qsidebar lib if you plan to use the trinity applet, (can be skipped if you don't):

g++ -shared -fPIC -O2 -fstrict-aliasing -flto -fno-fat-lto-objects -ffunction-sections -fdata-sections -fomit-frame-pointer -ffast-math -fno-math-errno -fno-rtti -fno-exceptions -fmerge-all-constants -fuse-ld=gold -o libqsidebar_dcop.so qsidebar_dcop.cpp -I/opt/trinity/include -I/usr/include/tqt/ -I/usr/include/tqt3/ -L/opt/trinity/lib /opt/trinity/lib/libDCOP.so -ltqt-mt -Wl,--gc-sections,--as-needed,--strip-all,-z,norelro,--icf=all,-O1 && strip --strip-all ./libqsidebar_dcop.so  
  
Then Move this lib to /opt/trinity/lib/  
  
2) Build qsidebar binary:  
- gtk3:

gcc -g0 -O2 -DNDEBUG -Wl,-z,norelro -fstrict-aliasing -flto -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables -fomit-frame-pointer -ffast-math -fno-math-errno -fvisibility=hidden -fmerge-all-constants -fuse-ld=gold -Wl,--gc-sections,--build-id=none,--as-needed,--strip-all,-O1,--icf=all,--compress-debug-sections=zlib -s -o qsidebar qsidebar.c `pkg-config --cflags --libs gtk+-3.0 libcanberra-gtk3 libnm gio-2.0 glib-2.0 x11 xrandr` -pthread -L. -lqsidebar_dcop -L/opt/trinity/lib -ltqt-mt -lstdc++ -ldbus-1 -Wl,-rpath=.:/opt/trinity/lib && strip --strip-all ./qsidebar  
  
    
- or gtk2:  
gcc -g0 -O2 -DNDEBUG -Wl,-z,norelro -fstrict-aliasing -flto -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables -fomit-frame-pointer -ffast-math -fno-math-errno -fvisibility=hidden -fmerge-all-constants -fuse-ld=gold -Wl,--gc-sections,--build-id=none,--as-needed,--strip-all,-O1,--icf=all,--compress-debug-sections=zlib -s -o qsidebar qsidebar_gtk2.c `pkg-config --cflags --libs gtk+-2.0 libcanberra-gtk gio-2.0 glib-2.0 x11 xrandr dbus-1 xinerama libnm` -pthread -L. -lqsidebar_dcop -L/opt/trinity/lib -ltqt-mt -lstdc++ -ldbus-1 -Wl,-rpath=.:/opt/trinity/lib && strip --strip-all ./qsidebar  
    
Then Move the binary "qsidebar" to /usr/share/local/bin/  
  
3) Build qsidebar trinity kicker applet :  
  
cd kickapplet  
./autogen.sh
./configure

cd src
./compile_and_install.sh
(this will build the kicker applet library files (qsidebar_applet_panelapplet.so and qsidebar_applet_panelapplet.la) to  /opt/trinity/lib/trinity/ and the qsidebar_applet.desktop file to /opt/trinity/share/apps/kicker/applets/
  
4) Create the ressources folder and copy ressources: 
mkdir -p /usr/share/qsidebar  
Then copy necessary files (icons, sounds) located in "assets" in /usr/share/qsidebar/ (the content of assets folder, not the assets folder itself).  
  
5) Allow qsidebar to control backlight if present:  
 Create file /etc/udev/rules.d/99-backlight.rules with this content:  
SUBSYSTEM=="backlight", ACTION=="add", RUN+="/bin/sh -c 'chmod 666 /sys/class/backlight/*/brightness'"  
then do  
sudo udevadm control --reload-rules && sudo udevadm trigger  
-----------------------------------------------------------  
**Build technical details:**
  
The application is linked against the libraries listed above using `pkg-config` to obtain the correct compiler and linker flags. It also uses `-pthread` for POSIX thread support.
The recommended compilation process uses `gcc` with the following notable flags:

* `-O2`: Optimization level 2, for improved performance (for the 'normal' use case, it's far better from -O3 which increase the final binary too much for no real performance improvements.)
* `-DNDEBUG`: Disables debug assertions.
* `-fstrict-aliasing`, `-flto`, `-ffunction-sections`, `-fdata-sections`, `-fno-asynchronous-unwind-tables`, `-fno-unwind-tables`, `-fomit-frame-pointer`, `-ffast-math`, `-fno-math-errno`, `-fvisibility=hidden`, `-fmerge-all-constants`: A set of advanced optimization flags to reduce binary size and improve execution speed.
* `-fuse-ld=gold`:  Use the Gold linker.
* `-Wl,...`: Linker flags for garbage collection of unused sections, disabling build IDs, stripping all symbols, interprocedural code optimization, and compressing debug sections.
* `-s`: Strip all symbols from the final executable :-)

----------------------------------------------------------- 
**Packages:**

Debian packages are coming soon for a simplified installation.

