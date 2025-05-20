## qsidebar configuration Details (Expanded)

### Animation Details

The `[General settings]` section includes options to control the panel's opening and closing animations:

* **`option_panel_anim_type`**: This option determines the type of animation used when the qsidebar panel appears or disappears.
    * `slide`:   The panel slides in or out from the edge of the screen. This feels like it's being pushed into or out of view.
    * `fade`:   The panel fades in or out, becoming gradually visible or invisible.
    * `slide+fade`:   A combination of both sliding and fading, creating a more dynamic effect.
    * `none`:    No animation is used; the panel appears or disappears instantly. Note that your window manager or compositor might still apply its own fade effects.
* **`option_panel_anim_ease_effect`**: When set to `1`, this option applies an "easing" effect to the animation. Easing makes the animation appear smoother and more natural by varying the speed of the animation over time. Instead of a constant speed, the panel might start slowly, accelerate, and then slow down again as it reaches its final position. This creates a more polished visual transition.

### Font Specification

The `[General settings]` section allows you to customize the fonts used in various parts of the qsidebar interface:

* **`option_paneltitle_font`**:  Sets the font for the main panel title ("Actions center" by default).
* **`option_panel_text_font`**:   Sets the font for general text within the panel.
* **`option_quick_actions_font`**: Sets the font for the text on the quick action buttons.
* **`option_project_font`**:      Sets the font for the text in the "Project" display management panel.
* **`option_notif_font`**:        Sets the font for the text in notification popups.

Each of these options accepts two possible values:

* `system`:   This value tells qsidebar to use the default font configured in your desktop environment. This ensures consistency with your system's overall look and feel.
* `<font_name> [<size>]`:   You can specify a custom font by providing its name. Optionally, you can also specify the font size in points.
    * Examples:
        * `option_paneltitle_font=Arial`
        * `option_panel_text_font="Helvetica Neue" 12`
        * `option_quick_actions_font="Segoe UI Bold" 14`

    **Important:** The font must be installed on your system for qsidebar to use it. If the font is not found, qsidebar will likely fall back to a default font.

#### Button Configuration Examples

The `[Quick action buttons]` section lets you define up to 16 custom buttons.

* **Oneshot Buttons:** These buttons execute a single command when clicked. You can optionally add a confirmation dialog before the command is executed.
    * Example (without confirmation):

        ```ini
        button_1_name=Restart
        button_1_type=oneshot
        button_1_icon=reboot.png
        button_1_cmd=systemctl reboot
        ```

        This creates a button labeled "Restart" with the "reboot.png" icon, which executes the `systemctl reboot` command to restart the system.

    * Example (with confirmation):

        ```ini
        button_2_name=Delete File
        button_2_type=oneshot
        button_2_icon=delete.png
        button_2_cmd=rm /path/to/important_file
        button_2_confirm_cmd=1
        button_2_confirm_text=Are you sure you want to delete this file?
        ```

        This creates a button that, when clicked, displays a confirmation dialog with the message "Are you sure you want to delete this file?". If the user clicks "Yes", the command `rm /path/to/important_file` is executed.

        * **`button_x_confirm_cmd`**: Set to `1` to enable the confirmation dialog. Set to `0` (or omit) to disable it.
        * **`button_x_confirm_text`**: The text to display in the confirmation dialog.

* **Toggle Buttons:** These buttons toggle a state on or off. They require an additional option to check the current state.
    * Example:

        ```ini
        button_3_name=Wi-Fi
        button_3_type=toggle
        button_3_icon=wifi.png
        button_3_cmd=nmcli radio wifi toggle
        button_3_initstate_cmd=nmcli radio wifi
        ```

        This creates a button labeled "Wi-Fi" that toggles Wi-Fi on or off using `nmcli radio wifi toggle`. The `nmcli radio wifi` command returns "enabled" or "disabled", and qsidebar interprets "enabled" as the "on" state.

    * Example using a script:

        ```ini
        button_4_name=MyService
        button_4_type=toggle
        button_4_icon=service.png
        button_4_cmd=systemctl toggle myservice.service
        button_4_initstate_cmd=/path/to/my/check_service_state.sh
        ```

        Here, `check_service_state.sh` is a script that *must* return `0` if the service is running/enabled, and `1` if the service is stopped/disabled.

        ```bash
        #!/bin/bash
        # /path/to/my/check_service_state.sh
        systemctl is-active myservice.service > /dev/null
        if [ $? -eq 0 ]; then
            echo 0
        else
            echo 1
        fi
        ```

### Notification Filters

The `[Notifications filters]` section allows you to define rules for how qsidebar handles incoming notifications. Each filter has a `type`, a `string` to search for, and an `action` or `exec` (or both).

* **`notif_filter_x_type`**: Specifies where to search for the `string`:
    * `title`:  Search only in the notification's title.
    * `body`:   Search only in the notification's body text.
    * `title+body`: Search in both the title and the body.
    * `app_name`: Search in the application name that sent the notification.
* **`notif_filter_x_string`**: The text to search for. The search is case-sensitive.
* **`notif_filter_x_action`**:  What to do with the notification:
    * `ignore`:   Completely suppress the notification. It will not be displayed.
    * `accept_but_silent`: Display the notification but without playing a sound.
    * `set_urgent`:  Mark the notification as urgent (this might affect how it's displayed).
* **`notif_filter_x_exec`**: A command to execute when the filter matches.

Examples:

* Silence battery notifications:
    ```ini
    notif_filter_1_type=body
    notif_filter_1_string=battery
    notif_filter_1_action=accept_but_silent
    ```
* Ignore notifications from a specific application:
    ```ini
    notif_filter_2_type=app_name
    notif_filter_2_string=Thunderbird
    notif_filter_2_action=ignore
    ```
* Run a script when a specific title appears:
    ```ini
    notif_filter_3_type=title
    notif_filter_3_string=Backup Started
    notif_filter_3_exec=/home/user/backup_alert.sh
    ```
* Combine action and exec:
    ```ini
    notif_filter_4_type=title+body
    notif_filter_4_string=Important Update Available
    notif_filter_4_action=set_urgent
    notif_filter_4_exec=popup_dialog.sh "Update Now!"
    ```
    This filter will mark notifications containing "Important Update Available" in their title or body as urgent and also execute the `popup_dialog.sh` script with the message "Update Now!".
