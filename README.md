# qsidebar: A Customizable Notification and Quick Action Center for Linux

`qsidebar` is a lightweight and efficient utility for Linux, written in pure C using the GTK3 toolkit.  It provides a feature-rich sidebar that emulates the functionality of the Windows 10 notification center.  Key features include:

* **Customizable Quick Action Buttons:** The sidebar offers a set of user-customizable buttons for frequently used actions.  It includes pre-programmed support for common actions, streamlining tasks.
* **Enhanced Display Management ("Project" Panel):** `qsidebar` incorporates a robust "Project" panel designed for managing multiple display configurations.  Unlike traditional tools like `xrandr`, it employs a complete rewrite of the display configuration mechanism, prioritizing reliability and preventing potential display issues.  This aims to provide a safer and more stable multi-monitor experience.
* **Notification Management:** The application seamlessly integrates with the notify-daemon to handle desktop notifications.  It offers advanced filtering capabilities, enabling users to define actions or modify display behavior based on received notifications.
* **Extensive Customization:** `qsidebar` is highly customizable, allowing users to personalize the panel's appearance.  This includes options to change icons, panel colors, background images, and notification colors, providing a tailored user experience.
* **Flexible Integration:** `qsidebar` can be used as a standalone application with its own system tray icon.  Alternatively, it offers integration with **Trinity Desktop environment** through a dedicated Kicker applet, providing seamless control of the sidebar.
* **Optimized Performance:** The program is designed for efficiency, featuring optimized code that results in a small binary size (approximately 100KB) and minimal memory footprint.
