# qsidebar: A Customizable Notification and Quick Action Center for Linux

**qsidebar** is a lightweight and efficient desktop utility for Linux, written in pure C using the GTK3 toolkit.  It provides a feature-rich sidebar that emulates the functionality of the Windows 10 notification center (with much more configuration options and features).  
-Key features include:

* **Customizable Quick Action Buttons:** Define up to 16 custom buttons with names, icons, types (oneshot/toggle), and associated commands. Supports predefined buttons for common actions like Wi-Fi, Bluetooth, Airplane mode, Night Light, Project (display management), and Focus Assist. Buttons can also have confirmation dialogs.
* **Enhanced Display Management ("Project" Panel):** A dedicated panel for managing dual displays with a custom implementation that aims to be more reliable than `xrandr`. Unlike traditional tools like `xrandr`, it employs a complete rewrite of the display configuration mechanism, prioritizing reliability and preventing potential display issues. Offers options for PC screen only, duplicate, extend, and second screen only modes. Users can also define custom scripts to handle display configurations.  
*The application implements robust logic to accurately detect the primary display in multi-monitor setups. It iterates through display outputs and CRTCs using the Xrandr library to reliably identify the correct primary monitor, addressing a common challenge in multi-display environments. (Note: While qsidebar uses the Xrandr library, it implements its own display management logic rather than directly relying on the `xrandr` command-line tool.)*
* **Integrated Night Light:** qsidebar includes a built-in night light feature, similar to the one found in Windows. This feature adjusts the color temperature of the display, reducing blue light emission to minimize eye strain during evening or nighttime use. The intensity of the effect is customizable, allowing users to fine-tune the warmth of the screen to their preference.
* **Notification Management:** Full notify-daemon with the ability to filter notifications based on title, body, title+body, or application name. Filters can be configured to ignore notifications, accept them silently, or set their urgency. You can also execute custom commands upon matching a filter. Options for notification popup position, colors, opacity, and timeouts are available. You can also hide notification icons and control the tray icon's notification number indicator.
* **Flexible Integration:** `qsidebar` can be used as a standalone application with its own system tray icon.  Alternatively, it offers full integration with **Trinity Desktop environment** through a dedicated Kicker applet, providing seamless control of the sidebar.
* **Optimized Performance:** The program is designed for efficiency, featuring optimized code that results in a small binary size (approximately 100KB) and minimal memory footprint, allowing it to run smoothly even on low-end systems.
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
  
*No wayland support for now (and probably never, I'm not interested at all by wayland)

