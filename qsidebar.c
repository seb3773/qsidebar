#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <canberra-gtk.h>
#include <locale.h>
#include <glib.h>
#include <gio/gio.h>
#include <libnm/NetworkManager.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>
#include <dbus/dbus.h>
#include <pthread.h>
#include "qsidebar_dcop.h"

#define NM_DBUS_SERVICE      "org.freedesktop.NetworkManager"
#define NM_DBUS_PATH         "/org/freedesktop/NetworkManager"
#define NM_DBUS_INTERFACE    "org.freedesktop.NetworkManager"
#define DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define BT_DBUS_SERVICE      "org.bluez"
#define BT_DBUS_MANAGER_PATH "/org/bluez"
#define BT_DBUS_ADAPTER_IFACE "org.bluez.Adapter1"
#define BT_DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
#define MAX_BUTTONS 16
#define MAX_LINE_LENGTH 200
#define MAX_ICON_LENGTH 8192 
#define MAX_DBUS_NOTIFICATIONS 20
#define ICON_PATH "/usr/share/qsidebar/icons/"
#define SCRIPT_PATH "/usr/share/qsidebar/scripts/"
#define MAX_OUTPUTS 10
#define MAX_LINE 200
#define SIG_DBUS_NOTIF 1000
#define MAX_FILTERS 20
#define QUICKSETTINGS_MARGIN           10
#define QUICKSETTINGS_BUTTON_MARGIN    5
#define QUICKSETTINGS_BUTTONS_PER_ROW  4
#define NOTIF_BOX_SPACING 5
#define NOTIF_LABEL_WIDTH 16

#define DEFAULT_CONFIG_CONTENT \
"[General settings]\n" \
"option_tint=230,230,230\n" \
"option_opacity=0.96\n" \
"option_nightlight_intensity=0.8\n" \
"option_dark_mode=0\n" \
"option_panel_anim_type=slide\n" \
"option_use_transparent_click=1\n" \
"option_notif_sound=win10\n\n" \
"option_backlight_control=0\n" \
"[Quick action buttons]\n" \
"button_1_name={wifi}\n" \
"button_1_icon=wifi.png\n\n" \
"button_2_name={nightlight}\n" \
"button_2_icon=darkmode.png\n\n" \
"button_3_name=Network\n" \
"button_3_type=oneshot\n" \
"button_3_icon=network.png\n" \
"button_3_cmd=/usr/share/qsidebar/scripts/network.sh\n\n" \
"button_4_name=Snapshot\n" \
"button_4_type=oneshot\n" \
"button_4_icon=snapshot.png\n" \
"button_4_cmd=ksnapshot\n\n" \
"button_5_name={airplane}\n" \
"button_5_icon=airplane.png\n\n" \
"button_6_name=All Settings\n" \
"button_6_type=oneshot\n" \
"button_6_icon=settings.png\n" \
"button_6_cmd=kcontrol\n\n" \
"button_7_name={bluetooth}\n" \
"button_7_icon=bt.png\n\n" \
"button_8_name={project}\n" \
"button_8_icon=project.png\n" \
"[Project settings]\n" \
"project_extend_full_panel_height=1\n\n"

const gchar *css_data =
"button { border: none;padding: 3px;margin-bottom: -5px;text-decoration: none;}"
".original-button label { padding-left: 3px;color: #000000;}"
".original-button {border: 1px solid #F4F4F4;background-image: image(#F4F4F4);}"
".original-button:hover { border: 1px solid #000000; background-image: image(#FFFFFF);}"
".custom-toggle {background-image: image(#F4F4F4);color: #000000;}"
".custom-toggle:hover { background-image: image(#FFFFFF); }"
".active-toggle label {color: #FFFFFF;}"
".active-toggle {border: 1px solid #3584E4;background-image: image(#3584E4);}"
".active-toggle:hover { background-image: image(#337EDB);color: #EEEEEE; }"
".transparent-button { background:transparent;margin-bottom: -10px;color: #000000;  }"
".transparent-button:hover { background-color: #F6F6F6; }"
".delnotifs-button { background:transparent;margin-bottom: -10px;color: #000000;  }"
".delnotifs-button:hover { color: #444444;}"
".notification-button { border: none;margin: 0px 15px 0px 15px;padding: 5px;color: #000000;background-image: image(#FFFFFF);}"
".notification-button:hover { background-image: image(#F6F6F6); }"
".notification-critical { border: 3px solid #FF0000; background-image: image(#FFFFFF); color: #000000; }"
".notification-critical:hover { background-image: image(#F6F6F6); }"
"#transparent-window { background-color: transparent; }"
"tooltip { border-width: 0px;border: 0px;background-image: image(#FFFFFF);color: #000000;}"
".backlight-slider slider { border-radius: 20%; min-width: 1px; min-height: 32px;padding: 0px;margin-top: -14px;margin-bottom: -14px;background: #3584E4;}"
".actions-center-label { color: #000000; }"
"#confirm-dialog { background-color: #FFFFFF;}"
"#confirm-dialog label { color: #000000;}"
"#confirm-dialog button { background-image: image(#DDDDDD);color: #000000;border: none;padding: 10px 20px;}"
"#confirm-dialog button:hover { background-image: image(#EEEEEE);}"
".project-label { font-weight: bold; color: #000000; }";
const gchar *css_data_dark =
"button { border: none;padding: 3px;margin-bottom: -5px;text-decoration: none;}"
".original-button label { padding-left: 3px;color: #FFFFFF; }"
".original-button {border: 1px solid #2E2E2E;background-image: image(#2E2E2E); }"
".original-button:hover { border: 1px solid #FFFFFF; background-image: image(#000000);}"
".custom-toggle {background-image: image(#2E2E2E);color: #FFFFFF;}"
".custom-toggle:hover { background-image: image(#000000); }"
".active-toggle label {color: #000000;}"
".active-toggle {border: 1px solid #3584E4;background-image: image(#3584E4);}"
".active-toggle:hover { background-image: image(#337EDB);color: #111111; }"
".transparent-button { background:transparent;margin-bottom: -10px;color: #FFFFFF; }"
".transparent-button:hover { background-color: #2E2E2E; }"
".delnotifs-button { background:transparent;margin-bottom: -10px;color:  #FFFFFF; }"
".delnotifs-button:hover { color: #CCCCCC;}"
".notification-button { border: none;margin: 0px 15px 0px 15px;padding: 5px;color: #FFFFFF;background-image: image(#000000);}"
".notification-button:hover { background-image: image(#2E2E2E); }"
".notification-critical { border: 2px solid #FF0000; background-image: image(#000000); color: #FFFFFF; }"
".notification-critical:hover { background-image: image(#2E2E2E); }"
"#transparent-window { background-color: transparent; }"
"tooltip { border-width: 0px;border: 0px;background-image: image(#000000);color: #FFFFFF;}"
".backlight-slider slider { border-radius: 20%; min-width: 1px; min-height: 32px;padding: 0px;margin-top: -14px;margin-bottom: -14px;background: #3584E4;}"
".actions-center-label { color: #FFFFFF; }"
"#confirm-dialog { background-color: #000000;}"
"#confirm-dialog label { color: #FFFFFF;}"
"#confirm-dialog button { background-image: image(#3A3B3C);color: #FFFFFF;border: none;padding: 10px 20px;}"
"#confirm-dialog button:hover { background-image: image(#555555);}"
".project-label { font-weight: bold; color: #FFFFFF; }";

static void regenerate_notifications(GtkWidget *window);
static void slider_changed(GtkRange *range, gpointer user_data);
static void update_current_brightness(void);
static gboolean check_wifi_available(void);
static gboolean check_bluetooth_available(void);

static uint32_t sidebar_flags = 0;
enum SidebarFlags {
    FLAG_IS_BACKGROUND           = 1 << 0,
    FLAG_DARKMODE                = 1 << 1,
    FLAG_ROUNDEDBUTTONS       = 1 << 2,
    FLAG_USE_SYSTRAY             = 1 << 3,
    FLAG_TRAYCOLORMODE           = 1 << 4,
    FLAG_TRINITY_APPLET          = 1 << 5,
    FLAG_PANEL_SOLIDBACKGROUND = 1 << 6,
    FLAG_JUST_CHANGED_CONFIG     = 1 << 7,
    FLAG_HAS_WIFI                = 1 << 8,
    FLAG_HAS_BLUETOOTH           = 1 << 9,
    FLAG_FOCUS_ASSIST            = 1 << 10,
    FLAG_BACKLIGHT_CONTROL       = 1 << 11,
    FLAG_EASE_EFFECT             = 1 << 12,
    FLAG_SILENT_THIS             = 1 << 13,
    FLAG_NOTIF_NUMBER_INDICATOR  = 1 << 14,
    FLAG_NOTIF_HIDE_ICON         = 1 << 15,
    FLAG_TRANSPARENT_CLICK_MODE  = 1 << 16,
    FLAG_EXTEND_MODE             = 1 << 17,
    FLAG_PROJECT_EXTEND_FULL_HEIGHT = 1 << 18,
    FLAG_TINT_IS_DEFAULT = 1 << 19,
    FLAG_WIFI_TESTDONE  = 1 << 20,
    FLAG_BT_TESTDONE = 1 << 21
};

static int bottom_margin;
float intensity = 0.8;
static cairo_surface_t *background_source = NULL;
static int background_width = 0;
static int background_height = 0;
static GtkWidget *window = NULL;
static GtkWidget *notification_popup = NULL;
static GtkCssProvider *cssProvider = NULL;
//static GdkWindowTypeHint type_hint = GDK_WINDOW_TYPE_HINT_DOCK;
static GdkWindowTypeHint notif_type_hint = GDK_WINDOW_TYPE_HINT_DOCK;
static GtkStatusIcon *status_icon = NULL;
static guint timeout_id = 0;
static int max_notification_buttons = 0;
static const char *notification_sound = "/usr/share/qsidebar/sounds/notify_win10.wav";
char panel_title[MAX_LINE_LENGTH];
char panel_background[MAX_LINE_LENGTH];
static volatile sig_atomic_t restart_requested = 0;
static int total_display_width = 0;
static char *username = NULL;
//static GdkFilterReturn xrandr_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data);

typedef struct {
    char *directory;
    int max_brightness;
    int current_brightness;
} BacklightInfo;

static BacklightInfo backlight_info = {NULL, 0, 0};
static GtkWidget *backlight_slider = NULL;
static gboolean initialize_backlight(void);
static guint regenerate_timeout_id = 0;
static uint8_t current_popup_urgency = 255;
static int notif_available_height = 0;
const char *tray_prefix;
const char *tray_icon_normal;
const char *tray_icon_filled;
const char *tray_icon_focus_normal;
const char *tray_icon_focus_filled;
static int transparent_click_type = 1;
static int new_notifs = 0;
static int notif_pos = 0;
static int notif_low_timeout=15;
static int notif_normal_timeout=30;
static float popup_opacity=0.96;
static int tint_popup_r=255;
static int tint_popup_g=255;
static int tint_popup_b=255;
static void on_menu_open_sidebar(GtkMenuItem *item, gpointer user_data);
static void on_menu_focus_assist(GtkMenuItem *item, gpointer user_data);
static void on_menu_no_icons(GtkMenuItem *item, gpointer user_data);
static void on_menu_no_notif(GtkMenuItem *item, gpointer user_data);

static gboolean delayed_regenerate_notifications(gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    regenerate_notifications(window);
    regenerate_timeout_id = 0;
    return G_SOURCE_REMOVE;
}

GtkWidget *confirmation_dialog = NULL;
static int panel_bottom_margin = 0;
PangoFontDescription *font_desc = NULL;
PangoFontDescription *quickbuttons_font_desc = NULL;
PangoFontDescription *projectbuttons_font_desc = NULL;
PangoFontDescription *panel_font_desc = NULL;
PangoFontDescription *paneltitlefont_desc = NULL;
static GtkWidget *delete_notifications_button = NULL;

static int get_secondary_height(void) {
    GdkDisplay *display = gdk_display_get_default();
    int n_monitors = gdk_display_get_n_monitors(display);
    if (n_monitors < 2) {
        fprintf(stderr, "No secondary monitor found, returning default height\n");
        return 1080;
    }
    GdkMonitor *secondary_monitor = gdk_display_get_monitor(display, 1);
    if (!secondary_monitor) {
        fprintf(stderr, "Failed to get secondary monitor, returning default height\n");
        return 1080;
    }
    GdkRectangle geometry;
    gdk_monitor_get_geometry(secondary_monitor, &geometry);
    return geometry.height;
}

gboolean get_wifi_status();
gboolean get_bluetooth_status();
// gboolean get_ethernet_status();
gboolean set_wifi_status(gboolean enable);
gboolean set_bluetooth_status(gboolean enable);
gboolean set_airplane_mode(gboolean enable);
// gboolean set_ethernet_status(gboolean enable);



typedef struct {
    float r, g, b;
} Gamma;



typedef enum {
    ANIM_TYPE_SLIDE,
    ANIM_TYPE_FADE,
    ANIM_TYPE_SLFD,
    ANIM_TYPE_NONE
} AnimationType;



typedef struct {
    float tint_r, tint_g, tint_b;
    float opacity;
    AnimationType anim_type;
    gboolean use_transparent_click;
} RenderOptions;
RenderOptions render_options = {0.90, 0.90, 0.90, 0.96, ANIM_TYPE_SLIDE, TRUE};


typedef struct {
    char summary[MAX_LINE_LENGTH];
    char body[MAX_LINE_LENGTH];
    char icon[MAX_ICON_LENGTH];
    char app_name[MAX_LINE_LENGTH];
    time_t timestamp;
    uint8_t urgency;
} DBusNotification;


typedef struct {
    DBusNotification notifications[MAX_DBUS_NOTIFICATIONS];
    int count;
} DBusNotificationStore;
static DBusNotificationStore dbus_notif_store = { .count = 0 };


typedef struct {
    time_t timestamp;
    char title[MAX_LINE_LENGTH];
    char icon[MAX_ICON_LENGTH];
    char content[MAX_LINE_LENGTH];
    char filename[MAX_LINE_LENGTH];
    uint8_t urgency;
} Notification;
static int compare_notifications(const void *a, const void *b) {
    const Notification *na = (const Notification *)a;
    const Notification *nb = (const Notification *)b;
    return (nb->timestamp - na->timestamp);
}


const Gamma night[] = {
    {1.0, 0.9, 0.90}, {1.0, 0.9, 0.8}, {1.0, 0.85, 0.7}, {1.0, 0.80, 0.7},
    {1.0, 0.80, 0.6}, {1.0, 0.75, 0.55}, {1.0, 0.75, 0.5}, {1.0, 0.75, 0.45},
    {1.0, 0.75, 0.4}, {1.0, 0.70, 0.4}
};
const float nightb[] = {0.9, 0.9, 0.8, 0.8, 0.7, 0.7, 0.6, 0.6, 0.5, 0.5};

const Gamma day[] = {
    {1.0, 0.75, 0.4}, {1.0, 0.75, 0.45}, {1.0, 0.75, 0.5}, {1.0, 0.75, 0.55},
    {1.0, 0.8, 0.6}, {1.0, 0.8, 0.7}, {1.0, 0.85, 0.7}, {1.0, 0.9, 0.8},
    {1.0, 0.9, 0.9}, {1.0, 1.0, 1.0}
};
const float dayb[] = {0.6, 0.6, 0.7, 0.7, 0.8, 0.8, 0.9, 0.9, 0.9, 1.0};



Gamma adjust_gamma(const Gamma base, float intensity) {
    Gamma adjusted;
    adjusted.r = 1.0 + (base.r - 1.0) * intensity;
    adjusted.g = 1.0 + (base.g - 1.0) * intensity;
    adjusted.b = 1.0 + (base.b - 1.0) * intensity;
    return adjusted;
}


float adjust_brightness(float base, float intensity) {
    return 1.0 + (base - 1.0) * intensity;
}


gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean animate_window(gpointer user_data);
void update_toggle_button_states();
GdkPixbuf *invert_pixbuf_colors(GdkPixbuf *original);
void update_button_icon(GtkWidget *button, gboolean is_active);
static void show_notification_popup(const Notification *notif);


typedef struct {
    GtkWidget *window;         // 8 octets
    GtkWidget *click_window;   // 8 octets
    GdkRectangle workarea;     // 16 octets (4x int)
    int target_x;              // 4 octets
    int current_x;             // 4 octets
    int width;                 // 4 octets
    int height;                // 4 octets
    int start_x;               // 4 octets
    int y_position;            // 4 octets
    int pas_anim;              // 4 octets
    int pas_hide;              // 4 octets
    float current_opacity;     // 4 octets
    float target_opacity;      // 4 octets
    unsigned int is_animating    : 1;
    unsigned int is_opening      : 1;
    unsigned int is_project_panel: 1;
    unsigned int update_pending  : 1;
    unsigned int _unused         : 4;
} AnimationData;



typedef struct {
    GtkWidget *button; // 8 octets
    int initial_width;   // 4 octets
    int initial_height;  // 4 octets
    int current_width;   // 4 octets
    int current_height;  // 4 octets
    int initial_x;       // 4 octets
    int initial_y;       // 4 octets
    int panel_width;     // 4 octets
    float current_opacity; // 4 octets
    gboolean is_animating; // 4 octets
} NotificationAnimData;



typedef struct {
    char name[MAX_LINE_LENGTH];
    char type[MAX_LINE_LENGTH];
    char icon_path[MAX_LINE_LENGTH];
    char cmd[MAX_LINE_LENGTH];
    char initstate_cmd[MAX_LINE_LENGTH];
    char confirm_text[MAX_LINE_LENGTH];
    GtkWidget *button;                // 8 octets
    unsigned int confirm_cmd      : 1;
    unsigned int is_active        : 1;
    unsigned int is_preprogrammed : 1;
    unsigned int icon_only        : 1;
    unsigned int _unused          : 4;
} ButtonConfig;


ButtonConfig button_configs[MAX_BUTTONS];
int num_buttons = 0;


typedef struct {
    char type[MAX_LINE_LENGTH];
    char string[MAX_LINE_LENGTH];
    char action[MAX_LINE_LENGTH];
    char exec[MAX_LINE_LENGTH];
} NotifFilter;

NotifFilter notif_filters[MAX_FILTERS];



AnimationData *anim_data = NULL;






gchar* build_css(const gchar *base_css) {
    extern int tint_popup_r, tint_popup_g, tint_popup_b;
    if ((sidebar_flags & FLAG_TINT_IS_DEFAULT) && (sidebar_flags & FLAG_DARKMODE)) {
        tint_popup_r = 0;
        tint_popup_g = 0;
        tint_popup_b = 0;
    }
    char color1[8], color2[8];
    snprintf(color1, sizeof(color1), "#%02X%02X%02X", tint_popup_r, tint_popup_g, tint_popup_b);
  int dr, dg, db;
    if (tint_popup_r == 0 && tint_popup_g == 0 && tint_popup_b == 0) {
        int temp_r = 15;
        int temp_g = 15;
        int temp_b = 15;
        if (sidebar_flags & FLAG_DARKMODE) {
            dr = (int)(temp_r * 2);
            dg = (int)(temp_g * 2);
            db = (int)(temp_b * 2);
        } else {
            dr = (int)(temp_r * 0.96);
            dg = (int)(temp_g * 0.96);
            db = (int)(temp_b * 0.96);
        }
    } else {
        if (sidebar_flags & FLAG_DARKMODE) {
            dr = (int)(tint_popup_r * 1.04);
            dg = (int)(tint_popup_g * 1.04);
            db = (int)(tint_popup_b * 1.04);
        } else {
            dr = (int)(tint_popup_r * 0.96);
            dg = (int)(tint_popup_g * 0.96);
            db = (int)(tint_popup_b * 0.96);
        }
    }
    if (dr > 255) dr = 255; if (dg > 255) dg = 255; if (db > 255) db = 255;
    if (dr < 0) dr = 0; if (dg < 0) dg = 0; if (db < 0) db = 0;
    snprintf(color2, sizeof(color2), "#%02X%02X%02X", dr, dg, db);
    GString *out = g_string_new(NULL);
    const char *p = base_css;
    while (*p) {
        const char *notif = g_strstr_len(p, -1, ".notification-button {");
        const char *hover = g_strstr_len(p, -1, ".notification-button:hover {");
        const char *next = NULL;
        gboolean is_hover = FALSE;
        if (notif && (!hover || notif < hover)) {
            next = notif;
            is_hover = FALSE;
        } else if (hover) {
            next = hover;
            is_hover = TRUE;
        }
        if (!next) {
            g_string_append(out, p);
            break;
        }
        g_string_append_len(out, p, next - p);
        const char *block_end = strchr(next, '}');
        if (!block_end) {
            g_string_append(out, next);
            break;
        }
        const char *imgpos = g_strstr_len(next, block_end - next, "background-image: image(");
        if (imgpos) {
            const char *hash = strchr(imgpos, '#');
            if (hash && hash < block_end) {
                const char *paren = strchr(hash, ')');
                if (paren && paren < block_end) {
                    g_string_append_len(out, next, hash - next);
                    g_string_append(out, is_hover ? color2 : color1);
                    g_string_append_len(out, paren, block_end - paren + 1);
                    p = block_end + 1;
                    continue;
                }
            }
        }
        g_string_append_len(out, next, block_end - next + 1);
        p = block_end + 1;
    }
if (sidebar_flags & FLAG_ROUNDEDBUTTONS) {
        const gchar *needle = ".original-button {";
        gchar *tmp = out->str;
        const gchar *start = g_strstr_len(tmp, -1, needle);
        if (start) {
            const gchar *end = strchr(start, '}');
            if (end) {
                size_t before_len = end - tmp;
                size_t inject_len = strlen(" border-radius: 18%;");
                size_t after_len = strlen(end);
                size_t total = before_len + inject_len + after_len + 1;
                gchar *result = g_malloc(total);
                memcpy(result, tmp, before_len);
                memcpy(result + before_len, " border-radius: 18%;", inject_len);
                memcpy(result + before_len + inject_len, end, after_len);
                result[total - 1] = 0;
                g_string_free(out, TRUE);
                return result;
            }
        }
    }
    return g_string_free(out, FALSE);
}



void load_css(void) {
    const char *base_css = (sidebar_flags & FLAG_DARKMODE) ? css_data_dark : css_data;
    gchar *css_data_to_use = build_css(base_css);
    if (cssProvider) {
        gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider));
        g_object_unref(cssProvider);
    }
    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cssProvider, css_data_to_use, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_free(css_data_to_use);
}



static void calculate_max_notification_buttons_once(int panel_height, int num_buttons) {
    int margin = 10, button_margin = 4, spacing = 5;
    int buttons_per_row = 4;
    int panel_height_div_4_5 = panel_height * 10 / 45;
    int button_width = (panel_height_div_4_5 - 2 * margin - 3 * button_margin) / buttons_per_row;
    int button_height = button_width * 5 / 6;
    int num_rows = (num_buttons + buttons_per_row - 1) / buttons_per_row;
    int quick_settings_height = (num_rows * (button_height + button_margin) + margin) * 11 / 10;
    int header_height = panel_height * 38 / 1000;
    int notif_button_height = panel_height / 10;
    int slider_height = (sidebar_flags & FLAG_BACKLIGHT_CONTROL) ? 40 : 0;
    int panel_height_0114 = panel_height * 114 / 1000;
    int available_height = panel_height - quick_settings_height - header_height - bottom_margin - panel_height_0114 - notif_button_height - slider_height;
    notif_available_height = available_height;
    max_notification_buttons = available_height / (notif_button_height + spacing);
}


typedef struct {
    char primary_display[64];
    char secondary_display[64];
    int has_secondary;
} DisplayInfo;
static char g_primary_display_name[64] = "";


static void add_notification_to_panel(GtkWidget *box, int width, int height);


static Display *open_display() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        g_print("Can't open X display\n");
    }
    return dpy;
}


static void identify_primary_display(void) {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        g_print("primary detection: failed to open X display\n");
        return;
    }
    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *resources = XRRGetScreenResources(dpy, root);
    if (!resources) {
        g_print("primary detection: failed to get screen resources\n");
        XCloseDisplay(dpy);
        return;
    }
    RROutput primary_output = XRRGetOutputPrimary(dpy, root);
    if (primary_output != None) {
        for (int i = 0; i < resources->noutput; i++) {
            XRROutputInfo *output_info = XRRGetOutputInfo(dpy, resources, resources->outputs[i]);
            if (output_info && output_info->connection == RR_Connected) {
                if (resources->outputs[i] == primary_output) {
                    strncpy(g_primary_display_name, output_info->name, sizeof(g_primary_display_name) - 1);
                    g_primary_display_name[sizeof(g_primary_display_name) - 1] = '\0';
                    XRRFreeOutputInfo(output_info);
                    break;
                }
                XRRFreeOutputInfo(output_info);
            }
        }
    }
    if (g_primary_display_name[0] == '\0') {
        for (int i = 0; i < resources->noutput; i++) {
            XRROutputInfo *output_info = XRRGetOutputInfo(dpy, resources, resources->outputs[i]);
            if (output_info && output_info->connection == RR_Connected) {
                strncpy(g_primary_display_name, output_info->name, sizeof(g_primary_display_name) - 1);
                g_primary_display_name[sizeof(g_primary_display_name) - 1] = '\0';
                XRRFreeOutputInfo(output_info);
                break;
            }
            XRRFreeOutputInfo(output_info);
        }
    }
    int primary_width = 0, secondary_width = 0;
    int active_crtcs = 0;
    gboolean is_extend = FALSE;
    for (int i = 0; i < resources->ncrtc; i++) {
        XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, resources, resources->crtcs[i]);
        if (crtc_info && crtc_info->mode != None) {
            active_crtcs++;
            for (int j = 0; j < resources->noutput; j++) {
                XRROutputInfo *output_info = XRRGetOutputInfo(dpy, resources, resources->outputs[j]);
                if (output_info && output_info->crtc == resources->crtcs[i] &&
                    strcmp(output_info->name, g_primary_display_name) == 0) {
                    primary_width = crtc_info->width;
                } else if (output_info && output_info->crtc == resources->crtcs[i]) {
                    if (crtc_info->x > 0) {
                        is_extend = TRUE;
                        secondary_width = crtc_info->width;
                    }
                }
                if (output_info) XRRFreeOutputInfo(output_info);
            }
        }
        if (crtc_info) XRRFreeCrtcInfo(crtc_info);
    }
    if (is_extend && active_crtcs >= 2) {
        sidebar_flags |= FLAG_EXTEND_MODE;
        total_display_width = primary_width + secondary_width;
      //  g_print("Detected Extend mode: primary_width=%d, secondary_width=%d, total_display_width=%d\n",
     //           primary_width, secondary_width, total_display_width);
    } else {
        sidebar_flags &= ~FLAG_EXTEND_MODE;
        total_display_width = primary_width;
//        g_print("Detected non-Extend mode: primary_width=%d\n", primary_width);
    }
    XRRFreeScreenResources(resources);
    XCloseDisplay(dpy);
//    if (g_primary_display_name[0] != '\0') {
//        g_print("Primary display identified: %s\n", g_primary_display_name);
//    }   else {
       // g_print("No primary display identified\n");
      // }
}







static DisplayInfo get_display_info() {
    DisplayInfo info = {"", "", 0};
    sidebar_flags &= ~FLAG_EXTEND_MODE;
    total_display_width = 0;
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        perror("Unable to open X display");
        return info;
    }
    int event_base, error_base;
    if (!XRRQueryExtension(display, &event_base, &error_base)) {
        g_print("RandR extension not available\n");
        XCloseDisplay(display);
        return info;
    }
    XRRScreenResources *screen_resources = XRRGetScreenResources(display, DefaultRootWindow(display));
    if (!screen_resources) {
        g_print("Unable to get screen resources\n");
        XCloseDisplay(display);
        return info;
    }
    if (g_primary_display_name[0] != '\0') {
        strncpy(info.primary_display, g_primary_display_name, sizeof(info.primary_display) - 1);
        info.primary_display[sizeof(info.primary_display) - 1] = '\0';
    }
    for (int i = 0; i < screen_resources->noutput; i++) {
        RROutput output = screen_resources->outputs[i];
        XRROutputInfo *output_info = XRRGetOutputInfo(display, screen_resources, output);
        if (output_info && output_info->connection == RR_Connected) {
            if (output_info->nameLen > 0 && strcmp(output_info->name, info.primary_display) != 0) {
                strncpy(info.secondary_display, output_info->name, sizeof(info.secondary_display) - 1);
                info.secondary_display[sizeof(info.secondary_display) - 1] = '\0';
                info.has_secondary = 1;
                if (output_info->crtc) {
                    XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(display, screen_resources, output_info->crtc);
                    if (crtc_info) {
                        if (crtc_info->x > 0) {
                            sidebar_flags |= FLAG_EXTEND_MODE;
                            total_display_width = crtc_info->x + crtc_info->width;
                        }
                        XRRFreeCrtcInfo(crtc_info);
                    }
                }
            }
            XRRFreeOutputInfo(output_info);
        } else if (output_info) {
            XRRFreeOutputInfo(output_info);
        }
    }
    XRRFreeScreenResources(screen_resources);
    XCloseDisplay(display);
    return info;
}







static const char* detect_display_configuration() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        perror("Unable to open X display");
        return "unknown";
    }
    int event_base, error_base;
    if (!XRRQueryExtension(display, &event_base, &error_base)) {
        g_print("RandR extension not available\n");
        XCloseDisplay(display);
        return "unknown";
    }
    XRRScreenResources *screen_resources = XRRGetScreenResources(display, DefaultRootWindow(display));
    if (!screen_resources) {
        g_print("Unable to get screen resources\n");
        XCloseDisplay(display);
        return "unknown";
    }
    struct {
        char name[64];
        int is_primary;
        int is_connected;
        int is_active;
        int x, y;
        int width, height;
    } outputs[2] = {0};
    int output_index = 0;
    for (int i = 0; i < screen_resources->noutput && output_index < 2; i++) {
        RROutput output = screen_resources->outputs[i];
        XRROutputInfo *output_info = XRRGetOutputInfo(display, screen_resources, output);
        if (output_info && output_info->connection == RR_Connected) {
            if (output_info->nameLen > 0) {
                strncpy(outputs[output_index].name, output_info->name, sizeof(outputs[output_index].name) - 1);
                outputs[output_index].name[sizeof(outputs[output_index].name) - 1] = '\0';
                outputs[output_index].is_connected = 1;
                if (output_info->crtc) {
                    XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(display, screen_resources, output_info->crtc);
                    if (crtc_info) {
                        outputs[output_index].width = crtc_info->width;
                        outputs[output_index].height = crtc_info->height;
                        outputs[output_index].x = crtc_info->x;
                        outputs[output_index].y = crtc_info->y;
                        outputs[output_index].is_active = (crtc_info->width > 0 && crtc_info->height > 0);
                        outputs[output_index].is_primary = (crtc_info->noutput == 1 && crtc_info->outputs[0] == output);
                        XRRFreeCrtcInfo(crtc_info);
                    }
                }
                output_index++;
            }
            XRRFreeOutputInfo(output_info);
        }
    }
    XRRFreeScreenResources(screen_resources);
    XCloseDisplay(display);
    if (output_index == 0) {
        return "unknown";
    }
    int act0 = outputs[0].is_active;
    int act1 = outputs[1].is_active;
    if (act0 && !act1) {
        return "PC screen only";
    }
    if (!act0 && act1) {
        return "Second screen only";
    }
    if (!act0 && !act1) {
        return "unknown";
    }
    if (outputs[0].x == outputs[1].x && outputs[0].y == outputs[1].y) {
        return "Duplicate";
    }
    return "Extend";
}


gboolean recreate_original_buttons(gpointer user_data);


void hide_project_panel_immediately() {
    anim_data->is_project_panel = FALSE;
    recreate_original_buttons(NULL);
    anim_data->current_x = anim_data->start_x;
    anim_data->current_opacity = 0.0;
    anim_data->is_animating = FALSE;
    anim_data->is_opening = FALSE;
    gtk_widget_hide(anim_data->window);
    if (anim_data->click_window) {
        gtk_widget_hide(anim_data->click_window);
    }
    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }
}



static RRCrtc find_available_crtc(Display *dpy, XRRScreenResources *resources, RRCrtc exclude_crtc) {
    for (int i = 0; i < resources->ncrtc; i++) {
        if (resources->crtcs[i] == exclude_crtc) continue;
        XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, resources, resources->crtcs[i]);
        if (crtc_info && crtc_info->mode == None) {
            XRRFreeCrtcInfo(crtc_info);
            return resources->crtcs[i];
        }
        XRRFreeCrtcInfo(crtc_info);
    }
    return None;
}





static void update_window_sizes(AnimationData *anim_data) {
    GdkRectangle workarea;
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_monitor(display, (sidebar_flags & FLAG_EXTEND_MODE) ? 1 : 0);
    if (!monitor) {
        monitor = gdk_display_get_monitor(display, 0);
    }
    gdk_monitor_get_workarea(monitor, &workarea);
int window_height = ((sidebar_flags & FLAG_EXTEND_MODE) && (sidebar_flags & FLAG_PROJECT_EXTEND_FULL_HEIGHT))
    ? get_secondary_height()
    : anim_data->height;
gtk_window_resize(GTK_WINDOW(anim_data->window), anim_data->width, window_height);
   if (anim_data->click_window) {
        int click_window_width = ((sidebar_flags & FLAG_EXTEND_MODE) ? total_display_width : workarea.width) - anim_data->width;
        gtk_window_resize(GTK_WINDOW(anim_data->click_window), click_window_width, window_height);
        gtk_window_move(GTK_WINDOW(anim_data->click_window), workarea.x, workarea.y);
    }
}




static void handle_project_action(const char *action) {
    DisplayInfo displays = get_display_info();
    if (displays.primary_display[0] == '\0') {
        g_print("No primary display found\n");
        return;
    }
    if (!displays.has_secondary) {
        return;
    }
    const char *current_mode = detect_display_configuration();
    if (strcmp(current_mode, action) == 0) {
        return;
    }
    hide_project_panel_immediately();
    usleep(200000);
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        g_print("Failed to open X display\n");
        return;
    }
    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *resources = XRRGetScreenResources(dpy, root);
    if (!resources) {
        g_print("Failed to get screen resources\n");
        XCloseDisplay(dpy);
        return;
    }
    for (int i = 0; i < resources->ncrtc; i++) {
        XRRSetCrtcConfig(dpy, resources, resources->crtcs[i],
                         CurrentTime, 0, 0, None, RR_Rotate_0, NULL, 0);
    }
    XSync(dpy, False);
    RROutput primary_output = None;
    RROutput secondary_output = None;
    RRMode primary_mode = None;
    RRMode secondary_mode = None;
    XRROutputInfo *primary_output_info = NULL;
    XRROutputInfo *secondary_output_info = NULL;
    // detect primary res & try to find compatible secondary mode
    int primary_width = 0, primary_height = 0;
    int secondary_width = 0, secondary_height = 0;
    for (int i = 0; i < resources->noutput; i++) {
        XRROutputInfo *output_info = XRRGetOutputInfo(dpy, resources, resources->outputs[i]);
        if (!output_info || output_info->connection != RR_Connected) {
            XRRFreeOutputInfo(output_info);
            continue;
        }
        if (strcmp(output_info->name, displays.primary_display) == 0) {
            primary_output = resources->outputs[i];
            primary_output_info = output_info;
            if (output_info->crtc) {
                XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, resources, output_info->crtc);
                if (crtc_info) {
                    primary_width = crtc_info->width;
                    primary_height = crtc_info->height;
                    primary_mode = crtc_info->mode;
                    XRRFreeCrtcInfo(crtc_info);
                }
            }
            if (!primary_mode && output_info->nmode > 0) {
                primary_mode = output_info->modes[0];
                for (int j = 0; j < resources->nmode; j++) {
                    if (resources->modes[j].id == primary_mode) {
                        primary_width = resources->modes[j].width;
                        primary_height = resources->modes[j].height;
                        break;
                    }
                }
            }
        } else if (displays.has_secondary && strcmp(output_info->name, displays.secondary_display) == 0) {
            secondary_output = resources->outputs[i];
            secondary_output_info = output_info;
            if (primary_width > 0 && primary_height > 0) {
                for (int j = 0; j < output_info->nmode; j++) {
                    for (int k = 0; k < resources->nmode; k++) {
                        if (resources->modes[k].id == output_info->modes[j] &&
                            (int)resources->modes[k].width == primary_width &&
                            (int)resources->modes[k].height == primary_height) {
                            secondary_mode = output_info->modes[j];
                            secondary_width = primary_width;
                            secondary_height = primary_height;
                            break;
                        }
                    }
                    if (secondary_mode) break;
                }
            }
            if (!secondary_mode && output_info->nmode > 0) {
                secondary_mode = output_info->modes[0];
                for (int j = 0; j < resources->nmode; j++) {
                    if (resources->modes[j].id == secondary_mode) {
                        secondary_width = resources->modes[j].width;
                        secondary_height = resources->modes[j].height;
                        g_print("Warning: Secondary display using default mode %dx%d\n",
                                secondary_width, secondary_height);
                        break;
                    }
                }
            }
        } else {
            XRRFreeOutputInfo(output_info);
        }
    }
    if (primary_output == None || primary_output_info == NULL) {
        g_print("Primary display not found/not connected\n");
        goto cleanup;
    }
    if (primary_width == 0 || primary_height == 0) {
        g_print("Unable to detect primary resolution, using default 1920x1080\n");
        primary_width = 1920;
        primary_height = 1080;
        if (primary_output_info && primary_output_info->nmode > 0) {
            for (int j = 0; j < primary_output_info->nmode; j++) {
                for (int k = 0; k < resources->nmode; k++) {
                    if (resources->modes[k].id == primary_output_info->modes[j] &&
                        resources->modes[k].width == 1920 &&
                        resources->modes[k].height == 1080) {
                        primary_mode = primary_output_info->modes[j];
                        break;
                    }
                }
                if (primary_mode) break;
            }
        }
        if (!primary_mode && primary_output_info->nmode > 0) {
            primary_mode = primary_output_info->modes[0];
        }
    }
    if ((strcmp(action, "Duplicate") == 0 || strcmp(action, "Extend") == 0 || strcmp(action, "Second screen only") == 0) &&
        secondary_mode == None && secondary_output_info && secondary_output_info->nmode > 0) {
        secondary_mode = secondary_output_info->modes[0];
        for (int j = 0; j < resources->nmode; j++) {
            if (resources->modes[j].id == secondary_mode) {
                secondary_width = resources->modes[j].width;
                secondary_height = resources->modes[j].height;
                g_print("Warning: Secondary display using default mode %dx%d\n",
                        secondary_width, secondary_height);
                break;
            }
        }
    }
    RRCrtc primary_crtc = primary_output_info->crtc ? primary_output_info->crtc : find_available_crtc(dpy, resources, None);
    if (primary_crtc == None) {
        g_print("No CRTC available for primary display\n");
        goto cleanup;
    }
    RRCrtc secondary_crtc = None;
    if (secondary_output != None && secondary_output_info != NULL) {
        secondary_crtc = secondary_output_info->crtc ? secondary_output_info->crtc : find_available_crtc(dpy, resources, primary_crtc);
    }
    Status status;
    if (strcmp(action, "PC screen only") == 0) {
        XRRSetOutputPrimary(dpy, root, primary_output);
        status = XRRSetCrtcConfig(dpy, resources, primary_crtc,
                                  CurrentTime, 0, 0, primary_mode,
                                  RR_Rotate_0, &primary_output, 1);
        if (status != Success) {
            g_print("Failed to configure primary display (%d)\n", status);
            goto cleanup;
        }
        XRRSetScreenSize(dpy, root, primary_width, primary_height,
                         primary_width * 25 / 10, primary_height * 25 / 10);
       sidebar_flags |= FLAG_JUST_CHANGED_CONFIG;
       sidebar_flags &= ~FLAG_EXTEND_MODE;
        XSync(dpy, False);
        {
            GdkDisplay *gdk_display = gdk_display_get_default();
            GdkMonitor *monitor = gdk_display_get_monitor(gdk_display, 0);
            GdkRectangle workarea;
            gdk_monitor_get_workarea(monitor, &workarea);
            anim_data->start_x = workarea.width;
            anim_data->target_x = workarea.width - anim_data->width;
            anim_data->current_x = anim_data->start_x;
            gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->start_x, anim_data->y_position);
            update_window_sizes(anim_data);
        }
    } else if (strcmp(action, "Duplicate") == 0) {
        XRRSetOutputPrimary(dpy, root, primary_output);
        status = XRRSetCrtcConfig(dpy, resources, primary_crtc,
                                  CurrentTime, 0, 0, primary_mode,
                                  RR_Rotate_0, &primary_output, 1);
        if (status != Success) {
            g_print("Failed to configure primary display for duplication (%d)\n", status);
            goto cleanup;
        }
        XSync(dpy, False);

        if (secondary_crtc == None) {
            secondary_crtc = find_available_crtc(dpy, resources, primary_crtc);
            if (secondary_crtc == None) {
                g_print("No CRTC available for secondary display\n");
                goto cleanup;
            }
        }
        status = XRRSetCrtcConfig(dpy, resources, secondary_crtc,
                                  CurrentTime, 0, 0, secondary_mode,
                                  RR_Rotate_0, &secondary_output, 1);
        if (status != Success) {
            g_print("Failed to configure secondary display for duplication (%d)\n", status);
            goto cleanup;
        }
        //        g_print("Setting screen size: %dx%d\n", primary_width, primary_height);
        XRRSetScreenSize(dpy, root, primary_width, primary_height,
                         primary_width * 25 / 10, primary_height * 25 / 10);
        sidebar_flags |= FLAG_JUST_CHANGED_CONFIG;
       sidebar_flags &= ~FLAG_EXTEND_MODE;
        XSync(dpy, False);
        {
            GdkDisplay *gdk_display = gdk_display_get_default();
            GdkMonitor *monitor = gdk_display_get_monitor(gdk_display, 0);
            GdkRectangle workarea;
            gdk_monitor_get_workarea(monitor, &workarea);
            anim_data->start_x = workarea.width;
            anim_data->target_x = workarea.width - anim_data->width;
            anim_data->current_x = anim_data->start_x;
            gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->start_x, anim_data->y_position);
            update_window_sizes(anim_data);
        }
    } else if (strcmp(action, "Extend") == 0) {
        XRRSetOutputPrimary(dpy, root, primary_output);
        status = XRRSetCrtcConfig(dpy, resources, primary_crtc,
                                  CurrentTime, 0, 0, primary_mode,
                                  RR_Rotate_0, &primary_output, 1);
        if (status != Success) {
            g_print("Failed to configure primary display for extend (%d)\n", status);
            goto cleanup;
        }
        XSync(dpy, False);
        if (secondary_crtc == None) {
            secondary_crtc = find_available_crtc(dpy, resources, primary_crtc);
            if (secondary_crtc == None) {
                g_print("No CRTC available for secondary display\n");
                goto cleanup;
            }
        }
        status = XRRSetCrtcConfig(dpy, resources, secondary_crtc,
                                  CurrentTime, primary_width, 0, secondary_mode,
                                  RR_Rotate_0, &secondary_output, 1);
        if (status != Success) {
            g_print("Failed to configure secondary display for extend (%d)\n", status);
            goto cleanup;
        }
        XRRSetScreenSize(dpy, root, primary_width + secondary_width, primary_height,
                         (primary_width + secondary_width) * 25 / 10, primary_height * 25 / 10);
        sidebar_flags |= FLAG_JUST_CHANGED_CONFIG;
        sidebar_flags |= FLAG_EXTEND_MODE;
        total_display_width = primary_width + secondary_width;
        XSync(dpy, False);
        {
            anim_data->start_x = primary_width + secondary_width;
            anim_data->target_x = (primary_width + secondary_width) - anim_data->width;
            anim_data->current_x = anim_data->start_x;
            gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->start_x, anim_data->y_position);
            update_window_sizes(anim_data);
        }
    } else if (strcmp(action, "Second screen only") == 0) {
        if (secondary_crtc == None) {
            secondary_crtc = find_available_crtc(dpy, resources, None);
            if (secondary_crtc == None) {
                g_print("No CRTC available for secondary display\n");
                goto cleanup;
            }
        }
        status = XRRSetCrtcConfig(dpy, resources, secondary_crtc,
                                  CurrentTime, 0, 0, secondary_mode,
                                  RR_Rotate_0, &secondary_output, 1);
        if (status != Success) {
            g_print("Failed to configure secondary display (%d)\n", status);
            goto cleanup;
        }
        sidebar_flags |= FLAG_JUST_CHANGED_CONFIG;
        sidebar_flags &= ~FLAG_EXTEND_MODE;
        XSync(dpy, False);
        {
            GdkDisplay *gdk_display = gdk_display_get_default();
            GdkMonitor *monitor = gdk_display_get_monitor(gdk_display, 0);
            GdkRectangle workarea;
            gdk_monitor_get_workarea(monitor, &workarea);
            anim_data->start_x = workarea.width;
            anim_data->target_x = workarea.width - anim_data->width;
            anim_data->current_x = anim_data->start_x;
            gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->start_x, anim_data->y_position);
            update_window_sizes(anim_data);
        }
    } 
cleanup:
    if (primary_output_info) XRRFreeOutputInfo(primary_output_info);
    if (secondary_output_info) XRRFreeOutputInfo(secondary_output_info);
    XRRFreeScreenResources(resources);
    XCloseDisplay(dpy);
}





float get_brightness() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 0.0f;
    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *res = XRRGetScreenResourcesCurrent(dpy, root);
    if (!res) { XCloseDisplay(dpy); return 0.0f; }
    float brightness = 1.0f;
    int found = 0;
    for (int i = 0; i < res->ncrtc && !found; ++i) {
        XRRCrtcGamma *gamma = XRRGetCrtcGamma(dpy, res->crtcs[i]);
        if (gamma) {
            unsigned long sum = 0;
            int size = gamma->size, j = 0;
            unsigned short *r = gamma->red, *g = gamma->green, *b = gamma->blue;
            for (; j < size; ++j)
                sum += r[j] + g[j] + b[j];
            brightness = (float)(sum * 2) / (size * 3 * 65535);
            XRRFreeGamma(gamma);
            found = 1;
        }
    }
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return brightness;
}







int get_connected_outputs(char outputs[MAX_OUTPUTS][MAX_LINE]) {
    Display *dpy = open_display();
    if (!dpy) return 0;
    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *resources = XRRGetScreenResources(dpy, root);
    if (!resources) {
        XCloseDisplay(dpy);
        return 0;
    }
    int count = 0;
    for (int i = 0; i < resources->noutput && count < MAX_OUTPUTS; i++) {
        XRROutputInfo *output_info = XRRGetOutputInfo(dpy, resources, resources->outputs[i]);
        if (!output_info) continue;
        if (output_info->connection == RR_Connected) {
            RRCrtc crtc = output_info->crtc;
            if (crtc != None) {
                XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(dpy, resources, crtc);
                if (crtc_info && crtc_info->width > 0 && crtc_info->height > 0) {
                    strncpy(outputs[count], output_info->name, MAX_LINE - 1);
                    outputs[count][MAX_LINE - 1] = '\0';
                    count++;
                }
                if (crtc_info) XRRFreeCrtcInfo(crtc_info);
            }
        }
        XRRFreeOutputInfo(output_info);
    }
    XRRFreeScreenResources(resources);
    XCloseDisplay(dpy);
    return count;
}






void apply_settings(const Gamma *gamma, const float *brightness, char outputs[MAX_OUTPUTS][MAX_LINE], int output_count, int settings_count, float intensity) {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return;
    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *res = XRRGetScreenResources(dpy, root);
    if (!res) { XCloseDisplay(dpy); return; }
    for (int i = 0; i < settings_count; ++i) {
        Gamma ag = adjust_gamma(gamma[i], intensity);
        float ab = adjust_brightness(brightness[i], intensity);
        for (int j = 0; j < output_count; ++j) {
            for (int k = 0; k < res->noutput; ++k) {
                XRROutputInfo *oi = XRRGetOutputInfo(dpy, res, res->outputs[k]);
                if (!oi || oi->connection != RR_Connected || !oi->crtc || strcmp(oi->name, outputs[j]) != 0) { if (oi) XRRFreeOutputInfo(oi); continue; }
                XRRCrtcGamma *cg = XRRGetCrtcGamma(dpy, oi->crtc);
                if (!cg) { XRRFreeOutputInfo(oi); continue; }
                int sz = cg->size;
                XRRCrtcGamma *ng = XRRAllocGamma(sz);
                if (!ng) { XRRFreeGamma(cg); XRRFreeOutputInfo(oi); continue; }
                float ar = ab * ag.r, agv = ab * ag.g, abv = ab * ag.b;
                unsigned int ar65535 = (unsigned int)(ar * 65535.0f + 0.5f);
                unsigned int agv65535 = (unsigned int)(agv * 65535.0f + 0.5f);
                unsigned int abv65535 = (unsigned int)(abv * 65535.0f + 0.5f);
                    for (int l = 0; l < sz; ++l) {
                    unsigned int val = (unsigned int)l * 65535 / (sz - 1);
                    ng->red[l]   = ar65535 * val / 65535;
                    ng->green[l] = agv65535 * val / 65535;
                    ng->blue[l]  = abv65535 * val / 65535;
                    if (ng->red[l] > 65535) ng->red[l] = 65535;
                    if (ng->green[l] > 65535) ng->green[l] = 65535;
                    if (ng->blue[l] > 65535) ng->blue[l] = 65535;
               }
                XRRSetCrtcGamma(dpy, oi->crtc, ng);
                XRRFreeGamma(ng);
                XRRFreeGamma(cg);
                XRRFreeOutputInfo(oi);
            }
        }
        XFlush(dpy);
        usleep(10000);
    }
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
}







void toggle_night_light() {
    float vbr = get_brightness();
    char outputs[MAX_OUTPUTS][MAX_LINE];
    int output_count = get_connected_outputs(outputs);
    if (!output_count) return;
    int ibr = (int)(vbr * 10.0f + 0.5f);
    apply_settings(
        (ibr == 10) ? night : day,
        (ibr == 10) ? nightb : dayb,
        outputs, output_count, 10, intensity
    );
}




int is_night_light_on() {
    float vbr = get_brightness();
    int ibr = (int)(vbr * 10.0f + 0.5f);
    return ibr < 10;
}



void execute_command(const char *cmd) {
    if (cmd && cmd[0] != '\0') {
        pid_t pid = fork();
        if (pid == 0) {
            system(cmd);
            exit(0);
        }
    }
}




int execute_command_get_result(const char *cmd) {
    if (cmd && cmd[0] != '\0') {
        char full_cmd[MAX_LINE_LENGTH * 2];
        snprintf(full_cmd, sizeof(full_cmd), "%s%s", SCRIPT_PATH, cmd);
        FILE *fp = popen(full_cmd, "r");
        if (fp == NULL) {
            return 0;
        }
        char output[10];
        if (fgets(output, sizeof(output) - 1, fp) != NULL) {
            pclose(fp);
            return atoi(output);
        }
        pclose(fp);
    }
    return 0;
}


static void update_systray_icon(void);


static gboolean animate_notification_close(gpointer user_data) {
    NotificationAnimData *anim = (NotificationAnimData *)user_data;
    if (!anim) return FALSE;
    if (!GTK_IS_WIDGET(anim->button) || !gtk_widget_get_realized(anim->button)) {
        if (anim->is_animating) {
            if (anim_data && anim_data->window) regenerate_notifications(anim_data->window);
            update_systray_icon();
        }
        g_free(anim);
        return FALSE;
    }
    if (render_options.anim_type == ANIM_TYPE_NONE) {
        time_t timestamp = 0;
        if (GTK_IS_WIDGET(anim->button))
            timestamp = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(anim->button), "timestamp"));
        GtkWidget *popup = GTK_IS_WIDGET(anim->button) ? g_object_get_data(G_OBJECT(anim->button), "popup-window") : NULL;
        if (timestamp) {
            for (int i = 0; i < dbus_notif_store.count; i++) {
                if (dbus_notif_store.notifications[i].timestamp == timestamp) {
                    for (int j = i; j < dbus_notif_store.count - 1; j++)
                        dbus_notif_store.notifications[j] = dbus_notif_store.notifications[j + 1];
                    dbus_notif_store.count--;
                    if (new_notifs > 0) new_notifs--;
                    if (delete_notifications_button && anim_data && anim_data->is_opening && !anim_data->is_animating)
                      if (dbus_notif_store.count < 2) gtk_widget_hide(delete_notifications_button);
                    update_systray_icon();
                    break;
                }
            }
        }
        if (GTK_IS_WIDGET(anim->button)) gtk_widget_destroy(anim->button);
        if (popup && GTK_IS_WIDGET(popup)) {
            gtk_widget_destroy(popup);
            notification_popup = NULL;
            if (timeout_id != 0) {
                g_source_remove(timeout_id);
                timeout_id = 0;
            }
        }
        if (anim_data && anim_data->window) regenerate_notifications(anim_data->window);
        g_free(anim);
        return FALSE;
    }
    static float opacity = -1.0f;
    if (opacity < 0.0f) {
        int stored_opacity_int = GTK_IS_WIDGET(anim->button) ? 
            GPOINTER_TO_INT(g_object_get_data(G_OBJECT(anim->button), "start-opacity")) : 100;
        opacity = stored_opacity_int * 0.01f;
    }
    if (!anim->is_animating) {
        time_t timestamp = 0;
        if (GTK_IS_WIDGET(anim->button))
            timestamp = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(anim->button), "timestamp"));
        GtkWidget *popup = GTK_IS_WIDGET(anim->button) ? g_object_get_data(G_OBJECT(anim->button), "popup-window") : NULL;
        if (timestamp) {
            for (int i = 0; i < dbus_notif_store.count; i++) {
                if (dbus_notif_store.notifications[i].timestamp == timestamp) {
                    for (int j = i; j < dbus_notif_store.count - 1; j++)
                        dbus_notif_store.notifications[j] = dbus_notif_store.notifications[j + 1];
                    dbus_notif_store.count--;
                    if (new_notifs > 0) new_notifs--;
                    if (delete_notifications_button && anim_data && anim_data->is_opening && !anim_data->is_animating)
                        if (dbus_notif_store.count < 2) gtk_widget_hide(delete_notifications_button);
                    update_systray_icon();
                    break;
                }
            }
        }
        if (GTK_IS_WIDGET(anim->button)) gtk_widget_destroy(anim->button);
        if (popup && GTK_IS_WIDGET(popup)) {
            gtk_widget_destroy(popup);
            notification_popup = NULL;
            if (timeout_id != 0) {
                g_source_remove(timeout_id);
                timeout_id = 0;
            }
        }
        if (anim_data && anim_data->window) regenerate_notifications(anim_data->window);
        g_free(anim);
        opacity = -1.0f;
        return FALSE;
    }
    int scale_step_i = 50;
    int current_scale_i = anim->current_width * 1000 / anim->initial_width;
    if (current_scale_i > 400) {
        current_scale_i -= scale_step_i;
        if (current_scale_i < 400) current_scale_i = 400;
        anim->current_width = anim->initial_width * current_scale_i / 1000;
        anim->current_height = anim->initial_height * current_scale_i / 1000;
        gtk_widget_set_size_request(anim->button, anim->current_width, anim->current_height);
        gtk_widget_set_halign(anim->button, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(anim->button, GTK_ALIGN_CENTER);
    }
    int opacity_i = (int)(opacity * 1000);
    if (opacity_i > 300) {
        opacity_i -= 50;
        if (opacity_i < 300) opacity_i = 300;
        opacity = opacity_i / 1000.0f;
        gtk_widget_set_opacity(anim->button, opacity);
    }
    if (current_scale_i <= 400 && opacity_i <= 300) {
        anim->is_animating = FALSE;
    }
    gtk_widget_queue_draw(anim->button);
    return TRUE;
}




PangoFontDescription* get_system_font_description() {
    GtkSettings *settings = gtk_settings_get_default();
    gchar *font_name;
    g_object_get(settings, "gtk-font-name", &font_name, NULL);
    PangoFontDescription *font_desc = pango_font_description_from_string(font_name);
    g_free(font_name);
    return font_desc;
}



int find_config_value(FILE *file, const char *key, char *value, size_t value_size) {
    char line[MAX_LINE_LENGTH];
    size_t key_len = strlen(key);
    rewind(file);
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        if (line[0] == '\n' || line[0] == '#') continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        if ((size_t)(eq - line) != key_len) continue;
        if (strncmp(line, key, key_len) == 0) {
            char *val = eq + 1;
            char *nl = strchr(val, '\n');
            if (nl) *nl = 0;
            strncpy(value, val, value_size - 1);
            value[value_size - 1] = 0;
            return 1;
        }
    }
    return 0;
}



#pragma GCC push_options
#pragma GCC optimize ("Os")
void load_config() {
    char config_path[200];
    snprintf(config_path, sizeof(config_path), "/home/%s/.qsidebar/qsidebar.conf", username);
    FILE *config_file = fopen(config_path, "r");
    if (config_file == NULL) {
        g_print("Can't open configuration file %s: %s\n", config_path, strerror(errno));
        return;
    }
    strncpy(panel_title, "Actions center", MAX_LINE_LENGTH - 1);
    char line[MAX_LINE_LENGTH];
    int max_button_idx = -1;
    sidebar_flags &= ~FLAG_BACKLIGHT_CONTROL;
    sidebar_flags &= ~FLAG_HAS_BLUETOOTH;
    sidebar_flags &= ~FLAG_HAS_WIFI;
    sidebar_flags &= ~FLAG_PROJECT_EXTEND_FULL_HEIGHT;
    sidebar_flags &= ~FLAG_ROUNDEDBUTTONS;
    sidebar_flags &= ~FLAG_TRINITY_APPLET;
    sidebar_flags |= FLAG_TINT_IS_DEFAULT;
    sidebar_flags &= ~FLAG_BT_TESTDONE;
    sidebar_flags &= ~FLAG_WIFI_TESTDONE;
    for (int i = 0; i < MAX_BUTTONS; i++) {
        button_configs[i].name[0] = '\0';
        button_configs[i].type[0] = '\0';
        button_configs[i].icon_path[0] = '\0';
        button_configs[i].cmd[0] = '\0';
        button_configs[i].initstate_cmd[0] = '\0';
        button_configs[i].confirm_cmd = FALSE;
        button_configs[i].confirm_text[0] = '\0';
        button_configs[i].button = NULL;
        button_configs[i].is_active = FALSE;
        button_configs[i].is_preprogrammed = FALSE;
    }
    for (int i = 0; i < MAX_FILTERS; i++) {
        notif_filters[i].type[0] = '\0';
        notif_filters[i].string[0] = '\0';
        notif_filters[i].action[0] = '\0';
        notif_filters[i].exec[0] = '\0';
    }
    sidebar_flags |= FLAG_TINT_IS_DEFAULT;
    while (fgets(line, MAX_LINE_LENGTH, config_file) != NULL) {
        if (line[0] == '\n' || line[0] == '#')
            continue;
        line[strcspn(line, "\n")] = '\0';
        char key[MAX_LINE_LENGTH];
        char value[MAX_LINE_LENGTH];
        if (sscanf(line, "%[^=]=%[^\n]", key, value) != 2)
            continue;
        int button_num;
        char property[MAX_LINE_LENGTH];
        int filter_num;
if (strcmp(key, "opyion_notifs_popup_position") == 0) {
            if (strcmp(value, "default") == 0) {
                notif_pos = 0;
            } else if (strcmp(value, "topright") == 0) {
                notif_pos = 1;
            } else if (strcmp(value, "topleft") == 0) {
                notif_pos = 2;
            } else if (strcmp(value, "bottomleft") == 0) {
                notif_pos = 3;
            } else {
                notif_pos = 0;
            }
} else if (strcmp(key, "option_notif_low_timeout") == 0) {
            int timeout_value;
            if (sscanf(value, "%d", &timeout_value) == 1 && timeout_value >= 1 && timeout_value <= 300) {
                notif_low_timeout = timeout_value;
            } else {
                g_print("Invalid option_notif_low_timeout value: %s, using default (15)\n", value);
                notif_low_timeout = 15;
            }
        } else if (strcmp(key, "option_notif_normal_timeout") == 0) {
            int timeout_value;
            if (sscanf(value, "%d", &timeout_value) == 1 && timeout_value >= 1 && timeout_value <= 300) {
                notif_normal_timeout = timeout_value;
            } else {
                g_print("Invalid option_notif_normal_timeout value: %s, using default (30)\n", value);
                notif_normal_timeout = 30;
            }
        }
        else if (sscanf(key, "notif_filter_%d_%s", &filter_num, property) == 2) {
        int filter_idx = filter_num - 1;
        if (filter_idx >= MAX_FILTERS)
            continue;
    if (strcmp(property, "type") == 0) {
        if (strcmp(value, "title") == 0 || strcmp(value, "body") == 0 ||
            strcmp(value, "title+body") == 0 || strcmp(value, "app_name") == 0) {
            strncpy(notif_filters[filter_idx].type, value, MAX_LINE_LENGTH - 1);
        } else {
            g_print("Invalid notif_filter_%d_type value: %s, must be one of 'title', 'body', 'title+body', 'app_name'\n", filter_num, value);
            notif_filters[filter_idx].type[0] = '\0';
        }
    } else if (strcmp(property, "string") == 0) {
        if (strlen(value) > 0) {
            strncpy(notif_filters[filter_idx].string, value, MAX_LINE_LENGTH - 1);
        } else {
            g_print("Invalid notif_filter_%d_string value: %s, must not be empty\n", filter_num, value);
            notif_filters[filter_idx].string[0] = '\0';
        }
    } else if (strcmp(property, "action") == 0) {
        if (strcmp(value, "ignore") == 0 || strcmp(value, "accept_but_silent") == 0 ||
            strcmp(value, "set_urgent") == 0) {
            strncpy(notif_filters[filter_idx].action, value, MAX_LINE_LENGTH - 1);
        } else {
            g_print("Invalid notif_filter_%d_action value: %s, must be one of 'ignore', 'accept_but_silent', 'set_urgent'\n", filter_num, value);
            notif_filters[filter_idx].action[0] = '\0';
        }
    } else if (strcmp(property, "exec") == 0) {
        if (strlen(value) > 0) {
            strncpy(notif_filters[filter_idx].exec, value, MAX_LINE_LENGTH - 1);
        } else {
            g_print("Invalid notif_filter_%d_exec value: %s, must not be empty\n", filter_num, value);
            notif_filters[filter_idx].exec[0] = '\0';
        }
    }
}
else if (strcmp(key, "option_tint") == 0) {
    if (strcmp(value, "default") == 0) {
        render_options.tint_r = 0.90;
        render_options.tint_g = 0.90;
        render_options.tint_b = 0.90;
    } else if (strcmp(value, "dark") == 0) {
        render_options.tint_r = 0.25;
        render_options.tint_g = 0.25;
        render_options.tint_b = 0.25;
    } else if (strcmp(value, "tde") == 0) {
        FILE *fp;
        char path[1024];
        char result[256];
        snprintf(path, sizeof(path), "kreadconfig --file %s/share/config/kdeglobals --group General --key background", getenv("TDEHOME"));
        fp = popen(path, "r");
        if (fp == NULL) {
            render_options.tint_r = 0.90;
            render_options.tint_g = 0.90;
            render_options.tint_b = 0.90;
        } else {
            if (fgets(result, sizeof(result), fp) != NULL) {
                int r, g, b;
                if (sscanf(result, "%d,%d,%d", &r, &g, &b) == 3 &&
                    r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                    render_options.tint_r = r / 255.0;
                    render_options.tint_g = g / 255.0;
                    render_options.tint_b = b / 255.0;
                } else {
                    g_print("Invalid background value: %s, using default (225,225,225)\n", result);
                    render_options.tint_r = 0.90;
                    render_options.tint_g = 0.90;
                    render_options.tint_b = 0.90;
                }
            } else {
                render_options.tint_r = 0.90;
                render_options.tint_g = 0.90;
                render_options.tint_b = 0.90;
            }
            pclose(fp);
        }
    } else {
        int r, g, b;
        if (sscanf(value, "%d,%d,%d", &r, &g, &b) == 3 &&
            r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
            render_options.tint_r = r / 255.0;
            render_options.tint_g = g / 255.0;
            render_options.tint_b = b / 255.0;
        } else {
            g_print("Invalid option_tint value: %s, using default (225,225,225)\n", value);
            render_options.tint_r = 0.90;
            render_options.tint_g = 0.90;
            render_options.tint_b = 0.90;
        }
    }
} else if (strcmp(key, "option_panel_image") == 0) {
    if (strcmp(value, "none") == 0) {
        panel_background[0] = '\0';
    } else {
        FILE *file = fopen(value, "r");
        if (file != NULL) {
            fclose(file);
            strncpy(panel_background, value, sizeof(panel_background) - 1);
            panel_background[sizeof(panel_background) - 1] = '\0';
        } else {
            panel_background[0] = '\0';
        }
    }
}
 else if (strcmp(key, "option_opacity") == 0) {
            float opacity;
            if (sscanf(value, "%f", &opacity) == 1 && opacity >= 0.1 && opacity <= 1.0) {
                render_options.opacity = opacity;
            } else {
                g_print("Invalid option_opacity value: %s, using default (0.96)\n", value);
            }
        } else if (strcmp(key, "option_notif_popup_opacity") == 0) {
    if (sscanf(value, "%f", &popup_opacity) != 1 || popup_opacity < 0.1 || popup_opacity > 1.0) {
        g_print("Invalid notif popup opacity value: %s, using default (0.96)\n", value);
        popup_opacity = 0.96;
    }
} else if (strcmp(key, "option_notifs_popup_color") == 0) {
    if (strcmp(value, "default") == 0) {
        tint_popup_r = 255;
        tint_popup_g = 255;
        tint_popup_b = 255;
    } else {
        int r, g, b;
        if (sscanf(value, "%d,%d,%d", &r, &g, &b) == 3) {
            if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                tint_popup_r = r;
                tint_popup_g = g;
                tint_popup_b = b;
                sidebar_flags &= ~FLAG_TINT_IS_DEFAULT;
            } else {
                g_print("Invalid color value: %s, using default (255, 255, 255)\n", value);
                tint_popup_r = 255;
                tint_popup_g = 255;
                tint_popup_b = 255;
            }
        } else {
            g_print("Invalid color format: %s, using default (255, 255, 255)\n", value);
            tint_popup_r = 255;
            tint_popup_g = 255;
            tint_popup_b = 255;
        }
    }
} else if (strcmp(key, "option_nightlight_intensity") == 0) {
            float temp_intensity;
            if (sscanf(value, "%f", &temp_intensity) == 1 && temp_intensity >= 0.1 && temp_intensity <= 1.0) {
                intensity = temp_intensity;
            } else {
                g_print("Invalid option_nightlight_intensity value: %s, using default (0.8)\n", value);
                intensity = 0.8;
            }
        } else if (strcmp(key, "option_backlight_control") == 0) {
         int use_backlight;
         if (sscanf(value, "%d", &use_backlight) == 1 && (use_backlight == 0 || use_backlight == 1)) {
             if (use_backlight)
                 sidebar_flags |= FLAG_BACKLIGHT_CONTROL;
             else
                 sidebar_flags &= ~FLAG_BACKLIGHT_CONTROL;
         } else {
             g_print("Invalid option_backlight_control value: %s, using default (0)\n", value);
             sidebar_flags &= ~FLAG_BACKLIGHT_CONTROL;
         }
        } else if (strcmp(key, "option_notif_sound") == 0) {
    if (strcmp(value, "win10") == 0) {
        notification_sound = "/usr/share/qsidebar/sounds/notify_win10.wav";
    } else if (strcmp(value, "win11") == 0) {
        notification_sound = "/usr/share/qsidebar/sounds/notify_win11.wav";
    } else if (strcmp(value, "system") == 0) {
        notification_sound = "system";
    } else if (strcmp(value, "silent") == 0) {
        notification_sound = "silent";
    } else {
        if (value[0] == '/' && strstr(value, ".wav") != NULL) {
            char *custom_sound = strdup(value);
            if (custom_sound) {
                notification_sound = custom_sound;
            } 
        } else {
            g_print("Invalid option_notif_sound value: %s, using default\n", value);
        }
    }
 } else if (strcmp(key, "option_panel_anim_type") == 0) {
    if (strcmp(value, "fade") == 0) {
        render_options.anim_type = ANIM_TYPE_FADE;
    } else if (strcmp(value, "none") == 0) {
        render_options.anim_type = ANIM_TYPE_NONE;
    } else if (strcmp(value, "slide+fade") == 0) {
        render_options.anim_type = ANIM_TYPE_SLFD;
    } else {
        render_options.anim_type = ANIM_TYPE_SLIDE;
    }
 } else if (strcmp(key, "option_use_transparent_click") == 0) {
            int use_transparent;
            if (sscanf(value, "%d", &use_transparent) == 1) {
                render_options.use_transparent_click = (use_transparent != 0);
            } else {
                g_print("Invalid option_use_transparent_click value: %s, using default (1)\n", value);
                render_options.use_transparent_click = TRUE;
            }
        } else if (strcmp(key, "project_extend_full_panel_height") == 0) {
            int extend_full_height;
            if (sscanf(value, "%d", &extend_full_height) == 1 && extend_full_height == 1) {
                sidebar_flags |= FLAG_PROJECT_EXTEND_FULL_HEIGHT;
            }
        } else if (strcmp(key, "option_dark_mode") == 0) {
    int dark_mode_value;
    if (sscanf(value, "%d", &dark_mode_value) == 1 && (dark_mode_value == 0 || dark_mode_value == 1)) {
        if (dark_mode_value)
            sidebar_flags |= FLAG_DARKMODE;
        else
            sidebar_flags &= ~FLAG_DARKMODE;
    } else {
        g_print("Invalid option_dark_mode value: %s, using default (0)\n", value);
        sidebar_flags &= ~FLAG_DARKMODE;
    }
} else if (strcmp(key, "option_panel_image_solidbackground") == 0) {
 int solidbackground_value;
if (sscanf(value, "%d", &solidbackground_value) == 1 && (solidbackground_value == 0 || solidbackground_value == 1)) {
    if (solidbackground_value)
        sidebar_flags |= FLAG_PANEL_SOLIDBACKGROUND;
    else
        sidebar_flags &= ~FLAG_PANEL_SOLIDBACKGROUND;
} else {
    g_print("Invalid option_panel_image_solidbackground value: %s, using default (1)\n", value);
    sidebar_flags |= FLAG_PANEL_SOLIDBACKGROUND;
}
} else if (strcmp(key, "option_rounded_buttons") == 0) {
    int rounded_buttons_value;
    if (sscanf(value, "%d", &rounded_buttons_value) == 1 && (rounded_buttons_value == 0 || rounded_buttons_value == 1)) {
        if (rounded_buttons_value)
            sidebar_flags |= FLAG_ROUNDEDBUTTONS;
        else
            sidebar_flags &= ~FLAG_ROUNDEDBUTTONS;
    } else {
        g_print("Invalid option_rounded_buttons value: %s, using default (0)\n", value);
        sidebar_flags &= ~FLAG_ROUNDEDBUTTONS;
    }
}
else if (strcmp(key, "option_use_systray") == 0) {
    int use_systray_value;
    if (sscanf(value, "%d", &use_systray_value) == 1 && (use_systray_value == 0 || use_systray_value == 1)) {
        if (use_systray_value)
            sidebar_flags |= FLAG_USE_SYSTRAY;
        else
            sidebar_flags &= ~FLAG_USE_SYSTRAY;
    } else {
        g_print("Invalid option_use_systray value: %s, using default (1)\n", value);
        sidebar_flags |= FLAG_USE_SYSTRAY;
    }
} else if (strcmp(key, "option_tray_icon") == 0) {
            if (strcmp(value, "default") == 0) {
                tray_prefix="/usr/share/qsidebar/tray_icons/qsidebartray_";
                tray_icon_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_main.png";
                tray_icon_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_filled.png";
                tray_icon_focus_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_fmain.png";
                tray_icon_focus_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_ffilled.png";
                sidebar_flags &= ~FLAG_TRAYCOLORMODE;
            } else if (strcmp(value, "bell") == 0) {
                 tray_prefix="/usr/share/qsidebar/tray_icons/qsidebartray_b";
                tray_icon_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_bmain.png";
                tray_icon_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_bfilled.png";
                tray_icon_focus_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_bfmain.png";
                tray_icon_focus_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_bffilled.png";
		sidebar_flags &= ~FLAG_TRAYCOLORMODE;
            } else if (strcmp(value, "color") == 0) {
                tray_prefix="/usr/share/qsidebar/tray_icons/qsidebartray_c";
                tray_icon_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_cmain.png";
                tray_icon_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_cfilled.png";
                tray_icon_focus_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_cfmain.png";
                tray_icon_focus_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_cffilled.png";
		sidebar_flags |= FLAG_TRAYCOLORMODE;
            } else {
                tray_prefix="/usr/share/qsidebar/tray_icons/qsidebartray_";
                fprintf(stderr, "Invalid option_tray_icon value: %s, using default\n", value);
                tray_icon_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_main.png";
                tray_icon_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_filled.png";
                tray_icon_focus_normal = "/usr/share/qsidebar/tray_icons/qsidebartray_fmain.png";
                tray_icon_focus_filled = "/usr/share/qsidebar/tray_icons/qsidebartray_ffilled.png";
		sidebar_flags &= ~FLAG_TRAYCOLORMODE;
            }
        } else if (strcmp(key, "option_transparent_type") == 0) {
            if (strcmp(value, "DOCK") == 0) {
                transparent_click_type = 0;
            } else if (strcmp(value, "COMBO") == 0) {
                transparent_click_type = 1;
            } else if (strcmp(value, "UTILITY") == 0) {
                transparent_click_type = 2;
            } else if (strcmp(value, "DIALOG") == 0) {
                transparent_click_type = 3;
            } else if (strcmp(value, "MENU") == 0) {
                transparent_click_type = 4;
            } else {
                g_print("Invalid option_transparent_click_type value: %s, using default (COMBO)\n", value);
                transparent_click_type = 1;
            }
        } else if (strcmp(key, "option_transparent_click_mode") == 0) {
            if (strcmp(value, "ALL") == 0) {
                sidebar_flags |= FLAG_TRANSPARENT_CLICK_MODE;
            } else if (strcmp(value, "DESKTOP") == 0) {
                sidebar_flags &= ~FLAG_TRANSPARENT_CLICK_MODE;
            } else {
                g_print("Invalid option_transparent_click_mode value: %s, using default (1)\n", value);
                sidebar_flags |= FLAG_TRANSPARENT_CLICK_MODE;
            }
        } else if (strcmp(key, "option_bottom_margin") == 0) {
            int bottom_margin;
            if (sscanf(value, "%d", &bottom_margin) == 1 && bottom_margin >= 0 && bottom_margin <= 999) {
                panel_bottom_margin = bottom_margin;
            } else {
                g_print("Invalid option_bottom_margin value: %s, using default (0)\n", value);
                panel_bottom_margin = 0;
            }
        } else if (strcmp(key, "option_notif_font") == 0) {
            if (strcmp(value, "system") == 0) {
              font_desc = get_system_font_description();
             } else {
             font_desc = pango_font_description_from_string(value);
             if (font_desc == NULL) {
                font_desc = get_system_font_description();
             }
          }
        } else if (strcmp(key, "option_project_font") == 0) {
            if (strcmp(value, "system") == 0) {
              projectbuttons_font_desc = get_system_font_description();
             } else {
             projectbuttons_font_desc = pango_font_description_from_string(value);
             if (projectbuttons_font_desc == NULL) {
                projectbuttons_font_desc = get_system_font_description();
             }
          }
        } else if (strcmp(key, "option_panel_text_font") == 0) {
            if (strcmp(value, "system") == 0) {
              panel_font_desc = get_system_font_description();
             } else {
             panel_font_desc = pango_font_description_from_string(value);
             if (panel_font_desc == NULL) {
                panel_font_desc = get_system_font_description();
             }
          }
        } else if (strcmp(key, "option_quick_actions_font") == 0) {
             if (strcmp(value, "system") == 0) {
                 quickbuttons_font_desc = get_system_font_description();
              } else {
                 quickbuttons_font_desc = pango_font_description_from_string(value);
                 if (quickbuttons_font_desc == NULL) {
                    quickbuttons_font_desc = get_system_font_description();
                }
             }
        } else if (strcmp(key, "option_paneltitle_font") == 0) {
             if (strcmp(value, "system") == 0) {
                 paneltitlefont_desc = get_system_font_description();
              } else {
                 paneltitlefont_desc = pango_font_description_from_string(value);
                 if (paneltitlefont_desc == NULL) {
                    paneltitlefont_desc = get_system_font_description();
                }
             }
        } else if (strcmp(key, "option_panel_title") == 0) {
            if (strcmp(value, "default") == 0) {
                strncpy(panel_title, "Actions center", MAX_LINE_LENGTH - 1);
            } else {
                strncpy(panel_title, value, MAX_LINE_LENGTH - 1);
            }
        } else if (strcmp(key, "option_trinity_kicker_applet") == 0) {
            int trinity_value;
            if (sscanf(value, "%d", &trinity_value) == 1 && (trinity_value == 0 || trinity_value == 1)) {
if (trinity_value)
    sidebar_flags |= FLAG_TRINITY_APPLET;
else
    sidebar_flags &= ~FLAG_TRINITY_APPLET;
            } else {
                g_print("Invalid option_trinity_kicker_applet value: %s, using default (0)\n", value);
                 sidebar_flags &= ~FLAG_TRINITY_APPLET;
            }
        } else if (strcmp(key, "option_notif_number_indicator") == 0) {
            int notif_number_indicator_value;
         if (sscanf(value, "%d", &notif_number_indicator_value) == 1 && (notif_number_indicator_value == 0 || notif_number_indicator_value == 1)) {
             if (notif_number_indicator_value)
                 sidebar_flags |= FLAG_NOTIF_NUMBER_INDICATOR;
             else
                 sidebar_flags &= ~FLAG_NOTIF_NUMBER_INDICATOR;
         } else {
             g_print("Invalid option_notif_number_indicator value: %s, using default\n", value);
             sidebar_flags |= FLAG_NOTIF_NUMBER_INDICATOR;
         }
        } else if (strcmp(key, "option_notif_hide_icon") == 0) {
            int notif_hide_icon_value;
            if (sscanf(value, "%d", &notif_hide_icon_value) == 1 && (notif_hide_icon_value == 0 || notif_hide_icon_value == 1)) {
                if (notif_hide_icon_value)
                    sidebar_flags |= FLAG_NOTIF_HIDE_ICON;
                else
                    sidebar_flags &= ~FLAG_NOTIF_HIDE_ICON;
            } else {
                g_print("Invalid option_notif_hide_icon value: %s, using default\n", value);
                sidebar_flags &= ~FLAG_NOTIF_HIDE_ICON;
            }
        } else if (strcmp(key, "option_panel_anim_ease_effect") == 0) {
    int ease_effect_value;
    if (sscanf(value, "%d", &ease_effect_value) == 1 && (ease_effect_value == 0 || ease_effect_value == 1)) {
        if (ease_effect_value)
            sidebar_flags |= FLAG_EASE_EFFECT;
        else
            sidebar_flags &= ~FLAG_EASE_EFFECT;
    } else {
        g_print("Invalid option_panel_anim_ease_effect value: %s, using default (0)\n", value);
        sidebar_flags &= ~FLAG_EASE_EFFECT;
    }
        } else if (sscanf(key, "button_%d_%s", &button_num, property) == 2) {
    int button_idx = button_num - 1;
    if (button_idx >= MAX_BUTTONS)
        continue;
    if (button_idx > max_button_idx)
        max_button_idx = button_idx;
    button_configs[button_idx].icon_only = FALSE;
    if (strcmp(property, "name") == 0) {
        if (strcmp(value, "{wifi}") == 0) {
            if (!(sidebar_flags & FLAG_WIFI_TESTDONE)) {
                if (check_wifi_available()) {
                    sidebar_flags |= FLAG_HAS_WIFI;
                } else {
                    continue;
                }
            } else if (!(sidebar_flags & FLAG_HAS_WIFI)) {
                continue;
            }
        }
        if (strcmp(value, "{bluetooth}") == 0) {
            if (!(sidebar_flags & FLAG_BT_TESTDONE)) {
                if (check_bluetooth_available()) {
                    sidebar_flags |= FLAG_HAS_BLUETOOTH;
                } else {
                    continue;
                }
            } else if (!(sidebar_flags & FLAG_HAS_BLUETOOTH)) {
                continue;
            }
        }
        if (strcmp(value, "{wifi}") == 0) {
            button_configs[button_idx].is_preprogrammed = TRUE;
            strncpy(button_configs[button_idx].name, "Wifi", MAX_LINE_LENGTH - 1);
            strncpy(button_configs[button_idx].type, "toggle", MAX_LINE_LENGTH - 1);
        } else if (strcmp(value, "{airplane}") == 0) {
            if (!(sidebar_flags & FLAG_WIFI_TESTDONE)) {
                if (check_wifi_available()) {
                    sidebar_flags |= FLAG_HAS_WIFI;
                }
            }
            if (!(sidebar_flags & FLAG_BT_TESTDONE)) {
                if (check_bluetooth_available()) {
                    sidebar_flags |= FLAG_HAS_BLUETOOTH;
                }
            }
            if (!(sidebar_flags & FLAG_HAS_WIFI) && !(sidebar_flags & FLAG_HAS_BLUETOOTH)) {
                continue;
            }
            button_configs[button_idx].is_preprogrammed = TRUE;
            strncpy(button_configs[button_idx].name, "Airplane mode", MAX_LINE_LENGTH - 1);
            strncpy(button_configs[button_idx].type, "toggle", MAX_LINE_LENGTH - 1);
        } else if (strcmp(value, "{nightlight}") == 0) {
            button_configs[button_idx].is_preprogrammed = TRUE;
            strncpy(button_configs[button_idx].name, "Night light", MAX_LINE_LENGTH - 1);
            strncpy(button_configs[button_idx].type, "toggle", MAX_LINE_LENGTH - 1);
        } else if (strcmp(value, "{project}") == 0) {
            button_configs[button_idx].is_preprogrammed = TRUE;
            strncpy(button_configs[button_idx].name, "Project", MAX_LINE_LENGTH - 1);
            strncpy(button_configs[button_idx].type, "oneshot", MAX_LINE_LENGTH - 1);
        } else if (strcmp(value, "{bluetooth}") == 0) {
            button_configs[button_idx].is_preprogrammed = TRUE;
            strncpy(button_configs[button_idx].name, "Bluetooth", MAX_LINE_LENGTH - 1);
            strncpy(button_configs[button_idx].type, "toggle", MAX_LINE_LENGTH - 1);
        } else if (strcmp(value, "{focus}") == 0) {
            button_configs[button_idx].is_preprogrammed = TRUE;
            strncpy(button_configs[button_idx].name, "Focus Assist", MAX_LINE_LENGTH - 1);
            strncpy(button_configs[button_idx].type, "toggle", MAX_LINE_LENGTH - 1);
        } else {
            strncpy(button_configs[button_idx].name, value, MAX_LINE_LENGTH - 1);
        }
    } else if (strcmp(property, "type") == 0) {
        strncpy(button_configs[button_idx].type, value, MAX_LINE_LENGTH - 1);
    } else if (strcmp(property, "icon") == 0) {
        if (value[0] == '/') {
            strncpy(button_configs[button_idx].icon_path, value, MAX_LINE_LENGTH - 1);
        } else {
            snprintf(button_configs[button_idx].icon_path, MAX_LINE_LENGTH, "%s%s", ICON_PATH, value);
        }
    } else if (strcmp(property, "cmd") == 0) {
        strncpy(button_configs[button_idx].cmd, value, MAX_LINE_LENGTH - 1);
    } else if (strcmp(property, "initstate_cmd") == 0) {
        strncpy(button_configs[button_idx].initstate_cmd, value, MAX_LINE_LENGTH - 1);
    } else if (strcmp(property, "confirm_cmd") == 0) {
        if (button_configs[button_idx].is_preprogrammed || strcmp(button_configs[button_idx].type, "oneshot") != 0) {
            continue;
        }
        button_configs[button_idx].confirm_cmd = (strcmp(value, "1") == 0);
        if (!button_configs[button_idx].confirm_cmd) {
            continue;
        }
        char confirm_text_key[MAX_LINE_LENGTH];
        snprintf(confirm_text_key, sizeof(confirm_text_key), "button_%d_confirm_text", button_num);
        char confirm_text_value[MAX_LINE_LENGTH];
        if (find_config_value(config_file, confirm_text_key, confirm_text_value, sizeof(confirm_text_value))) {
            strncpy(button_configs[button_idx].confirm_text, confirm_text_value, MAX_LINE_LENGTH - 1);
        } else {
            g_print("Error: button_%d_confirm_cmd specified but button_%d_confirm_text is missing or empty\n", button_num, button_num);
            button_configs[button_idx].confirm_cmd = FALSE;
            button_configs[button_idx].confirm_text[0] = '\0';
        }
    } else if (strcmp(property, "icon_only") == 0) {
        button_configs[button_idx].icon_only = (strcmp(value, "1") == 0);
    }
}
    }
    fclose(config_file);
if (strlen(panel_background) == 0) {
    sidebar_flags |= FLAG_PANEL_SOLIDBACKGROUND;
}
    num_buttons = 0;
    for (int i = 0; i <= max_button_idx; i++) {
        if (button_configs[i].name[0] != '\0') {
            num_buttons++;
        }
    }
// g_print("Notification filters:\n");
// for (int i = 0; i < MAX_FILTERS; i++) {
//     g_print("Filter %d: type=%s, string=%s, action=%s, exec=%s\n",
//            i + 1,
//            notif_filters[i].type[0] != '\0' ? notif_filters[i].type : "N/A",
//            notif_filters[i].string[0] != '\0' ? notif_filters[i].string : "N/A",
//            notif_filters[i].action[0] != '\0' ? notif_filters[i].action : "N/A",
//            notif_filters[i].exec[0] != '\0' ? notif_filters[i].exec : "N/A");
// }
}
#pragma GCC pop_options


static GtkWidget* create_button_with_icon_and_label(ButtonConfig *config, int button_width, int button_height);
static void on_button_clicked(GtkWidget *widget, gpointer user_data);
gboolean recreate_original_buttons(gpointer user_data);
static GtkWidget* create_quick_settings_panel(int width, int height);


gboolean recreate_original_buttons(gpointer user_data) {
    GtkWidget *window = anim_data->window;
    GtkWidget *box = gtk_bin_get_child(GTK_BIN(window));
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));
    GList *iter = children;
    while (iter != NULL) {
        GtkWidget *child = GTK_WIDGET(iter->data);
        gtk_widget_destroy(child);
        iter = iter->next;
    }
    g_list_free(children);
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), header_box, FALSE, FALSE, 10);
    GtkWidget *label = gtk_label_new(panel_title);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_widget_override_font(label, paneltitlefont_desc);
    #pragma GCC diagnostic pop
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "actions-center-label");
    gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 0);
    add_notification_to_panel(box, anim_data->width, anim_data->height);
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), spacer, TRUE, TRUE, 0);
    GtkWidget *quick_settings = create_quick_settings_panel(anim_data->width, anim_data->height);
    gtk_box_pack_end(GTK_BOX(box), quick_settings, FALSE, FALSE, bottom_margin);
    gtk_widget_show_all(box);
    if (delete_notifications_button) {
        if (dbus_notif_store.count >= 2) {
            gtk_widget_show(delete_notifications_button);
        } else {
           gtk_widget_hide(delete_notifications_button);
        }
    }
    if (anim_data->update_pending) {
        regenerate_notifications(window);
        anim_data->update_pending = FALSE;
    }
    return FALSE;
}








void show_project_buttons(GtkWidget *parent_button);
void show_project_buttons(GtkWidget *parent_button) {
    GtkWidget *window = anim_data->window;
    GtkWidget *box = gtk_bin_get_child(GTK_BIN(window));
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), header_box, FALSE, FALSE, 10);
    GtkWidget *label = gtk_label_new("    PROJECT");
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "project-label");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 0);
    int margin = 0;
    int button_margin = 0;
    int width = anim_data->width;
    int height = anim_data->height;
    if (width <= 2 * margin || height <= bottom_margin + height*0.093) {
        return;
    }
    int usable_width = width;
    int usable_height = height - bottom_margin - height*0.093;
    int button_height = usable_height / 8;
    int button_width = usable_width;
    const char *button_names[] = {
        "PC screen only",
        "Duplicate",
        "Extend",
        "Second screen only"
    };
    GtkWidget *project_buttons[4];
    for (int i = 0; i < 4; i++) {
        ButtonConfig *config = g_new0(ButtonConfig, 1);
        strncpy(config->name, button_names[i], MAX_LINE_LENGTH - 1);
        strncpy(config->type, "oneshot", MAX_LINE_LENGTH - 1);
        config->is_preprogrammed = TRUE;
GtkWidget *button = create_button_with_icon_and_label(config, button_width, button_height);
        project_buttons[i] = button;
        g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), config);
        gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, button_margin);
    }
    const char *current_config = detect_display_configuration();
    for (int i = 0; i < 4; i++) {
        if (strcmp(button_names[i], current_config) == 0) {
            GtkStyleContext *context = gtk_widget_get_style_context(project_buttons[i]);
            gtk_style_context_add_class(context, "custom-toggle");
        } else {
            GtkStyleContext *context = gtk_widget_get_style_context(project_buttons[i]);
        gtk_style_context_add_class(context, "transparent-button");
  }
      }
int window_height = ((sidebar_flags & FLAG_EXTEND_MODE) && (sidebar_flags & FLAG_PROJECT_EXTEND_FULL_HEIGHT))
    ? get_secondary_height()
    : height;
gtk_window_set_default_size(GTK_WINDOW(window), width, window_height);
gtk_widget_show_all(window);
anim_data->is_project_panel = TRUE;
}





gboolean show_project_buttons_with_animation(gpointer user_data) {
    int target_x = anim_data->start_x - anim_data->width;
    show_project_buttons(NULL);
    if (render_options.anim_type == ANIM_TYPE_NONE) {
        anim_data->is_animating = FALSE;
        anim_data->is_opening = FALSE;
        anim_data->current_x = target_x;
        anim_data->current_opacity = render_options.opacity;
        gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->current_x, anim_data->y_position);
        gtk_widget_set_opacity(anim_data->window, anim_data->current_opacity);
        gtk_widget_show_all(anim_data->window);
        if (anim_data->click_window) {
            gtk_widget_show(anim_data->click_window);
        }
        anim_data->is_project_panel = TRUE;
        return FALSE;
    } else {
        anim_data->target_x = target_x;
        if (render_options.anim_type == ANIM_TYPE_SLIDE) {
            anim_data->current_x = anim_data->start_x;
            anim_data->current_opacity = render_options.opacity;
        } else if (render_options.anim_type == ANIM_TYPE_FADE) {
            anim_data->current_x = target_x;
            anim_data->current_opacity = 0.0f;
            anim_data->target_opacity = render_options.opacity;
        } else if (render_options.anim_type == ANIM_TYPE_SLFD) {
            anim_data->current_x = anim_data->start_x;
            anim_data->current_opacity = 0.0f;
            anim_data->target_opacity = render_options.opacity;
        }
        anim_data->is_animating = TRUE;
        anim_data->is_opening = TRUE;
        gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->current_x, anim_data->y_position);
        gtk_widget_set_opacity(anim_data->window, anim_data->current_opacity);
        gtk_widget_show_all(anim_data->window);
        if (anim_data->click_window) {
            gtk_widget_show(anim_data->click_window);
            if ((sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) && transparent_click_type != 0) {
                gtk_window_set_keep_above(GTK_WINDOW(anim_data->click_window), TRUE);
            }
        }
        anim_data->is_project_panel = TRUE;
        g_timeout_add(8, animate_window, anim_data);
        return FALSE;
    }
}



void on_confirmation_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    ButtonConfig *config = (ButtonConfig *)user_data;
    if (response_id == GTK_RESPONSE_ACCEPT) {
        execute_command(config->cmd);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    confirmation_dialog = NULL;
}




void show_confirmation_dialog(ButtonConfig *config, GtkWidget *parent_widget) {
    GtkWidget *parent_window = gtk_widget_get_toplevel(parent_widget);
    if (!GTK_IS_WINDOW(parent_window)) {
        parent_window = NULL;
    }
    confirmation_dialog = gtk_dialog_new_with_buttons(
        "Confirmation",
        GTK_WINDOW(parent_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Ok",
        GTK_RESPONSE_ACCEPT,
        "_Cancel",
        GTK_RESPONSE_REJECT,
        NULL
    );
     gtk_widget_set_name(confirmation_dialog, "confirm-dialog");
    gtk_window_set_icon_from_file(GTK_WINDOW(confirmation_dialog), config->icon_path, NULL);
    gtk_window_set_position(GTK_WINDOW(confirmation_dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(confirmation_dialog), 350, 120);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(confirmation_dialog));
        char *label_text = g_strdup_printf("\n%s", config->confirm_text);
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_set_homogeneous(GTK_BOX(box), FALSE);
    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(content_area);
    g_signal_connect(confirmation_dialog, "response", G_CALLBACK(on_confirmation_response), config);
    gtk_widget_show(confirmation_dialog);
}







static void on_button_clicked(GtkWidget *widget, gpointer user_data) {
    ButtonConfig *config = (ButtonConfig *)user_data;
    if (strcmp(config->type, "toggle") == 0) {
        if (config->is_preprogrammed) {
            if (strcmp(config->name, "Wifi") == 0) {
                if (set_wifi_status(!config->is_active)) {
                    config->is_active = !config->is_active;
                    if (config->is_active) {
                        for (int i = 0; i < MAX_BUTTONS; i++) {
                            if (strcmp(button_configs[i].name, "Airplane mode") == 0) {
                                GtkStyleContext *context = gtk_widget_get_style_context(button_configs[i].button);
                                gtk_style_context_remove_class(context, "active-toggle");
                                button_configs[i].is_active = FALSE;
                                update_button_icon(button_configs[i].button, button_configs[i].is_active);
                                gtk_widget_queue_draw(button_configs[i].button);
                            }
                        }
                    }
                }
            } else if (strcmp(config->name, "Airplane mode") == 0) {
                if (set_airplane_mode(!config->is_active)) {
                    config->is_active = !config->is_active;
                    for (int i = 0; i < MAX_BUTTONS; i++) {
                        if (strcmp(button_configs[i].name, "Wifi") == 0 || strcmp(button_configs[i].name, "Bluetooth") == 0) {
                            GtkStyleContext *context = gtk_widget_get_style_context(button_configs[i].button);
                            button_configs[i].is_active = !config->is_active;
                            if (button_configs[i].is_active) {
                                gtk_style_context_add_class(context, "active-toggle");
                            } else {
                                gtk_style_context_remove_class(context, "active-toggle");
                            }
                            update_button_icon(button_configs[i].button, button_configs[i].is_active);
                            gtk_widget_queue_draw(button_configs[i].button);
                        }
                    }
                }
            } else if (strcmp(config->name, "Night light") == 0) {
                toggle_night_light();
                config->is_active = !config->is_active;
            } else if (strcmp(config->name, "Bluetooth") == 0) {
                gboolean target_state = !config->is_active;
                if (set_bluetooth_status(target_state)) {
                    if (target_state) {
                        execute_command("bluetoothctl power on > /dev/null 2>&1");
                    }
                    config->is_active = target_state;
                }
            } else if (strcmp(config->name, "Focus Assist") == 0) {
               sidebar_flags ^= FLAG_FOCUS_ASSIST;
               config->is_active = (sidebar_flags & FLAG_FOCUS_ASSIST) != 0;
               update_systray_icon();
           }
            GtkStyleContext *context = gtk_widget_get_style_context(widget);
            if (config->is_active) {
                gtk_style_context_add_class(context, "active-toggle");
            } else {
                gtk_style_context_remove_class(context, "active-toggle");
            }
            update_button_icon(widget, config->is_active);
            gtk_widget_queue_draw(widget);
            return;
        }
    }
    if (config->is_preprogrammed) {
        if (strcmp(config->name, "Project") == 0) {
            if (!anim_data->is_animating) {
                if (render_options.anim_type == ANIM_TYPE_NONE) {
                    anim_data->current_x = anim_data->start_x;
                    gtk_widget_hide(anim_data->window);
                    if (anim_data->click_window) {
                        gtk_widget_hide(anim_data->click_window);
                    }
                    show_project_buttons(NULL);
                    anim_data->current_x = anim_data->start_x - anim_data->width;
                    anim_data->current_opacity = render_options.opacity;
                    gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->current_x, anim_data->y_position);
                    gtk_widget_set_opacity(anim_data->window, anim_data->current_opacity);
                    gtk_widget_show_all(anim_data->window);
                    if (anim_data->click_window) {
                        gtk_widget_show(anim_data->click_window);
                        if ((sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) && transparent_click_type != 0) {
                            gtk_window_set_keep_above(GTK_WINDOW(anim_data->click_window), TRUE);
                        }
                    }
                    anim_data->is_project_panel = TRUE;
                    anim_data->is_opening = TRUE;
                } else {
                    anim_data->target_x = anim_data->start_x;
                    anim_data->is_animating = TRUE;
                    anim_data->is_opening = FALSE;
                    g_timeout_add(8, animate_window, anim_data);
                    g_timeout_add(250, (GSourceFunc)show_project_buttons_with_animation, NULL);
                }
            }
            return;
        } else if (strcmp(config->name, "PC screen only") == 0) {
            handle_project_action("PC screen only");
        } else if (strcmp(config->name, "Duplicate") == 0) {
            handle_project_action("Duplicate");
        } else if (strcmp(config->name, "Extend") == 0) {
            handle_project_action("Extend");
        } else if (strcmp(config->name, "Second screen only") == 0) {
            handle_project_action("Second screen only");
        }
        if (render_options.anim_type == ANIM_TYPE_NONE) {
            anim_data->current_x = anim_data->start_x;
            anim_data->current_opacity = 0.0;
            gtk_widget_hide(anim_data->window);
            if (anim_data->click_window) {
                gtk_widget_hide(anim_data->click_window);
            }
            anim_data->is_opening = FALSE;
            if (strcmp(config->name, "PC screen only") == 0 ||
                strcmp(config->name, "Duplicate") == 0 ||
                strcmp(config->name, "Extend") == 0 ||
                strcmp(config->name, "Second screen only") == 0) {
                recreate_original_buttons(NULL);
            }
        } else {
            if (render_options.anim_type == ANIM_TYPE_SLIDE) {
                anim_data->target_x = anim_data->start_x;
            } else if (render_options.anim_type == ANIM_TYPE_FADE) {
                anim_data->target_opacity = 0.0;
            } else if (render_options.anim_type == ANIM_TYPE_SLFD) {
                anim_data->target_x = anim_data->start_x;
                anim_data->target_opacity = 0.0;
            }
            anim_data->is_animating = TRUE;
            anim_data->is_opening = FALSE;
            g_timeout_add(8, animate_window, anim_data);
            if (strcmp(config->name, "PC screen only") == 0 ||
                strcmp(config->name, "Duplicate") == 0 ||
                strcmp(config->name, "Extend") == 0 ||
                strcmp(config->name, "Second screen only") == 0) {
                g_timeout_add(200, (GSourceFunc)recreate_original_buttons, NULL);
            }
        }
    } else {
        if (config->confirm_cmd) {
            show_confirmation_dialog(config, gtk_widget_get_toplevel(widget));
        } else {
            execute_command(config->cmd);
        }
    }
    if ((strcmp(config->type, "oneshot") == 0 && anim_data != NULL) ||
        (config->is_preprogrammed && (strcmp(config->name, "PC screen only") == 0 ||
                                      strcmp(config->name, "Duplicate") == 0 ||
                                      strcmp(config->name, "Extend") == 0 ||
                                      strcmp(config->name, "Second screen only") == 0))) {
        if (render_options.anim_type == ANIM_TYPE_NONE) {
            anim_data->current_x = anim_data->start_x;
            anim_data->current_opacity = 0.0;
            gtk_widget_hide(anim_data->window);
            if (anim_data->click_window) {
                gtk_widget_hide(anim_data->click_window);
            }
            anim_data->is_opening = FALSE;
        } else {
            if (render_options.anim_type == ANIM_TYPE_SLIDE) {
                anim_data->target_x = anim_data->start_x;
            } else if (render_options.anim_type == ANIM_TYPE_FADE) {
                anim_data->target_opacity = 0.0;
            } else if (render_options.anim_type == ANIM_TYPE_SLFD) {
                anim_data->target_x = anim_data->start_x;
                anim_data->target_opacity = 0.0;
            }
            anim_data->is_animating = TRUE;
            anim_data->is_opening = FALSE;
            g_timeout_add(8, animate_window, anim_data);
        }
    }
}




static void on_window_destroy(GtkWidget *widget __attribute__((unused)), gpointer data __attribute__((unused))) {
    if (!restart_requested && gtk_main_level() > 0) {
        gtk_main_quit();
    }
}



GdkPixbuf *invert_pixbuf_colors(GdkPixbuf *original) {
    if (!original || !gdk_pixbuf_get_has_alpha(original)) return NULL;
    int w = gdk_pixbuf_get_width(original), h = gdk_pixbuf_get_height(original);
    int rs = gdk_pixbuf_get_rowstride(original), nc = gdk_pixbuf_get_n_channels(original);
    GdkPixbuf *inv = gdk_pixbuf_copy(original);
    guchar *px = gdk_pixbuf_get_pixels(inv);
    for (int y = 0; y < h; ++y) {
        guchar *p = px + y * rs;
        guchar *end = p + w * nc;
        while (p < end) {
            p[0] = 255 - p[0];
            p[1] = 255 - p[1];
            p[2] = 255 - p[2];
            p += nc;
        }
    }
    return inv;
}


void update_button_icon(GtkWidget *button, gboolean is_active) {
    GtkWidget *image = g_object_get_data(G_OBJECT(button), "icon-image");
    if (!image) return;
    GdkPixbuf *pixbuf = NULL;
    if (is_active) {
        pixbuf = g_object_get_data(G_OBJECT(button), "inverted-pixbuf");
    } else {
        pixbuf = g_object_get_data(G_OBJECT(button), "normal-pixbuf");
    }
    if (pixbuf) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
    }
}



static GtkWidget* create_button_with_icon_and_label(ButtonConfig *config, int button_width, int button_height) {
    GtkWidget *button = gtk_button_new();
    config->button = button;
    if (button_width < 0 || button_height < 0) {
        button_width = 100;
        button_height = 50;
    }
    GtkWidget *box;
    if (config->is_preprogrammed && (strcmp(config->name, "PC screen only") == 0 ||
                                     strcmp(config->name, "Duplicate") == 0 ||
                                     strcmp(config->name, "Extend") == 0 ||
                                     strcmp(config->name, "Second screen only") == 0)) {
        box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    } else {
        box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    }
    gtk_container_add(GTK_CONTAINER(button), box);

    if (config->is_preprogrammed && (strcmp(config->name, "PC screen only") == 0 ||
                                     strcmp(config->name, "Duplicate") == 0 ||
                                     strcmp(config->name, "Extend") == 0 ||
                                     strcmp(config->name, "Second screen only") == 0)) {
        const char *icon_path = NULL;
        if (strcmp(config->name, "PC screen only") == 0) {
            icon_path = "/usr/share/qsidebar/icons/project_pconly.png";
        } else if (strcmp(config->name, "Duplicate") == 0) {
            icon_path = "/usr/share/qsidebar/icons/project_duplicate.png";
        } else if (strcmp(config->name, "Extend") == 0) {
            icon_path = "/usr/share/qsidebar/icons/project_extend.png";
        } else if (strcmp(config->name, "Second screen only") == 0) {
            icon_path = "/usr/share/qsidebar/icons/project_2ndonly.png";
        }
        if (icon_path) {
            GdkPixbuf *original_pixbuf = gdk_pixbuf_new_from_file(icon_path, NULL);
            if (original_pixbuf) {
                int icon_width = button_width / 4;
                int original_width = gdk_pixbuf_get_width(original_pixbuf);
                int original_height = gdk_pixbuf_get_height(original_pixbuf);
                int icon_height = (icon_width * original_height) / original_width;
                GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(
                    original_pixbuf, icon_width, icon_height, GDK_INTERP_BILINEAR);
   GdkPixbuf *display_pixbuf = scaled_pixbuf;
   if (sidebar_flags & FLAG_DARKMODE) {
    GdkPixbuf *inverted_pixbuf = invert_pixbuf_colors(scaled_pixbuf);
    if (inverted_pixbuf) {
        display_pixbuf = inverted_pixbuf;
        g_object_unref(scaled_pixbuf);
     }
  }
            GtkWidget *image = gtk_image_new_from_pixbuf(display_pixbuf);
             gtk_widget_set_halign(image, GTK_ALIGN_START);
             gtk_widget_set_margin_end(image, 15);
             gtk_widget_set_margin_start(image, 15);
             gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
             g_object_unref(display_pixbuf);
             g_object_unref(original_pixbuf);
           }
       }
        GtkWidget *label = gtk_label_new(config->name);
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        gtk_widget_override_font(label, projectbuttons_font_desc);
        #pragma GCC diagnostic pop
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
        char markup[MAX_LINE_LENGTH + 50];
        snprintf(markup, sizeof(markup), "<span size='larger'>%s</span>", config->name);
        gtk_label_set_markup(GTK_LABEL(label), markup);
        gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
     } else {
        GdkPixbuf *original_pixbuf = NULL;
        GdkPixbuf *normal_pixbuf = NULL;
        GdkPixbuf *inverted_pixbuf = NULL;
        if (config->icon_path[0] != '\0') {
            original_pixbuf = gdk_pixbuf_new_from_file(config->icon_path, NULL);
            if (original_pixbuf) {
                int icon_width = button_width / 4;
                if (config->icon_only) {
                    icon_width = button_width * 0.6;
                }
                int original_width = gdk_pixbuf_get_width(original_pixbuf);
                int original_height = gdk_pixbuf_get_height(original_pixbuf);
                int icon_height = (icon_width * original_height) / original_width;
                GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(
                    original_pixbuf, icon_width, icon_height, GDK_INTERP_BILINEAR);
                if (config->icon_only) {
                    gtk_widget_set_tooltip_text(button, config->name);
                    GdkPixbuf *display_pixbuf = scaled_pixbuf;
       if (sidebar_flags & FLAG_DARKMODE) {
          inverted_pixbuf = invert_pixbuf_colors(scaled_pixbuf);
              if (inverted_pixbuf) {
                   display_pixbuf = inverted_pixbuf;
              }
       }
                GtkWidget *image = gtk_image_new_from_pixbuf(display_pixbuf);
                 gtk_widget_set_halign(image, GTK_ALIGN_CENTER);
                 gtk_widget_set_valign(image, GTK_ALIGN_CENTER);
                 gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);
                   if (inverted_pixbuf && inverted_pixbuf != display_pixbuf) {
                     g_object_unref(inverted_pixbuf);
                    }
                } else {
                    if (strcmp(config->type, "toggle") == 0) {
                        if (sidebar_flags & FLAG_DARKMODE) {
                            normal_pixbuf = invert_pixbuf_colors(scaled_pixbuf);
                            inverted_pixbuf = g_object_ref(scaled_pixbuf);
                        } else {
                            normal_pixbuf = g_object_ref(scaled_pixbuf);
                            inverted_pixbuf = invert_pixbuf_colors(scaled_pixbuf);
                        }
                        g_object_set_data_full(G_OBJECT(button), "normal-pixbuf",
                                              g_object_ref(normal_pixbuf), g_object_unref);
                        if (inverted_pixbuf) {
                            g_object_set_data_full(G_OBJECT(button), "inverted-pixbuf",
                                                  g_object_ref(inverted_pixbuf), g_object_unref);
                        }
                        GtkWidget *image = gtk_image_new_from_pixbuf(normal_pixbuf);
                        gtk_widget_set_halign(image, GTK_ALIGN_START);
                        gtk_widget_set_valign(image, GTK_ALIGN_START);
                        g_object_set_data(G_OBJECT(button), "icon-image", image);
                        gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
                    } else {
                        GdkPixbuf *display_pixbuf = scaled_pixbuf;
                        if (sidebar_flags & FLAG_DARKMODE) {
                            inverted_pixbuf = invert_pixbuf_colors(scaled_pixbuf);
                            if (inverted_pixbuf) {
                                display_pixbuf = inverted_pixbuf;
                            }
                        }
                        GtkWidget *image = gtk_image_new_from_pixbuf(display_pixbuf);
                        gtk_widget_set_halign(image, GTK_ALIGN_START);
                        gtk_widget_set_valign(image, GTK_ALIGN_START);
                        g_object_set_data(G_OBJECT(button), "icon-image", image);
                        gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
                        if (inverted_pixbuf && inverted_pixbuf != display_pixbuf) {
                            g_object_unref(inverted_pixbuf);
                        }
                    }
                    char button_text[MAX_LINE_LENGTH];
                    strncpy(button_text, config->name, MAX_LINE_LENGTH - 1);
                    char *space_pos = strchr(button_text, ' ');
                    if (space_pos != NULL) {
                        *space_pos = '\0';
                        GtkWidget *label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
                        gtk_box_pack_start(GTK_BOX(box), label_box, FALSE, FALSE, 0);
                        GtkWidget *first_label = gtk_label_new(button_text);
                        #pragma GCC diagnostic push
                        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                        gtk_widget_override_font(first_label, quickbuttons_font_desc);
                        #pragma GCC diagnostic pop
                        gtk_widget_set_halign(first_label, GTK_ALIGN_START);
                        gtk_box_pack_start(GTK_BOX(label_box), first_label, FALSE, FALSE, 0);
                        GtkWidget *second_label = gtk_label_new(space_pos + 1);
                        #pragma GCC diagnostic push
                        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                        gtk_widget_override_font(second_label, quickbuttons_font_desc);
                        #pragma GCC diagnostic pop
                        gtk_widget_set_halign(second_label, GTK_ALIGN_START);
                        gtk_box_pack_start(GTK_BOX(label_box), second_label, FALSE, FALSE, 0);
                    } else {
                        GtkWidget *label = gtk_label_new(config->name);
                        #pragma GCC diagnostic push
                        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                        gtk_widget_override_font(label, quickbuttons_font_desc);
                        #pragma GCC diagnostic pop
                        gtk_widget_set_halign(label, GTK_ALIGN_START);
                        gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
                    }
                }
                g_object_unref(scaled_pixbuf);
                g_object_unref(original_pixbuf);
                if (normal_pixbuf) g_object_unref(normal_pixbuf);
                if (inverted_pixbuf) g_object_unref(inverted_pixbuf);
            }
        }
        gtk_style_context_add_class(gtk_widget_get_style_context(button), "original-button");
    }
    gtk_widget_set_size_request(button, button_width, button_height);
    return button;
}









static void on_notification_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *button = GTK_WIDGET(widget);
    GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
    NotificationAnimData *anim = g_new(NotificationAnimData, 1);
    anim->button = button;
    GtkRequisition natural_size;
    gtk_widget_get_preferred_size(button, NULL, &natural_size);
    anim->initial_width = natural_size.width;
    anim->initial_height = natural_size.height;
    anim->current_width = natural_size.width;
    anim->current_height = natural_size.height;
    anim->initial_x = gtk_widget_get_margin_start(button);
    anim->initial_y = gtk_widget_get_margin_top(button);
    anim->panel_width = GPOINTER_TO_INT(data);
    anim->is_animating = TRUE;
    g_object_set_data(G_OBJECT(anim->button), "start-opacity", GINT_TO_POINTER((int)(render_options.opacity * 100)));
    if (toplevel == notification_popup) {
        g_object_set_data(G_OBJECT(anim->button), "popup-window", toplevel);
    }
    g_timeout_add(16, animate_notification_close, anim);
}








static GtkWidget* create_notification_button(const Notification *notif, int panel_width, int panel_height, gboolean show_date) {
    GtkWidget *button = gtk_button_new();
    int button_width = panel_width - panel_width * 0.095;
    int button_height = panel_height / 10;
    gtk_widget_set_size_request(button, button_width, button_height);
    GtkStyleContext *context = gtk_widget_get_style_context(button);
    gtk_style_context_add_class(context, "notification-button");
    if (notif->urgency == 2) {
        gtk_style_context_add_class(context, "notification-critical");
    }
    g_object_set_data(G_OBJECT(button), "timestamp", GINT_TO_POINTER(notif->timestamp));
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(button), hbox);
    int icon_width = 0;
    if (!(sidebar_flags & FLAG_NOTIF_HIDE_ICON) && strlen(notif->icon) > 0) {
        GdkPixbuf *pixbuf = NULL;
        if (strncmp(notif->icon, "data:image/", 11) == 0) {
            const char *base64_prefix = "base64,";
            char *base64_data = strstr(notif->icon, base64_prefix);
            if (base64_data) {
                base64_data += strlen(base64_prefix);
                gsize decoded_len = 0;
                guchar *decoded_data = g_base64_decode(base64_data, &decoded_len);
                if (decoded_data && decoded_len > 0) {
                    GInputStream *stream = g_memory_input_stream_new_from_data(decoded_data, decoded_len, g_free);
                    pixbuf = gdk_pixbuf_new_from_stream(stream, NULL, NULL);
                    g_object_unref(stream);
                }
            }
        } else {
            pixbuf = gdk_pixbuf_new_from_file(notif->icon, NULL);
        }
        if (pixbuf) {
            int icon_height = button_height / 2;
            icon_width = gdk_pixbuf_get_width(pixbuf) * icon_height / gdk_pixbuf_get_height(pixbuf);
            GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, icon_width, icon_height, GDK_INTERP_BILINEAR);
            GtkWidget *image = gtk_image_new_from_pixbuf(scaled_pixbuf);
            gtk_widget_set_valign(image, GTK_ALIGN_CENTER);
            gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 5);
            g_object_unref(pixbuf);
            g_object_unref(scaled_pixbuf);
        }
    }
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
    int margin_start = 10;
    int margin_end = 10;
    int hbox_spacing = 10;
    int text_width = button_width - margin_start - margin_end - icon_width - (icon_width > 0 ? hbox_spacing : 0);
    PangoLayout *layout = gtk_widget_create_pango_layout(button, NULL);
    pango_layout_set_font_description(layout, font_desc);
    char title_markup[MAX_LINE_LENGTH + 20];
    snprintf(title_markup, sizeof(title_markup), "%s", notif->title);
    GtkWidget *title = gtk_label_new(NULL);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_widget_override_font(title, font_desc);
    #pragma GCC diagnostic pop
    gtk_label_set_markup(GTK_LABEL(title), title_markup);
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    gboolean is_truncated = FALSE;
    char full_text[2 * MAX_LINE_LENGTH + 50];
    snprintf(full_text, sizeof(full_text), "%s\n", notif->title);
    const char *content = notif->content;
    if (strlen(content) > 0) {
        char first_line[MAX_LINE_LENGTH] = {0};
        char second_line[MAX_LINE_LENGTH] = {0};
        gboolean has_newline = FALSE;
        const char *newline_pos = strstr(content, "\\n");
        if (newline_pos) {
            has_newline = TRUE;
            size_t first_len = newline_pos - content;
            if (first_len >= MAX_LINE_LENGTH) first_len = MAX_LINE_LENGTH - 1;
            strncpy(first_line, content, first_len);
            first_line[first_len] = '\0';
            const char *remaining = newline_pos + 2;
            const char *second_newline_pos = strstr(remaining, "\\n");
            size_t second_len;
            if (second_newline_pos) {
                second_len = second_newline_pos - remaining;
                if (second_len >= MAX_LINE_LENGTH - 6) second_len = MAX_LINE_LENGTH - 7;
                strncpy(second_line, remaining, second_len);
                second_line[second_len] = '\0';
                strcat(second_line, " [...]");
                is_truncated = TRUE;
            } else {
                second_len = strlen(remaining);
                if (second_len >= MAX_LINE_LENGTH) second_len = MAX_LINE_LENGTH - 1;
                strncpy(second_line, remaining, second_len);
                second_line[second_len] = '\0';
            }
            strncat(full_text, content, sizeof(full_text) - strlen(full_text) - 1);
        } else {
            strncpy(first_line, content, MAX_LINE_LENGTH - 1);
            first_line[MAX_LINE_LENGTH - 1] = '\0';
            strncat(full_text, content, sizeof(full_text) - strlen(full_text) - 1);
        }
        pango_layout_set_text(layout, first_line, -1);
        int layout_width, layout_height;
        pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
        if (layout_width <= text_width || has_newline) {
            GtkWidget *first_label = gtk_label_new(first_line);
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            gtk_widget_override_font(first_label, font_desc);
            #pragma GCC diagnostic pop
            gtk_widget_set_halign(first_label, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(vbox), first_label, FALSE, FALSE, 0);
        } else {
            int char_count = strlen(first_line);
            int cut_pos = 0;
            for (int i = 1; i <= char_count; i++) {
                pango_layout_set_text(layout, first_line, i);
                pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
                if (layout_width > text_width) {
                    cut_pos = i - 1;
                    while (cut_pos > 0 && first_line[cut_pos] != ' ' && first_line[cut_pos] != '\0') {
                        cut_pos--;
                    }
                    if (cut_pos == 0) cut_pos = i - 1;
                    break;
                }
            }
            if (cut_pos == 0) cut_pos = char_count;
            char temp[MAX_LINE_LENGTH];
            strncpy(temp, first_line, cut_pos);
            temp[cut_pos] = '\0';
            GtkWidget *first_label = gtk_label_new(temp);
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            gtk_widget_override_font(first_label, font_desc);
            #pragma GCC diagnostic pop
            gtk_widget_set_halign(first_label, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(vbox), first_label, FALSE, FALSE, 0);
            const char *remaining = first_line + cut_pos + (first_line[cut_pos] == ' ' ? 1 : 0);
            strncpy(second_line, remaining, MAX_LINE_LENGTH - 1);
            second_line[MAX_LINE_LENGTH - 1] = '\0';
            is_truncated = TRUE;
        }
        if (second_line[0] != '\0') {
            pango_layout_set_text(layout, second_line, -1);
            pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
            if (layout_width <= text_width) {
                GtkWidget *second_label = gtk_label_new(second_line);
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                gtk_widget_override_font(second_label, font_desc);
                 #pragma GCC diagnostic pop
                gtk_widget_set_halign(second_label, GTK_ALIGN_START);
                gtk_box_pack_start(GTK_BOX(vbox), second_label, FALSE, FALSE, 0);
            } else {
                int char_count = strlen(second_line);
                int cut_pos = 0;
                for (int i = 1; i <= char_count; i++) {
                    pango_layout_set_text(layout, second_line, i);
                    pango_layout_get_pixel_size(layout, &layout_width, &layout_height);
                    if (layout_width > text_width) {
                        cut_pos = i - 1;
                        while (cut_pos > 0 && second_line[cut_pos] != ' ' && second_line[cut_pos] != '\0') {
                            cut_pos--;
                        }
                        if (cut_pos == 0) cut_pos = i - 1;
                        break;
                    }
                }
                if (cut_pos == 0) cut_pos = char_count;
                if (cut_pos > 6) {
                    cut_pos -= 6;
                    char temp[MAX_LINE_LENGTH];
                    strncpy(temp, second_line, cut_pos);
                    temp[cut_pos] = '\0';
                    char marked_up_text[MAX_LINE_LENGTH + 50];
                    snprintf(marked_up_text, sizeof(marked_up_text), "%s [...]", temp);
                    GtkWidget *second_label = gtk_label_new(NULL);
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    gtk_widget_override_font(second_label, font_desc);
                    #pragma GCC diagnostic pop
                    gtk_label_set_markup(GTK_LABEL(second_label), marked_up_text);
                    gtk_widget_set_halign(second_label, GTK_ALIGN_START);
                    gtk_box_pack_start(GTK_BOX(vbox), second_label, FALSE, FALSE, 0);
                    is_truncated = TRUE;
                } else {
                    char temp[MAX_LINE_LENGTH];
                    strncpy(temp, second_line, cut_pos);
                    temp[cut_pos] = '\0';
                    GtkWidget *second_label = gtk_label_new(temp);
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    gtk_widget_override_font(second_label, font_desc);
                    #pragma GCC diagnostic pop
                    gtk_widget_set_halign(second_label, GTK_ALIGN_START);
                    gtk_box_pack_start(GTK_BOX(vbox), second_label, FALSE, FALSE, 0);
                }
            }
        }
    }
    struct tm *tm_info = localtime(&notif->timestamp);
    char time_str[20];
    if (show_date) {
        strftime(time_str, sizeof(time_str), "%d/%m/%Y - %H:%M", tm_info);
    } else {
        strftime(time_str, sizeof(time_str), "%H:%M", tm_info);
    }
    GtkWidget *time = gtk_label_new(time_str);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_widget_override_font(time, font_desc);
    #pragma GCC diagnostic pop
    gtk_widget_set_halign(time, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), time, FALSE, FALSE, 0);
    if (is_truncated) {
        char tooltip_text[2 * MAX_LINE_LENGTH + 50];
        strncpy(tooltip_text, full_text, sizeof(tooltip_text) - 1);
        tooltip_text[sizeof(tooltip_text) - 1] = '\0';
        for (char *p = tooltip_text; *p; p++) {
            if (p[0] == '\\' && p[1] == 'n') {
                *p = '\n';
                memmove(p + 1, p + 2, strlen(p + 1));
            }
        }
        gtk_widget_set_tooltip_text(button, tooltip_text);
    }
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_notification_clicked), GINT_TO_POINTER(panel_width));
    g_object_unref(layout);
    return button;
}








static void on_delete_notifications_clicked(GtkWidget *widget, gpointer user_data) {
    dbus_notif_store.count = 0;
    new_notifs = 0;
    memset(dbus_notif_store.notifications, 0, sizeof(dbus_notif_store.notifications));
    if (delete_notifications_button && anim_data && anim_data->is_opening && !anim_data->is_animating) {
        gtk_widget_hide(delete_notifications_button);
    }
    if (anim_data && anim_data->window) {
        regenerate_notifications(anim_data->window);
    }
    update_systray_icon();
}






static void add_notification_to_panel(GtkWidget *box, int width, int height) {
    GtkWidget *notif_box = NULL;
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));
    for (GList *iter = children; iter; iter = iter->next) {
        GtkWidget *child = GTK_WIDGET(iter->data);
        const gchar *name = gtk_widget_get_name(child);
        if (name && !strcmp(name, "notification-box")) {
            notif_box = child;
            break;
        }
    }
    g_list_free(children);
    if (!notif_box) {
        notif_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, NOTIF_BOX_SPACING);
        gtk_widget_set_name(notif_box, "notification-box");
        gtk_box_pack_start(GTK_BOX(box), notif_box, FALSE, FALSE, 0);
    }
    GList *notif_children = gtk_container_get_children(GTK_CONTAINER(notif_box));
    for (GList *iter = notif_children; iter; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(notif_children);
    int n = dbus_notif_store.count;
    int m = (n < max_notification_buttons) ? n : max_notification_buttons;
    if (n > 0) {
        for (int i = 0; i < m; i++) {
            Notification notif;
            DBusNotification *src = &dbus_notif_store.notifications[i];
            notif.timestamp = src->timestamp;
            notif.urgency = src->urgency;
            strncpy(notif.title, src->summary, MAX_LINE_LENGTH - 1);
            notif.title[MAX_LINE_LENGTH - 1] = '\0';
            strncpy(notif.content, src->body, MAX_LINE_LENGTH - 1);
            notif.content[MAX_LINE_LENGTH - 1] = '\0';
            strncpy(notif.icon, src->icon, MAX_ICON_LENGTH - 1);
            notif.icon[MAX_ICON_LENGTH - 1] = '\0';
            notif.filename[0] = '\0';
            GtkWidget *b = create_notification_button(&notif, width, height, FALSE);
            gtk_box_pack_start(GTK_BOX(notif_box), b, FALSE, FALSE, 1);
        }
        if (n > m) {
            char t[NOTIF_LABEL_WIDTH];
            snprintf(t, sizeof(t), "(+%d more...)", n - m);
            GtkWidget *l = gtk_label_new(t);
            gtk_widget_set_halign(l, GTK_ALIGN_CENTER);
            gtk_box_pack_start(GTK_BOX(notif_box), l, FALSE, FALSE, NOTIF_BOX_SPACING);
        }
    } else {
        GtkWidget *c = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_size_request(c, -1, notif_available_height);
        GtkWidget *l = gtk_label_new("No new notifications");
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        gtk_widget_override_font(l, panel_font_desc);
        #pragma GCC diagnostic pop
        gtk_widget_set_halign(l, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(l, GTK_ALIGN_CENTER);
        gtk_style_context_add_class(gtk_widget_get_style_context(l), "project-label");
        gtk_box_pack_start(GTK_BOX(c), l, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(notif_box), c, FALSE, FALSE, NOTIF_BOX_SPACING);
    }
    if (GTK_IS_WIDGET(delete_notifications_button)) {
        if (n >= 2)
            gtk_widget_show(delete_notifications_button);
        else
            gtk_widget_hide(delete_notifications_button);
    }
    gtk_widget_show_all(notif_box);
}





void update_toggle_button_states() {
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        ButtonConfig *b = &button_configs[i];
        if (!b->name[0] || strcmp(b->type, "toggle")) continue;
        if (!b->button) continue;
        int state = 0;
        if (b->is_preprogrammed) {
            char *n = b->name;
            if (*n == 'W' && !strcmp(n, "Wifi")) state = get_wifi_status();
            else if (*n == 'A' && !strcmp(n, "Airplane mode")) state = !(get_wifi_status() || get_bluetooth_status());
            else if (*n == 'N' && !strcmp(n, "Night light")) state = is_night_light_on();
            else if (*n == 'B' && !strcmp(n, "Bluetooth")) state = get_bluetooth_status();
            else if (*n == 'F' && !strcmp(n, "Focus Assist")) state = (sidebar_flags & FLAG_FOCUS_ASSIST) != 0;
        } else {
            state = execute_command_get_result(b->initstate_cmd);
        }
        GtkStyleContext *ctx = gtk_widget_get_style_context(b->button);
        if (state) {
            b->is_active = TRUE;
            gtk_style_context_add_class(ctx, "active-toggle");
        } else {
            b->is_active = FALSE;
            gtk_style_context_remove_class(ctx, "active-toggle");
        }
        update_button_icon(b->button, b->is_active);
        gtk_widget_queue_draw(b->button);
    }
}





static GtkWidget* create_quick_settings_panel(int width, int height) {
    GtkWidget *quick_settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, QUICKSETTINGS_BUTTON_MARGIN);
    int usable_width = width - 2 * QUICKSETTINGS_MARGIN;
    int button_width = (usable_width - 3 * QUICKSETTINGS_BUTTON_MARGIN) / QUICKSETTINGS_BUTTONS_PER_ROW;
    int button_height = button_width * 5 / 6;
    int num_rows = (num_buttons + QUICKSETTINGS_BUTTONS_PER_ROW - 1) / QUICKSETTINGS_BUTTONS_PER_ROW;
    delete_notifications_button = gtk_button_new_with_label("Delete notifications");
    GtkStyleContext *ctx = gtk_widget_get_style_context(delete_notifications_button);
    gtk_style_context_add_class(ctx, "delnotifs-button");
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_widget_override_font(delete_notifications_button, panel_font_desc);
    #pragma GCC diagnostic pop
    gtk_widget_set_halign(delete_notifications_button, GTK_ALIGN_END);
    gtk_widget_set_margin_end(delete_notifications_button, QUICKSETTINGS_MARGIN);
    gtk_widget_set_margin_top(delete_notifications_button, 5);
    gtk_widget_set_margin_bottom(delete_notifications_button, 13);
    g_signal_connect(delete_notifications_button, "clicked", G_CALLBACK(on_delete_notifications_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(quick_settings), delete_notifications_button, FALSE, FALSE, 0);
    gtk_widget_hide(delete_notifications_button);
    for (int row = 0, btn_idx = 0; row < num_rows; row++) {
        GtkWidget *button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, QUICKSETTINGS_BUTTON_MARGIN);
        gtk_widget_set_margin_start(button_row, QUICKSETTINGS_MARGIN);
        gtk_widget_set_margin_end(button_row, QUICKSETTINGS_MARGIN);
        gtk_widget_set_margin_bottom(button_row, QUICKSETTINGS_BUTTON_MARGIN);
        int buttons_in_row = (row == num_rows - 1 && num_buttons % QUICKSETTINGS_BUTTONS_PER_ROW != 0) ?
            (num_buttons % QUICKSETTINGS_BUTTONS_PER_ROW ? num_buttons % QUICKSETTINGS_BUTTONS_PER_ROW : QUICKSETTINGS_BUTTONS_PER_ROW)
            : QUICKSETTINGS_BUTTONS_PER_ROW;
        for (int col = 0; col < buttons_in_row; col++, btn_idx++) {
            int found_idx = -1, count = 0;
            for (int i = 0; i < MAX_BUTTONS && count <= btn_idx; i++) {
                if (button_configs[i].name[0]) {
                    if (count == btn_idx) { found_idx = i; break; }
                    count++;
                }
            }
            if (found_idx >= 0) {
                ButtonConfig *config = &button_configs[found_idx];
                GtkWidget *button = create_button_with_icon_and_label(config, button_width, button_height);
                if (!strcmp(config->type, "toggle") && config->initstate_cmd[0]) {
                    int state = execute_command_get_result(config->initstate_cmd);
                    if (state) {
                        config->is_active = TRUE;
                        gtk_style_context_add_class(gtk_widget_get_style_context(button), "active-toggle");
                    }
                }
                g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), config);
                gtk_box_pack_start(GTK_BOX(button_row), button, FALSE, FALSE, 0);
            }
        }
        for (int i = buttons_in_row; i < QUICKSETTINGS_BUTTONS_PER_ROW; i++) {
            GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_size_request(spacer, button_width, button_height);
            gtk_box_pack_start(GTK_BOX(button_row), spacer, FALSE, FALSE, 0);
        }
        gtk_box_pack_start(GTK_BOX(quick_settings), button_row, FALSE, FALSE, 0);
    }
    GtkWidget *bottom_spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(bottom_spacer, -1, 20);
    gtk_box_pack_start(GTK_BOX(quick_settings), bottom_spacer, FALSE, FALSE, 0);
    if (sidebar_flags & FLAG_BACKLIGHT_CONTROL) {
        GtkWidget *slider_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_widget_set_margin_start(slider_container, QUICKSETTINGS_MARGIN);
        gtk_widget_set_margin_end(slider_container, QUICKSETTINGS_MARGIN);
        gtk_widget_set_margin_top(slider_container, 10);
        gtk_widget_set_margin_bottom(slider_container, 10);
        int total_slider_width = usable_width * 8 / 10;
        int icon_size = width * 6 / 100;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size("/usr/share/qsidebar/icons/backlight.png", icon_size, icon_size, NULL);
        if ((sidebar_flags & FLAG_DARKMODE) && pixbuf) {
            GdkPixbuf *inverted = invert_pixbuf_colors(pixbuf);
            if (inverted) { g_object_unref(pixbuf); pixbuf = inverted; }
        }
        GtkWidget *icon = gtk_image_new_from_pixbuf(pixbuf);
        gtk_box_pack_start(GTK_BOX(slider_container), icon, FALSE, FALSE, 0);
        if (pixbuf) g_object_unref(pixbuf);
        backlight_slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
        gtk_style_context_add_class(gtk_widget_get_style_context(backlight_slider), "backlight-slider");
        gtk_scale_set_draw_value(GTK_SCALE(backlight_slider), FALSE);
        gtk_widget_set_size_request(backlight_slider, total_slider_width - icon_size - 5, 30);
        gtk_box_pack_start(GTK_BOX(slider_container), backlight_slider, TRUE, TRUE, 0);
        if (backlight_info.directory) {
            double percentage = (double)(backlight_info.current_brightness * 100) / backlight_info.max_brightness;
            gtk_range_set_value(GTK_RANGE(backlight_slider), percentage);
        }
        g_signal_connect(backlight_slider, "value-changed", G_CALLBACK(slider_changed), NULL);
        gtk_widget_set_size_request(slider_container, total_slider_width, -1);
        gtk_widget_set_halign(slider_container, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(quick_settings), slider_container, FALSE, FALSE, 0);
        GtkWidget *slider_spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_size_request(slider_spacer, -1, 20);
        gtk_box_pack_start(GTK_BOX(quick_settings), slider_spacer, FALSE, FALSE, 0);
    }
    gtk_widget_show_all(quick_settings);
    return quick_settings;
}




void* play_sound_thread_func(void* arg) {
    ca_context* c = ca_gtk_context_get();
    if (notification_sound != NULL && strcmp(notification_sound, "system") == 0) {
        ca_context_play(c, 0,
                        CA_PROP_EVENT_ID, "message-new-instant", 
                        NULL);
    } else {
        ca_context_play(c, 0,
                        CA_PROP_MEDIA_FILENAME, notification_sound,
                        NULL);
    }
    return NULL;
}





void play_sound_in_thread() {
    pthread_t thread;
    pthread_create(&thread, NULL, play_sound_thread_func, NULL);
    pthread_join(thread, NULL);
}




static gboolean show_notification_popup_wrapper(gpointer user_data) {
    Notification *notif = (Notification *)user_data;
    show_notification_popup(notif);
    g_free(notif);
    return G_SOURCE_REMOVE;
}



void handle_signal(int signum) {
    if (anim_data == NULL) return;
    if (signum == SIGHUP) {
if (status_icon && !(sidebar_flags & FLAG_TRINITY_APPLET)) {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            gtk_status_icon_set_visible(status_icon, FALSE);
            #pragma GCC diagnostic pop
            g_object_unref(status_icon);
            status_icon = NULL;
        }
        if (gtk_main_level() > 0) {
            restart_requested = 1;
            gtk_main_quit();
        } else {
            restart_requested = 1;
        }
    } else if (signum == SIGUSR1) {
        if (!anim_data->is_animating) {
            if (!anim_data->is_opening) {
                new_notifs = 0;
                update_systray_icon();
                update_toggle_button_states();
                if (sidebar_flags & FLAG_BACKLIGHT_CONTROL) {
                    update_current_brightness();
                }
                if (render_options.anim_type == ANIM_TYPE_NONE) {
                    anim_data->current_x = anim_data->start_x - anim_data->width;
                    anim_data->current_opacity = render_options.opacity;
                    gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->current_x, anim_data->y_position);
                    gtk_widget_set_opacity(anim_data->window, anim_data->current_opacity);
                    gtk_widget_show_all(anim_data->window);
                    if (delete_notifications_button) {
                        if (dbus_notif_store.count >= 2) {
                            gtk_widget_show(delete_notifications_button);
                        } else {
                            gtk_widget_hide(delete_notifications_button);
                        }
                    }
                    if (anim_data->click_window) {
                        gtk_widget_show(anim_data->click_window);
                        if ((sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) && transparent_click_type != 0) {
                            gtk_window_set_keep_above(GTK_WINDOW(anim_data->click_window), TRUE);
                        }
                    }
                    anim_data->is_opening = TRUE;
                    anim_data->is_project_panel = FALSE;
                } else {
                    anim_data->target_x = anim_data->start_x - anim_data->width;
                    if (render_options.anim_type == ANIM_TYPE_SLIDE) {
                        anim_data->current_x = anim_data->start_x;
                        anim_data->current_opacity = render_options.opacity;
                    } else if (render_options.anim_type == ANIM_TYPE_FADE) {
                        anim_data->current_x = anim_data->target_x;
                        anim_data->current_opacity = 0.0;
                        anim_data->target_opacity = render_options.opacity;
                    } else if (render_options.anim_type == ANIM_TYPE_SLFD) {
                        anim_data->current_x = anim_data->start_x;
                        anim_data->current_opacity = 0.0;
                        anim_data->target_opacity = render_options.opacity;
                    }
                    anim_data->is_animating = TRUE;
                    anim_data->is_opening = TRUE;
                    gtk_window_move(GTK_WINDOW(anim_data->window), anim_data->current_x, anim_data->y_position);
                    gtk_widget_set_opacity(anim_data->window, anim_data->current_opacity);
                    gtk_widget_show_all(anim_data->window);
                    if (delete_notifications_button) {
                        if (dbus_notif_store.count >= 2) {
                            gtk_widget_show(delete_notifications_button);
                        } else {
                            gtk_widget_hide(delete_notifications_button);
                        }
                    }
                    if (anim_data->click_window) {
                        gtk_widget_show(anim_data->click_window);
                        if ((sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) && transparent_click_type != 0) {
                            gtk_window_set_keep_above(GTK_WINDOW(anim_data->click_window), TRUE);
                        }
                    }
                    g_timeout_add(8, animate_window, anim_data);
                }
                anim_data->is_project_panel = FALSE;
                if (anim_data->update_pending) {
                    regenerate_notifications(anim_data->window);
                    anim_data->update_pending = FALSE;
                }
            } else {
                if (anim_data->is_project_panel) {
                    update_systray_icon();
                } else {
                    new_notifs = 0;
                    update_systray_icon();
                }
                if (render_options.anim_type == ANIM_TYPE_NONE) {
                    anim_data->current_x = anim_data->start_x;
                    anim_data->current_opacity = 0.0;
                    gtk_widget_hide(anim_data->window);
                    if (anim_data->click_window) {
                        gtk_widget_hide(anim_data->click_window);
                    }
                    anim_data->is_opening = FALSE;
                    if (anim_data->is_project_panel) {
                        recreate_original_buttons(NULL);
                        anim_data->is_project_panel = FALSE;
                    }
                } else {
                    if (render_options.anim_type == ANIM_TYPE_SLIDE) {
                        anim_data->target_x = anim_data->start_x;
                    } else if (render_options.anim_type == ANIM_TYPE_FADE) {
                        anim_data->target_opacity = 0.0;
                    } else if (render_options.anim_type == ANIM_TYPE_SLFD) {
                        anim_data->target_x = anim_data->start_x;
                        anim_data->target_opacity = 0.0;
                    }
                    anim_data->is_animating = TRUE;
                    anim_data->is_opening = FALSE;
                    if (anim_data->is_project_panel) {
                        recreate_original_buttons(NULL);
                        anim_data->is_project_panel = FALSE;
                    }
                    g_timeout_add(8, animate_window, anim_data);
                }
            }
            if (notification_popup != NULL) {
                gtk_widget_destroy(notification_popup);
                notification_popup = NULL;
                if (timeout_id != 0) {
                    g_source_remove(timeout_id);
                    timeout_id = 0;
                }
                current_popup_urgency = 255;
            }
        }
    } else if (signum == SIGUSR2) {
        if (anim_data->is_opening && !anim_data->is_animating) {
            if (render_options.anim_type == ANIM_TYPE_NONE) {
                anim_data->current_x = anim_data->start_x;
                anim_data->current_opacity = 0.0;
                gtk_widget_hide(anim_data->window);
                if (anim_data->click_window) {
                    gtk_widget_hide(anim_data->click_window);
                }
                anim_data->is_opening = FALSE;
                if (anim_data->is_project_panel) {
                    recreate_original_buttons(NULL);
                    anim_data->is_project_panel = FALSE;
                }
            } else {
                if (render_options.anim_type == ANIM_TYPE_SLIDE) {
                    anim_data->target_x = anim_data->start_x;
                } else if (render_options.anim_type == ANIM_TYPE_FADE) {
                    anim_data->target_opacity = 0.0;
                } else if (render_options.anim_type == ANIM_TYPE_SLFD) {
                    anim_data->target_x = anim_data->start_x;
                    anim_data->target_opacity = 0.0;
                }
                anim_data->is_animating = TRUE;
                anim_data->is_opening = FALSE;
                if (anim_data->is_project_panel) {
                    recreate_original_buttons(NULL);
                    anim_data->is_project_panel = FALSE;
                }
                g_timeout_add(8, animate_window, anim_data);
            }
        }
    } else if (signum == SIG_DBUS_NOTIF) {
        if (dbus_notif_store.count == 0) return;
        if (!anim_data->is_opening || anim_data->is_project_panel) {
            new_notifs++;
        }
        DBusNotification *latest_dbus_notif = &dbus_notif_store.notifications[dbus_notif_store.count - 1];
        if (!(sidebar_flags & FLAG_FOCUS_ASSIST) ||
              latest_dbus_notif->urgency == 2) {
            if (notification_sound != NULL &&
            strcmp(notification_sound, "silent") != 0 &&
            !(sidebar_flags & FLAG_SILENT_THIS)) {
            play_sound_in_thread();
       }
        }
        if (anim_data->is_opening && !anim_data->is_animating && !anim_data->is_project_panel) {
            if (regenerate_timeout_id == 0) {
                regenerate_timeout_id = g_timeout_add(100, delayed_regenerate_notifications, anim_data->window);
            }
            if (delete_notifications_button) {
                if (dbus_notif_store.count >= 2) {
                    gtk_widget_show(delete_notifications_button);
                } else {
                    gtk_widget_hide(delete_notifications_button);
                }
            }
        } else {
            anim_data->update_pending = TRUE;
            if (!anim_data->is_opening && !anim_data->is_animating) {
                Notification *latest_notif = g_malloc0(sizeof(Notification));
                latest_notif->timestamp = latest_dbus_notif->timestamp;
                latest_notif->urgency = latest_dbus_notif->urgency;
                strncpy(latest_notif->title, latest_dbus_notif->summary, MAX_LINE_LENGTH - 1);
                latest_notif->title[MAX_LINE_LENGTH - 1] = '\0';
                strncpy(latest_notif->content, latest_dbus_notif->body, MAX_LINE_LENGTH - 1);
                latest_notif->content[MAX_LINE_LENGTH - 1] = '\0';
                strncpy(latest_notif->icon, latest_dbus_notif->icon, MAX_ICON_LENGTH - 1);
                latest_notif->icon[MAX_ICON_LENGTH - 1] = '\0';
                latest_notif->filename[0] = '\0';
                g_idle_add(show_notification_popup_wrapper, latest_notif);
            }
        }
        update_systray_icon();
    } else if (signum == SIGWINCH) {
        update_systray_icon();
    } else if (signum == SIGALRM) {
      sidebar_flags ^= FLAG_FOCUS_ASSIST;
      update_systray_icon();
        } else if (signum == SIGVTALRM) {
     sidebar_flags ^= FLAG_NOTIF_NUMBER_INDICATOR;
     update_systray_icon();
     } else if (signum == SIGXFSZ) {
     sidebar_flags ^= FLAG_NOTIF_HIDE_ICON;
    }
}



static void on_status_icon_activate(GtkStatusIcon *icon, gpointer user_data) {
    handle_signal(SIGUSR1);
}


static void handle_dbus_notification(void);
static void handle_dbus_notification(void) {
    handle_signal(SIG_DBUS_NOTIF);
}



int custom_ceil(float value) {
    int int_value = (int)value;
    if (value > int_value) {
        return int_value + 1;
    }
    return int_value;
}




static void regenerate_notifications(GtkWidget *window) {
    GtkWidget *box = gtk_bin_get_child(GTK_BIN(window));
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));
    GtkWidget *notif_box = NULL;
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        GtkWidget *child = GTK_WIDGET(iter->data);
        const gchar *name = gtk_widget_get_name(child);
        if (name && strcmp(name, "notification-box") == 0) {
            notif_box = child;
            break;
        }
    }
    g_list_free(children);
    if (!notif_box) return;
    GList *notif_children = gtk_container_get_children(GTK_CONTAINER(notif_box));
    for (GList *iter = notif_children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(notif_children);
    int notif_count = 0;
    Notification *notifications = NULL;
    if (dbus_notif_store.count > 0) {
        notifications = malloc(dbus_notif_store.count * sizeof(Notification));
        for (int i = 0; i < dbus_notif_store.count; i++) {
            DBusNotification *dbus_notif = &dbus_notif_store.notifications[i];
            Notification *notif = &notifications[notif_count++];
            notif->timestamp = dbus_notif->timestamp;
            notif->urgency = dbus_notif->urgency;
            strncpy(notif->title, dbus_notif->summary, MAX_LINE_LENGTH - 1);
            notif->title[MAX_LINE_LENGTH - 1] = '\0';
            strncpy(notif->content, dbus_notif->body, MAX_LINE_LENGTH - 1);
            notif->content[MAX_LINE_LENGTH - 1] = '\0';
            strncpy(notif->icon, dbus_notif->icon, MAX_ICON_LENGTH - 1);
            notif->icon[MAX_ICON_LENGTH - 1] = '\0';
            notif->filename[0] = '\0';
        }
    }
    if (notif_count > 0) {
        qsort(notifications, notif_count, sizeof(Notification), compare_notifications);
        int display_count = (notif_count < max_notification_buttons) ? notif_count : max_notification_buttons;
        int hidden_count = notif_count - display_count;
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);
        int today_year = tm_now.tm_year;
        int today_mon = tm_now.tm_mon;
        int today_mday = tm_now.tm_mday;
        for (int i = 0; i < display_count; i++) {
            struct tm tm_notif;
            localtime_r(&notifications[i].timestamp, &tm_notif);
            gboolean show_date = (tm_notif.tm_year != today_year ||
                                  tm_notif.tm_mon != today_mon ||
                                  tm_notif.tm_mday != today_mday);
            GtkWidget *notif_button = create_notification_button(&notifications[i], anim_data->width, anim_data->height, show_date);
            gtk_box_pack_start(GTK_BOX(notif_box), notif_button, FALSE, FALSE, 1);
        }
        if (hidden_count > 0) {
            char label_text[32];
            snprintf(label_text, sizeof(label_text), "(+%d more...)", hidden_count);
            GtkWidget *more_label = gtk_label_new(label_text);
            gtk_widget_set_halign(more_label, GTK_ALIGN_CENTER);
            gtk_box_pack_start(GTK_BOX(notif_box), more_label, FALSE, FALSE, 5);
        }
    } else {
        GtkWidget *center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_size_request(center_box, -1, notif_available_height);
        GtkWidget *no_notif_label = gtk_label_new("No new notifications");
        gtk_widget_set_halign(no_notif_label, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(no_notif_label, GTK_ALIGN_CENTER);
        GtkStyleContext *context = gtk_widget_get_style_context(no_notif_label);
        gtk_style_context_add_class(context, "project-label");
        gtk_box_pack_start(GTK_BOX(center_box), no_notif_label, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(notif_box), center_box, FALSE, FALSE, 5);
    }
    if (notifications) free(notifications);
    gtk_widget_show_all(notif_box);
}









static gboolean on_notification_popup_timeout(gpointer user_data) {
    GtkWidget *popup = (GtkWidget *)user_data;
    if (GTK_IS_WIDGET(popup)) {
        gtk_widget_destroy(popup);
    }
    if (popup == notification_popup) {
        notification_popup = NULL;
        timeout_id = 0;
    }
    return G_SOURCE_REMOVE;
}




static void show_notification_popup(const Notification *notif) {
   if (((sidebar_flags & FLAG_FOCUS_ASSIST) && notif->urgency != 2) ||
       (notification_popup && notif->urgency < current_popup_urgency)) return;
    if (notification_popup) {
        gtk_widget_destroy(notification_popup);
        notification_popup = NULL;
        if (timeout_id) { g_source_remove(timeout_id); timeout_id = 0; }
    }
    notification_popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_widget_set_opacity(notification_popup, popup_opacity);
    gtk_window_set_keep_above(GTK_WINDOW(notification_popup), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(notification_popup), FALSE);
    gtk_window_set_type_hint(GTK_WINDOW(notification_popup), notif_type_hint);
    if (transparent_click_type && transparent_click_type != 4) {
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(notification_popup), TRUE);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(notification_popup), TRUE);
    }
    gtk_widget_set_app_paintable(notification_popup, TRUE);
    gtk_widget_set_name(notification_popup, "transparent-window");
    GdkScreen *screen = gtk_widget_get_screen(notification_popup);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (visual && gdk_screen_is_composited(screen))
        gtk_widget_set_visual(notification_popup, visual);
    int w = anim_data->width, h = anim_data->height;
    GtkWidget *notif_btn = create_notification_button(notif, w, h, FALSE);
    gtk_container_add(GTK_CONTAINER(notification_popup), notif_btn);
    GdkRectangle workarea = {0};
    gdk_monitor_get_workarea(gdk_display_get_monitor(gdk_display_get_default(), 0), &workarea);
    int bw = w, bh = h / 8;
    gtk_window_set_default_size(GTK_WINDOW(notification_popup), bw, bh);
    int x, y;
    int margin = 10;
    switch (notif_pos) {
        case 0:
            x = workarea.x + workarea.width - bw - margin;
            y = workarea.y + workarea.height - bh - margin;
            break;
        case 1:
            x = workarea.x + workarea.width - bw - margin;
            y = workarea.y + margin;
            break;
        case 2:
            x = workarea.x + margin;
            y = workarea.y + margin;
            break;
        case 3:
            x = workarea.x + margin;
            y = workarea.y + workarea.height - bh - margin;
            break;
        default:
            x = workarea.x + workarea.width - bw - margin;
            y = workarea.y + workarea.height - bh - margin;
            break;
    }
    gtk_window_move(GTK_WINDOW(notification_popup), x, y);
    g_signal_connect(notification_popup, "draw", G_CALLBACK(on_draw), NULL);
    gtk_widget_show_all(notification_popup);
    current_popup_urgency = notif->urgency;
    timeout_id = (notif->urgency == 0) ? g_timeout_add_seconds(notif_low_timeout, on_notification_popup_timeout, notification_popup)
             : (notif->urgency == 1) ? g_timeout_add_seconds(notif_normal_timeout, on_notification_popup_timeout, notification_popup)
             : 0;
}








static gboolean on_click_outside(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    if (anim_data != NULL && anim_data->is_opening && !anim_data->is_animating) {
        if (render_options.anim_type == ANIM_TYPE_SLIDE) {
            anim_data->target_x = anim_data->start_x;
        } else if (render_options.anim_type == ANIM_TYPE_FADE) {
            anim_data->target_opacity = 0.0;
        }
        anim_data->is_animating = TRUE;
        anim_data->is_opening = FALSE;
        if (anim_data->is_project_panel) {
            recreate_original_buttons(NULL);
            anim_data->is_project_panel = FALSE;
        }
        g_timeout_add(8, animate_window, anim_data);
        if (anim_data->click_window) {
            gtk_widget_hide(anim_data->click_window);
        }
        return TRUE;
    }
    return FALSE;
}







static void cleanup_sidebar(void) {
    if (anim_data) {
        if (anim_data->window) {
            gtk_widget_destroy(anim_data->window);
            anim_data->window = NULL;
        }
        if (anim_data->click_window) {
            gtk_widget_destroy(anim_data->click_window);
            anim_data->click_window = NULL;
        }
        g_free(anim_data);
        anim_data = NULL;
    }
    if (notification_popup) {
        gtk_widget_destroy(notification_popup);
        notification_popup = NULL;
    }
    if (timeout_id != 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }
    if (notification_sound != NULL &&
        strcmp(notification_sound, "silent") != 0 &&
        strcmp(notification_sound, "/usr/share/qsidebar/sounds/notify_win10.wav") != 0 &&
        strcmp(notification_sound, "/usr/share/qsidebar/sounds/notify_win11.wav") != 0) {
        free((char*)notification_sound);
    }
    if (backlight_info.directory) {
        free(backlight_info.directory);
        backlight_info.directory = NULL;
    }
    notification_sound = NULL;
}




static GdkPixbuf *crop_icon(const char *icon_path) {
    GError *e = NULL;
    GdkPixbuf *p = gdk_pixbuf_new_from_file(icon_path, &e);
    if (!p) { if (e) g_error_free(e); return NULL; }
    if (gdk_pixbuf_get_width(p) != 64 || gdk_pixbuf_get_height(p) != 64) { g_object_unref(p); return NULL; }
    GdkPixbuf *c = gdk_pixbuf_new_subpixbuf(p, 14, 14, 39, 39);
    g_object_unref(p);
    return c ? c : NULL;
}




static GtkWidget* create_tray_menu(void) {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *open_sidebar = gtk_menu_item_new_with_label("    Open sidebar                                                  ");
    g_signal_connect(open_sidebar, "activate", G_CALLBACK(on_menu_open_sidebar), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_sidebar);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    char label_focus[128];
    snprintf(label_focus, sizeof(label_focus), "%s  Focus assist",
         (sidebar_flags & FLAG_FOCUS_ASSIST) ? "✓" : "  ");
    GtkWidget *focus_assist_item = gtk_menu_item_new_with_label(label_focus);
    g_signal_connect(focus_assist_item, "activate", G_CALLBACK(on_menu_focus_assist), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), focus_assist_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    char label_no_icons[128];
    snprintf(label_no_icons, sizeof(label_no_icons), "%s  Don't display applications icons",
          (sidebar_flags & FLAG_NOTIF_HIDE_ICON) ? "✓" : "  ");
    GtkWidget *no_icons_item = gtk_menu_item_new_with_label(label_no_icons);
    g_signal_connect(no_icons_item, "activate", G_CALLBACK(on_menu_no_icons), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), no_icons_item);
    char label_no_notif[128];
   snprintf(label_no_notif, sizeof(label_no_notif), "%s  Don't display notifications count",
         (sidebar_flags & FLAG_NOTIF_NUMBER_INDICATOR) ? "  " : "✓");
    GtkWidget *no_notif_item = gtk_menu_item_new_with_label(label_no_notif);
    g_signal_connect(no_notif_item, "activate", G_CALLBACK(on_menu_no_notif), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), no_notif_item);
    gtk_widget_show_all(menu);
    return menu;
}




static void on_status_icon_popup_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data) {
handle_signal(SIGUSR2); 
    GtkWidget *menu = create_tray_menu();
    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}


static void update_systray_icon(void) {
if (!(sidebar_flags & FLAG_USE_SYSTRAY) && !(sidebar_flags & FLAG_TRINITY_APPLET)) {
        return;
    }
    const char *icon_path;
if (sidebar_flags & FLAG_FOCUS_ASSIST) {
        icon_path = (new_notifs > 0) ? tray_icon_focus_filled : tray_icon_focus_normal;
    } else if (!(sidebar_flags & FLAG_NOTIF_NUMBER_INDICATOR)) {
        icon_path = new_notifs > 0 ? tray_icon_filled : tray_icon_normal;
    } else {
        int notif_count = new_notifs;
        if (notif_count == 0) {
            icon_path = tray_icon_normal;
        } else {
            char icon_path_buffer[256];
            if (notif_count >= 1 && notif_count <= 9) {
                snprintf(icon_path_buffer, sizeof(icon_path_buffer), "%s%d.png", tray_prefix, notif_count);
            } else {
                snprintf(icon_path_buffer, sizeof(icon_path_buffer), "%s9+.png", tray_prefix);
             }
            icon_path = icon_path_buffer;
          }
       }
       if (g_file_test(icon_path, G_FILE_TEST_EXISTS)) {
         if ((sidebar_flags & FLAG_USE_SYSTRAY) && !(sidebar_flags & FLAG_TRINITY_APPLET)) {
            if (!status_icon) {
                return;
            }
            GdkPixbuf *cropped_pixbuf = crop_icon(icon_path);
            if (cropped_pixbuf) {
               if ((sidebar_flags & FLAG_DARKMODE) && !(sidebar_flags & FLAG_TRAYCOLORMODE)) {
                    GdkPixbuf *inverted_pixbuf = invert_pixbuf_colors(cropped_pixbuf);
                    if (inverted_pixbuf) {
                        #pragma GCC diagnostic push
                        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                        gtk_status_icon_set_from_pixbuf(status_icon, inverted_pixbuf);
                        #pragma GCC diagnostic pop
                        g_object_unref(inverted_pixbuf);
                    } else {
                        g_object_unref(cropped_pixbuf);
                        return;
                    }
                } else {
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    gtk_status_icon_set_from_pixbuf(status_icon, cropped_pixbuf);
                    #pragma GCC diagnostic pop
                }
                g_object_unref(cropped_pixbuf);
            }
        }
if (sidebar_flags & FLAG_TRINITY_APPLET) {
    if ((sidebar_flags & FLAG_DARKMODE) && !(sidebar_flags & FLAG_TRAYCOLORMODE)) {
        set_sidebar_icon(
            icon_path,
            TRUE,
            (sidebar_flags & FLAG_FOCUS_ASSIST) != 0,
            (sidebar_flags & FLAG_NOTIF_HIDE_ICON) != 0,
            !(sidebar_flags & FLAG_NOTIF_NUMBER_INDICATOR));
    } else {
          set_sidebar_icon(
            icon_path,
            FALSE,
            (sidebar_flags & FLAG_FOCUS_ASSIST) != 0,
            (sidebar_flags & FLAG_NOTIF_HIDE_ICON) != 0,
            !(sidebar_flags & FLAG_NOTIF_NUMBER_INDICATOR));
    }
}
    } else {
        g_print("System tray icon file not found: %s\n", icon_path);
    }
}



static void on_menu_open_sidebar(GtkMenuItem *item, gpointer user_data) {
    handle_signal(SIGUSR1); 
}

static void on_menu_focus_assist(GtkMenuItem *item, gpointer user_data) {
 sidebar_flags ^= FLAG_FOCUS_ASSIST;
}

static void on_menu_no_icons(GtkMenuItem *item, gpointer user_data) {
    sidebar_flags ^= FLAG_NOTIF_HIDE_ICON;
}

static void on_menu_no_notif(GtkMenuItem *item, gpointer user_data) {
sidebar_flags ^= FLAG_NOTIF_NUMBER_INDICATOR;
update_systray_icon();
}






static void main_sidebar(void) {
    load_config();
    gboolean backlight_initialized = FALSE;
    if (sidebar_flags & FLAG_BACKLIGHT_CONTROL) {
        backlight_initialized = initialize_backlight();
        if (!backlight_initialized) {
            sidebar_flags &= ~FLAG_BACKLIGHT_CONTROL;
        }
    }
    gtk_init(NULL, NULL);
    if ((sidebar_flags & FLAG_USE_SYSTRAY) || (sidebar_flags & FLAG_TRINITY_APPLET)) {
        const char *icon_path = tray_icon_normal;
        if (g_file_test(icon_path, G_FILE_TEST_EXISTS)) {
            if ((sidebar_flags & FLAG_USE_SYSTRAY) && !(sidebar_flags & FLAG_TRINITY_APPLET)) {
                GdkPixbuf *cropped_pixbuf = crop_icon(icon_path);
                if (cropped_pixbuf) {
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    status_icon = gtk_status_icon_new_from_pixbuf(cropped_pixbuf);
                    gtk_status_icon_set_visible(status_icon, TRUE);
                    #pragma GCC diagnostic pop
                    g_object_unref(cropped_pixbuf);
                    g_signal_connect(status_icon, "activate", G_CALLBACK(on_status_icon_activate), NULL);
                    g_signal_connect(status_icon, "popup-menu", G_CALLBACK(on_status_icon_popup_menu), NULL);
                }
            }
            update_systray_icon();
        } else {
            g_print("System tray icon file not found: %s\n", icon_path);
        }
    }
    load_css();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
    GdkRectangle workarea = {0};
    GdkMonitor *monitor = gdk_display_get_monitor(gdk_display_get_default(), 0);
    gdk_monitor_get_workarea(monitor, &workarea);
    int width = workarea.width / 4.5;
    int font_size = pango_font_description_get_size(panel_font_desc) / PANGO_SCALE;
    int delete_button_height = font_size * 1.2 + 5 + 24;
    int slider_height = (sidebar_flags & FLAG_BACKLIGHT_CONTROL) ? 40 : 0;
    int header_height = 30;
    int num_rows = (num_buttons + QUICKSETTINGS_BUTTONS_PER_ROW - 1) / QUICKSETTINGS_BUTTONS_PER_ROW;
    int usable_width = width - 2 * QUICKSETTINGS_MARGIN;
    int button_width = (usable_width - 3 * QUICKSETTINGS_BUTTON_MARGIN) / QUICKSETTINGS_BUTTONS_PER_ROW;
    int button_height = button_width * 5 / 6;
    int button_rows_height = num_rows * button_height + (num_rows > 1 ? (num_rows - 1) * QUICKSETTINGS_BUTTON_MARGIN : 0);
    int main_box_spacing = 20;
    int slider_bottom_margin = (sidebar_flags & FLAG_BACKLIGHT_CONTROL) ? 20 : 0;
    bottom_margin = 0;
    int fixed_height = header_height + delete_button_height + button_rows_height + 20 + slider_height +
                       main_box_spacing + slider_bottom_margin + panel_bottom_margin;
    int notif_height = workarea.height - fixed_height - panel_bottom_margin;
    int window_height = ((sidebar_flags & FLAG_EXTEND_MODE) && (sidebar_flags & FLAG_PROJECT_EXTEND_FULL_HEIGHT))
        ? MIN(get_secondary_height(), workarea.height - panel_bottom_margin)
        : workarea.height - panel_bottom_margin;
    int pas_anim = custom_ceil(workarea.width * 0.013);
    int pas_hide = custom_ceil(workarea.width * 0.018);
    calculate_max_notification_buttons_once(notif_height, num_buttons);
    gtk_window_set_default_size(GTK_WINDOW(window), width, window_height);
    if (panel_background[0] != '\0') {
        background_source = cairo_image_surface_create_from_png(panel_background);
        if (cairo_surface_status(background_source) != CAIRO_STATUS_SUCCESS) {
            g_warning("Error loading background '%s'. Only .png images are supported.", panel_background);
            sidebar_flags &= ~FLAG_IS_BACKGROUND;
            background_source = NULL;
        } else {
            background_width = cairo_image_surface_get_width(background_source);
            background_height = cairo_image_surface_get_height(background_source);
            sidebar_flags |= FLAG_IS_BACKGROUND;
        }
    } else {
        sidebar_flags &= ~FLAG_IS_BACKGROUND;
        background_source = NULL;
    }
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_set_name(window, "transparent-window");
    GdkScreen *screen_rgba = gtk_widget_get_screen(window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen_rgba);
    if (visual != NULL && gdk_screen_is_composited(screen_rgba)) {
        gtk_widget_set_visual(window, visual);
    }
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), header_box, FALSE, FALSE, 10);
    GtkWidget *label = gtk_label_new(panel_title);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_widget_override_font(label, paneltitlefont_desc);
    #pragma GCC diagnostic pop
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "actions-center-label");
    gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 0);
    add_notification_to_panel(box, width, notif_height);
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(box), spacer, TRUE, TRUE, 0);
    GtkWidget *quick_settings = create_quick_settings_panel(width, notif_height);
    gtk_box_pack_end(GTK_BOX(box), quick_settings, FALSE, FALSE, bottom_margin);
    GtkWidget *click_window = NULL;
    switch (transparent_click_type) {
        case 0: notif_type_hint = GDK_WINDOW_TYPE_HINT_DOCK; break;
        case 1: notif_type_hint = GDK_WINDOW_TYPE_HINT_COMBO; break;
        case 2: notif_type_hint = GDK_WINDOW_TYPE_HINT_UTILITY; break;
        case 3: notif_type_hint = GDK_WINDOW_TYPE_HINT_DIALOG; break;
        case 4: notif_type_hint = GDK_WINDOW_TYPE_HINT_MENU; break;
        default: notif_type_hint = GDK_WINDOW_TYPE_HINT_COMBO; break;
    }
    if (render_options.use_transparent_click) {
        click_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_decorated(GTK_WINDOW(click_window), FALSE);
        GdkWindowTypeHint type_hint = (sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) ? notif_type_hint : GDK_WINDOW_TYPE_HINT_DESKTOP;
        gtk_window_set_type_hint(GTK_WINDOW(click_window), type_hint);
        if ((sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) && transparent_click_type != 0 && transparent_click_type != 4) {
            gtk_window_set_skip_taskbar_hint(GTK_WINDOW(click_window), TRUE);
            gtk_window_set_skip_pager_hint(GTK_WINDOW(click_window), TRUE);
        }
        gtk_widget_set_app_paintable(click_window, TRUE);
        gtk_widget_set_name(click_window, "transparent-window");
        GdkScreen *screen = gtk_widget_get_screen(click_window);
        visual = gdk_screen_get_rgba_visual(screen);
        if (visual && gdk_screen_is_composited(screen)) {
            gtk_widget_set_visual(click_window, visual);
        }
        gtk_widget_set_opacity(click_window, 0.0);
        int click_window_width = ((sidebar_flags & FLAG_EXTEND_MODE) ? total_display_width : workarea.width) - width;
        gtk_window_set_default_size(GTK_WINDOW(click_window), click_window_width, workarea.height);
        gtk_window_move(GTK_WINDOW(click_window), workarea.x, workarea.y);
        GtkWidget *click_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(click_window), click_box);
        g_signal_connect(click_window, "button-press-event", G_CALLBACK(on_click_outside), NULL);
    }
    int start_x = (sidebar_flags & FLAG_EXTEND_MODE) ? total_display_width : workarea.width;
    int target_x = workarea.width - width;
    gtk_window_move(GTK_WINDOW(window), start_x, workarea.y);
    anim_data = g_new(AnimationData, 1);
    anim_data->window = window;
    anim_data->click_window = click_window;
    anim_data->target_x = target_x;
    anim_data->current_x = start_x;
    anim_data->width = width;
    anim_data->height = window_height;
    anim_data->start_x = start_x;
    anim_data->y_position = workarea.y;
    anim_data->is_animating = FALSE;
    anim_data->is_opening = FALSE;
    anim_data->is_project_panel = FALSE;
    anim_data->pas_anim = pas_anim;
    anim_data->pas_hide = pas_hide;
    anim_data->current_opacity = 1.0;
    anim_data->target_opacity = 1.0;
    anim_data->update_pending = FALSE;
    gtk_widget_set_opacity(window, 1.0);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    g_signal_connect(window, "draw", G_CALLBACK(on_draw), NULL);
    gtk_widget_show_all(window);
    gtk_main();
    cairo_surface_destroy(background_source);
    cleanup_sidebar();
}






gboolean get_wifi_status() {
    GError *e = NULL;
    GDBusConnection *c = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &e);
    if (!c) return FALSE;
    GVariant *r = g_dbus_connection_call_sync(
        c, NM_DBUS_SERVICE, NM_DBUS_PATH, DBUS_PROPERTIES_INTERFACE, "Get",
        g_variant_new("(ss)", NM_DBUS_INTERFACE, "WirelessEnabled"),
        G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &e
    );
    gboolean s = FALSE;
    if (r) {
        GVariant *v = g_variant_get_child_value(r, 0);
        GVariant *u = g_variant_get_variant(v);
        s = g_variant_get_boolean(u);
        g_variant_unref(u);
        g_variant_unref(v);
        g_variant_unref(r);
    }
    g_object_unref(c);
    return s;
}






gboolean get_bluetooth_status() {
    GError *e = NULL;
    GDBusConnection *c = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &e);
    if (!c) return FALSE;
    GVariant *r = g_dbus_connection_call_sync(
        c, BT_DBUS_SERVICE, "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects",
        NULL, G_VARIANT_TYPE("(a{oa{sa{sv}}})"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &e
    );
    if (!r) { g_object_unref(c); return FALSE; }
    GVariantIter *it; const gchar *op; gboolean s = FALSE;
    GVariant *od = g_variant_get_child_value(r, 0);
    g_variant_get(od, "a{oa{sa{sv}}}", &it);
    while (g_variant_iter_loop(it, "{&oa{sa{sv}}}", &op, NULL)) {
        if (!g_str_has_prefix(op, "/org/bluez/hci")) continue;
        GVariant *pr = g_dbus_connection_call_sync(
            c, BT_DBUS_SERVICE, op, BT_DBUS_PROPERTIES_IFACE, "Get",
            g_variant_new("(ss)", BT_DBUS_ADAPTER_IFACE, "Powered"),
            G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &e
        );
        if (pr) {
            GVariant *v; g_variant_get(pr, "(v)", &v);
            s = g_variant_get_boolean(v);
            g_variant_unref(v); g_variant_unref(pr); break;
        }
    }
    g_variant_iter_free(it); g_variant_unref(od); g_variant_unref(r); g_object_unref(c);
    return s;
}




// gboolean get_ethernet_status() {
//     NMClient *client;
//     const GPtrArray *devices;
//     GError *error = NULL;
//     gboolean status = FALSE;
//     client = nm_client_new(NULL, &error);
//     if (!client) {
//         return FALSE;
//     }
//     devices = nm_client_get_devices(client);
//     for (guint i = 0; devices && i < devices->len; i++) {
//         NMDevice *device = g_ptr_array_index(devices, i);
//         if (NM_IS_DEVICE_ETHERNET(device)) {
//             NMDeviceState state = nm_device_get_state(device);
//             if (state >= NM_DEVICE_STATE_PREPARE && state <= NM_DEVICE_STATE_ACTIVATED) {
//                 status = TRUE;
//                 break;
//             }
//         }
//     }
//     g_object_unref(client);
//     return status;
// }




gboolean set_wifi_status(gboolean enable) {
    GError *e = NULL;
    GDBusConnection *c = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &e);
    if (!c) return FALSE;
    GVariant *r = g_dbus_connection_call_sync(
        c, NM_DBUS_SERVICE, NM_DBUS_PATH, DBUS_PROPERTIES_INTERFACE, "Set",
        g_variant_new("(ssv)", NM_DBUS_INTERFACE, "WirelessEnabled", g_variant_new_boolean(enable)),
        NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &e
    );
    gboolean s = (r != NULL);
    if (r) g_variant_unref(r);
    g_object_unref(c);
    return s;
}




gboolean set_bluetooth_status(gboolean enable) {
    GError *e = NULL;
    GDBusConnection *c = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &e);
    if (!c) return FALSE;
    GVariant *r = g_dbus_connection_call_sync(
        c, BT_DBUS_SERVICE, "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects",
        NULL, G_VARIANT_TYPE("(a{oa{sa{sv}}})"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &e
    );
    if (!r) { g_object_unref(c); return FALSE; }
    GVariantIter *it; const gchar *op; gboolean ok = TRUE, found = FALSE;
    GVariant *od = g_variant_get_child_value(r, 0);
    g_variant_get(od, "a{oa{sa{sv}}}", &it);
    while (g_variant_iter_loop(it, "{&oa{sa{sv}}}", &op, NULL)) {
        if (!g_str_has_prefix(op, "/org/bluez/hci") || strstr(op, "/dev_")) continue;
        found = TRUE;
        GVariant *pr = g_dbus_connection_call_sync(
            c, BT_DBUS_SERVICE, op, BT_DBUS_PROPERTIES_IFACE, "Set",
            g_variant_new("(ssv)", BT_DBUS_ADAPTER_IFACE, "Powered", g_variant_new_boolean(enable)),
            NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &e
        );
        if (!pr) ok = FALSE;
        else g_variant_unref(pr);
    }
    g_variant_iter_free(it); g_variant_unref(od); g_variant_unref(r); g_object_unref(c);
    return found && ok;
}




gboolean set_airplane_mode(gboolean enable) {
    gboolean wifi_success = set_wifi_status(!enable);
    gboolean bt_success = set_bluetooth_status(!enable);
    return wifi_success || bt_success;
}





// gboolean set_ethernet_status(gboolean enable) {
//     GDBusConnection *connection;
//     GError *error = NULL;
//     NMClient *client;
//     const GPtrArray *devices;
//     int success_count = 0;
//     int ethernet_count = 0;
//     client = nm_client_new(NULL, &error);
//     if (!client) {
//         return FALSE;
//     }
//     connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
//     if (!connection) {
//         g_object_unref(client);
//         return FALSE;
//     }
//     devices = nm_client_get_devices(client);
//     for (guint i = 0; devices && i < devices->len; i++) {
//         NMDevice *device = g_ptr_array_index(devices, i);
//         if (NM_IS_DEVICE_ETHERNET(device)) {
//             ethernet_count++;
//             const char *device_path = nm_object_get_path(NM_OBJECT(device));
//             if (!enable) {
//                 GVariant *result = g_dbus_connection_call_sync(
//                     connection,
//                     NM_DBUS_SERVICE,
//                     device_path,
//                     NM_DBUS_INTERFACE ".Device",
//                     "Disconnect",
//                     NULL,
//                     NULL,
//                     G_DBUS_CALL_FLAGS_NONE,
//                     -1,
//                     NULL,
//                     &error
//                 );
//                 if (result) {
//                     g_variant_unref(result);
//                     success_count++;
//                 }
//             } else {
//                 const GPtrArray *available_cons = nm_device_get_available_connections(device);
//                 if (available_cons && available_cons->len > 0) {
//                     NMRemoteConnection *conn = g_ptr_array_index(available_cons, 0);
//                     const char *conn_path = nm_connection_get_path(NM_CONNECTION(conn));
//                     GVariant *result = g_dbus_connection_call_sync(
//                         connection,
//                         NM_DBUS_SERVICE,
//                         NM_DBUS_PATH,
//                         NM_DBUS_INTERFACE,
//                         "ActivateConnection",
//                         g_variant_new("(ooo)", 
//                                      conn_path,
//                                      device_path,
//                                      "/"),
//                         NULL,
//                         G_DBUS_CALL_FLAGS_NONE,
//                         -1,
//                         NULL,
//                         &error
//                     );
//                     if (result) {
//                         g_variant_unref(result);
//                         success_count++;
//                     }
//                 }
//             }
//         }
//     }
//     g_object_unref(connection);
//     g_object_unref(client);
//     return (ethernet_count > 0 && success_count > 0);
// }



static gboolean check_wifi_available(void) {
    NMClient *client = nm_client_new(NULL, NULL);
    if (!client) {
        return FALSE;
    }
    const GPtrArray *devices = nm_client_get_devices(client);
    if (!devices) {
        g_object_unref(client);
        return FALSE;
    }
    for (guint i = 0; i < devices->len; i++) {
        NMDevice *device = g_ptr_array_index(devices, i);
        if (NM_IS_DEVICE_WIFI(device)) {
            g_object_unref(client);
            sidebar_flags |= FLAG_WIFI_TESTDONE;
            return TRUE;
        }
    }
    g_object_unref(client);
    sidebar_flags |= FLAG_WIFI_TESTDONE;
    return FALSE;
}

static gboolean check_bluetooth_available(void) {
    GError *error = NULL;
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (!conn) {
        g_clear_error(&error);
        return FALSE;
    }
    GVariant *iface = g_variant_new_string("org.bluez.Adapter1");
    GVariant *prop = g_variant_new_string("Address");
    GVariant *params = g_variant_new_tuple((GVariant *[]){ iface, prop }, 2);
    GVariant *result = g_dbus_connection_call_sync(
        conn,
        "org.bluez",
        "/org/bluez/hci0",
        "org.freedesktop.DBus.Properties",
        "Get",
        params,
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        2000,
        NULL,
        &error
    );
    gboolean ok = (result != NULL);
    if (result) g_variant_unref(result);
    g_clear_error(&error);
    g_object_unref(conn);
    sidebar_flags |= FLAG_BT_TESTDONE;
    return ok;
}




static void ensure_config_file_exists(const char *username) {
    char config_path[256];
    snprintf(config_path, sizeof(config_path), "/home/%s/.qsidebar/qsidebar.conf", username);
    FILE *config_file = fopen(config_path, "r");
    if (config_file == NULL) {
        char dir_path[256];
        snprintf(dir_path, sizeof(dir_path), "/home/%s/.qsidebar", username);
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            if (mkdir(dir_path, 0755) != 0) {
                g_print("Can't create directory %s: %s\n", dir_path, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        config_file = fopen(config_path, "w");
        if (config_file != NULL) {
            fprintf(config_file, "%s", DEFAULT_CONFIG_CONTENT);
            fclose(config_file);
        } else {
            g_print("Can't create default configuration file %s: %s\n", config_path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        fclose(config_file);
    }
}






static gboolean delayed_restart(gpointer user_data) {
   gboolean *timeout_pending = (gboolean *)user_data;
    if (timeout_pending) {
        *timeout_pending = FALSE;
    }
     if (gtk_main_level() > 0) {
if (sidebar_flags & FLAG_JUST_CHANGED_CONFIG) {
    sidebar_flags &= ~FLAG_JUST_CHANGED_CONFIG;
    return G_SOURCE_REMOVE;
}
         restart_requested = 1;
         gtk_main_quit();
     } else {
        restart_requested = 1;
    }
    return G_SOURCE_REMOVE;
}





static void on_screen_size_changed(GdkScreen *screen, gpointer user_data) {
    static gboolean timeout_pending = FALSE;
    (void)user_data;
    if (timeout_pending) {
        return;
    }
usleep(200000);
    GdkDisplay *display = gdk_display_get_default();
    int n_monitors = gdk_display_get_n_monitors(display);
    if (n_monitors < 1) {
        return;
    }
    timeout_pending = TRUE;
    g_timeout_add(150, delayed_restart, &timeout_pending);
}




static gboolean initialize_backlight(void) {
    const char *backlight_path = "/sys/class/backlight/";
    DIR *dir = opendir(backlight_path);
    if (!dir) {
        g_print("Can't open  %s\n", backlight_path);
        return FALSE;
    }
    struct dirent *entry;
    char device[128] = {0};
    char full_path[256];
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            continue;
        strcpy(full_path, backlight_path);
        strcat(full_path, entry->d_name);
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            strncpy(device, entry->d_name, sizeof(device) - 1);
            break;
        }
    }
    if (!device[0]) {
        closedir(dir);
        g_print("No backlight device found\n");
        return FALSE;
    }
    size_t dirlen = strlen(backlight_path) + strlen(device) + 1;
    backlight_info.directory = malloc(dirlen);
    strcpy(backlight_info.directory, backlight_path);
    strcat(backlight_info.directory, device);
    char max_brightness_path[512], brightness_path[512];
    strcpy(max_brightness_path, backlight_info.directory);
    strcat(max_brightness_path, "/max_brightness");
    strcpy(brightness_path, backlight_info.directory);
    strcat(brightness_path, "/brightness");
    FILE *fp = fopen(max_brightness_path, "r");
    if (!fp) {
        g_print("Can't open max_brightness\n");
        free(backlight_info.directory);
        closedir(dir);
        backlight_info.directory = NULL;
        return FALSE;
    }
    if (fscanf(fp, "%d", &backlight_info.max_brightness) != 1) {
        fclose(fp);
        g_print("Can't read max_brightness\n");
        free(backlight_info.directory);
        closedir(dir);
        backlight_info.directory = NULL;
        return FALSE;
    }
    fclose(fp);
    fp = fopen(brightness_path, "r");
    if (!fp) {
        g_print("Can't open brightness\n");
        free(backlight_info.directory);
        closedir(dir);
        backlight_info.directory = NULL;
        return FALSE;
    }
    if (fscanf(fp, "%d", &backlight_info.current_brightness) != 1) {
        fclose(fp);
        g_print("Can't read brightness\n");
        free(backlight_info.directory);
        closedir(dir);
        backlight_info.directory = NULL;
        return FALSE;
    }
    fclose(fp);
    closedir(dir);
    return TRUE;
}



static void set_brightness_percentage(double p) {
    if (p < 0.0) p = 0.0;
    if (p > 100.0) p = 100.0;
    int max = backlight_info.max_brightness;
    int v = (int)(p * max * 0.01);
    if (v < 1) v = 1;
    if (v > max) v = max;
    char buf[512];
    snprintf(buf, sizeof(buf), "%s/brightness", backlight_info.directory);
    FILE *f = fopen(buf, "w");
    if (f) {
        fprintf(f, "%d\n", v);
        fclose(f);
    }
    backlight_info.current_brightness = v;
}






static void update_current_brightness(void) {
    if (!backlight_info.directory) return;
    char buf[512];
    snprintf(buf, sizeof(buf), "%s/brightness", backlight_info.directory);
    FILE *f = fopen(buf, "r");
    if (!f) return;
    int v;
    if (fscanf(f, "%d", &v) == 1) backlight_info.current_brightness = v;
    fclose(f);
    if (backlight_slider)
        gtk_range_set_value(GTK_RANGE(backlight_slider),
            (double)backlight_info.current_brightness * 100.0 / backlight_info.max_brightness);
}




static void slider_changed(GtkRange *range, gpointer user_data) {
    (void)user_data;
    double value = gtk_range_get_value(range);
    set_brightness_percentage(value);
}




typedef struct {
    DBusConnection *conn;
    volatile gboolean running;
} DBusThreadData;




static void reply_get_server_info(DBusMessage *msg, DBusConnection *conn) {
    DBusMessage *reply = dbus_message_new_method_return(msg);
    DBusMessageIter args;
    dbus_message_iter_init_append(reply, &args);
    const char *vendor = "qsidebar";
    const char *version = "0.9";
    const char *spec_version = "0.9";
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &vendor);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &vendor);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &version);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &spec_version);
    dbus_connection_send(conn, reply, NULL);
    dbus_message_unref(reply);
}




static gboolean trigger_dbus_notification(gpointer user_data) {
    handle_dbus_notification();
    return G_SOURCE_REMOVE;
}




static char *resolve_icon_path(const char *icon_name) {
    if (!icon_name || strlen(icon_name) == 0) {
        return NULL;
    }
    if (g_path_is_absolute(icon_name)) {
        if (g_file_test(icon_name, G_FILE_TEST_EXISTS)) {
            return g_strdup(icon_name);
        }
        return NULL;
    }
    GtkIconTheme *theme = gtk_icon_theme_get_default();
    GtkIconInfo *icon_info = gtk_icon_theme_lookup_icon(theme, icon_name, 48, 0);
    if (icon_info) {
        char *icon_path = g_strdup(gtk_icon_info_get_filename(icon_info));
        g_object_unref(icon_info);
        if (icon_path && g_file_test(icon_path, G_FILE_TEST_EXISTS)) {
            return icon_path;
        }
        g_free(icon_path);
    }
    return NULL;
}




static char *get_app_icon(const char *app_name) {
    if (!app_name || !*app_name) return NULL;
    GList *apps = g_app_info_get_all(), *it = apps;
    GAppInfo *found = NULL;
    for (; it; it = it->next) {
        GAppInfo *a = G_APP_INFO(it->data);
        const char *n = g_app_info_get_name(a);
        if (n && !g_ascii_strcasecmp(n, app_name)) { found = a; break; }
        const char *e = g_app_info_get_executable(a);
        if (e && strstr(e, app_name)) { found = a; break; }
    }
    char *icon_path = NULL;
    if (found) {
        GIcon *icon = g_app_info_get_icon(found);
        if (icon && G_IS_THEMED_ICON(icon)) {
            const char *const *names = g_themed_icon_get_names(G_THEMED_ICON(icon));
            if (names && names[0]) icon_path = resolve_icon_path(names[0]);
        }
    }
    g_list_free_full(apps, g_object_unref);
    return icon_path;
}




static void reply_notify(DBusMessage *msg, DBusConnection *conn) {
    DBusMessageIter a;
    const char *an = "", *ai = "", *su = "", *bo = "";
    dbus_uint32_t rid = 0;
    dbus_int32_t to = -1;
    uint8_t ur = 1;
    if (!dbus_message_iter_init(msg, &a)) return;
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_STRING) { dbus_message_iter_get_basic(&a, &an); dbus_message_iter_next(&a); }
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_UINT32) { dbus_message_iter_get_basic(&a, &rid); dbus_message_iter_next(&a); }
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_STRING) { dbus_message_iter_get_basic(&a, &ai); dbus_message_iter_next(&a); }
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_STRING) { dbus_message_iter_get_basic(&a, &su); dbus_message_iter_next(&a); }
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_STRING) { dbus_message_iter_get_basic(&a, &bo); dbus_message_iter_next(&a); }
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_ARRAY) dbus_message_iter_next(&a);
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_ARRAY) {
        DBusMessageIter d; dbus_message_iter_recurse(&a, &d);
        while (dbus_message_iter_get_arg_type(&d) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter e; dbus_message_iter_recurse(&d, &e);
            const char *k = NULL;
            if (dbus_message_iter_get_arg_type(&e) == DBUS_TYPE_STRING) { dbus_message_iter_get_basic(&e, &k); dbus_message_iter_next(&e); }
            if (k && k[0] == 'u' && !strcmp(k, "urgency")) {
                DBusMessageIter v; dbus_message_iter_recurse(&e, &v);
                if (dbus_message_iter_get_arg_type(&v) == DBUS_TYPE_BYTE) dbus_message_iter_get_basic(&v, &ur);
            }
            dbus_message_iter_next(&d);
        }
        dbus_message_iter_next(&a);
    }
    if (dbus_message_iter_get_arg_type(&a) == DBUS_TYPE_INT32) dbus_message_iter_get_basic(&a, &to);
    if (!g_utf8_validate(su, -1, NULL)) su = "Invalid Summary";
    if (!g_utf8_validate(bo, -1, NULL)) bo = "Invalid Body";
    if (!g_utf8_validate(ai, -1, NULL)) ai = "";
    if (!g_utf8_validate(an, -1, NULL)) an = "";
    if (dbus_notif_store.count < MAX_DBUS_NOTIFICATIONS) {
        sidebar_flags &= ~FLAG_SILENT_THIS;
        gboolean ignore_this = FALSE;
        uint8_t urgency_this = ur;
        for (int i = 0; i < MAX_FILTERS; i++) {
            if (notif_filters[i].type[0] == '\0' || notif_filters[i].string[0] == '\0')
                continue;
            gboolean match = FALSE;
            if (strcmp(notif_filters[i].type, "title") == 0) {
                if (strstr(su, notif_filters[i].string)) match = TRUE;
            } else if (strcmp(notif_filters[i].type, "body") == 0) {
                if (strstr(bo, notif_filters[i].string)) match = TRUE;
            } else if (strcmp(notif_filters[i].type, "title+body") == 0) {
                if (strstr(su, notif_filters[i].string) || strstr(bo, notif_filters[i].string)) match = TRUE;
            } else if (strcmp(notif_filters[i].type, "app_name") == 0) {
                if (strstr(an, notif_filters[i].string)) match = TRUE;
            }
            if (!match)
                continue;
           if (notif_filters[i].exec[0] != '\0') {
               execute_command(notif_filters[i].exec);
           }
            if (notif_filters[i].action[0] != '\0') {
                if (strcmp(notif_filters[i].action, "ignore") == 0) {
                    ignore_this = TRUE;
                    break;
                } else if (strcmp(notif_filters[i].action, "set_urgent") == 0) {
                    urgency_this = 2;
                } else if (strcmp(notif_filters[i].action, "accept_but_silent") == 0) {
                    sidebar_flags |= FLAG_SILENT_THIS;
                }
            }
        }
        if (!ignore_this) {
            DBusNotification *n = &dbus_notif_store.notifications[dbus_notif_store.count];
            strncpy(n->summary, su, MAX_LINE_LENGTH - 1); n->summary[MAX_LINE_LENGTH - 1] = 0;
            strncpy(n->body, bo, MAX_LINE_LENGTH - 1); n->body[MAX_LINE_LENGTH - 1] = 0;
            strncpy(n->app_name, an, MAX_LINE_LENGTH - 1); n->app_name[MAX_LINE_LENGTH - 1] = 0;
            n->timestamp = time(NULL);
            n->urgency = urgency_this;
            char *ip = NULL;
            if (ai && *ai && strncmp(ai, "data:image/", 11) == 0) {
                strncpy(n->icon, ai, MAX_ICON_LENGTH - 1);
                n->icon[MAX_ICON_LENGTH - 1] = 0;
            } else {
                if (ai && *ai) ip = resolve_icon_path(ai);
                if (!ip && an && *an) ip = get_app_icon(an);
                if (ip) {
                    strncpy(n->icon, ip, MAX_LINE_LENGTH - 1);
                    n->icon[MAX_LINE_LENGTH - 1] = 0;
                    g_free(ip);
                } else {
                    const char *def = (urgency_this == 0) ? "/usr/share/qsidebar/icons/notificon_low.png"
                                         : (urgency_this == 2) ? "/usr/share/qsidebar/icons/notificon_critical.png"
                                         : "/usr/share/qsidebar/icons/notificon.png";
                    strncpy(n->icon, def, MAX_LINE_LENGTH - 1);
                    n->icon[MAX_LINE_LENGTH - 1] = 0;
                }
            }
            dbus_notif_store.count++;
            g_idle_add(trigger_dbus_notification, NULL);
        }
    } else {
        g_printerr("D-Bus Notification store is full (max %d)\n", MAX_DBUS_NOTIFICATIONS);
    }
    DBusMessage *r = dbus_message_new_method_return(msg);
    dbus_uint32_t id = (dbus_uint32_t)time(NULL);
    dbus_message_append_args(r, DBUS_TYPE_UINT32, &id, DBUS_TYPE_INVALID);
    dbus_connection_send(conn, r, NULL);
    dbus_message_unref(r);
}




static void *dbus_thread_func(void *user_data) {
    DBusThreadData *data = (DBusThreadData *)user_data;
    while (data->running) {
        dbus_connection_read_write_dispatch(data->conn, 100);
    }
    return NULL;
}




static DBusHandlerResult dbus_message_filter(DBusConnection *conn, DBusMessage *msg, void *user_data) {
    if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "GetServerInformation")) {
        reply_get_server_info(msg, conn);
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if (dbus_message_is_method_call(msg, "org.freedesktop.Notifications", "Notify")) {
        reply_notify(msg, conn);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}




int main(int argc, char *argv[]) {
    username = getenv("USER");
    if (!username) {
        perror("Can't retrieve username");
        exit(EXIT_FAILURE);
    }
    ensure_config_file_exists(username);
    identify_primary_display();
    gtk_init(&argc, &argv);
    DBusError err;
    dbus_error_init(&err);
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        g_printerr("D-Bus Connection Error: %s\n", err.message);
        dbus_error_free(&err);
        return 1;
    }
    if (!conn) {
        g_printerr("Failed to get D-Bus connection\n");
        return 1;
    }
    int ret = dbus_bus_request_name(conn, "org.freedesktop.Notifications", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        g_printerr("D-Bus Name Error: %s\n", err.message);
        dbus_error_free(&err);
        dbus_connection_unref(conn);
        return 1;
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        g_printerr("Not primary owner of org.freedesktop.Notifications or already running.\n");
        dbus_connection_unref(conn);
        return 1;
    }
    dbus_connection_add_filter(conn, dbus_message_filter, NULL, NULL);
    DBusThreadData *dbus_data = g_new0(DBusThreadData, 1);
    dbus_data->conn = conn;
    dbus_data->running = TRUE;
    pthread_t dbus_thread;
    if (pthread_create(&dbus_thread, NULL, dbus_thread_func, dbus_data) != 0) {
        g_printerr("Failed to create D-Bus thread\n");
        dbus_connection_unref(conn);
        g_free(dbus_data);
        return 1;
    }
    setlocale(LC_NUMERIC, "C");
    GdkScreen *screen = gdk_screen_get_default();
    g_signal_connect(screen, "size-changed", G_CALLBACK(on_screen_size_changed), NULL);
    signal(SIGUSR1, handle_signal); //toggle panel visibility
    signal(SIGUSR2, handle_signal); //close panel if opened
    signal(SIGHUP, handle_signal); //reload config file
    //trinity applet specific
    signal(SIGWINCH, handle_signal);//send configuration to trinity applet
    signal(SIGALRM, handle_signal);//toggle focus assist
    signal(SIGVTALRM, handle_signal);//toggle display of notif counter
    signal(SIGXFSZ, handle_signal);//toggle display of notif app icons
    do {
        restart_requested = 0;
        main_sidebar();
    } while (restart_requested);
    cleanup_sidebar();
    dbus_data->running = FALSE;
    pthread_join(dbus_thread, NULL);
    dbus_connection_unref(conn);
    g_free(dbus_data);
    if (font_desc) {
        pango_font_description_free(font_desc);
    }
    if (quickbuttons_font_desc) {
        pango_font_description_free(quickbuttons_font_desc);
    }
    if (projectbuttons_font_desc) {
        pango_font_description_free(projectbuttons_font_desc);
    }
    if (panel_font_desc) {
        pango_font_description_free(panel_font_desc);
    }
    if (paneltitlefont_desc) {
        pango_font_description_free(paneltitlefont_desc);
    }
    return 0;
}




#pragma GCC push_options
#pragma GCC optimize ("O3")
gboolean animate_window_normal(gpointer ud) {
    AnimationData *d = (AnimationData *)ud;
    if (!d->is_animating) return FALSE;
    if (d->is_opening && confirmation_dialog) {
        gtk_widget_destroy(confirmation_dialog);
        confirmation_dialog = NULL;
    }
    int at = render_options.anim_type;
#define MOVE_WIN gtk_window_move(GTK_WINDOW(d->window), d->current_x, d->y_position)
#define SET_OP gtk_widget_set_opacity(d->window, d->current_opacity)
#define HIDE_WIN gtk_widget_hide(d->window)
#define SHOW_WIN gtk_widget_show(d->window)
#define HIDE_CLICK if (d->click_window) gtk_widget_hide(d->click_window)
#define SHOW_CLICK \
    if (d->click_window) { \
        gtk_widget_show(d->click_window); \
        if ((sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) && transparent_click_type) \
            gtk_window_set_keep_above(GTK_WINDOW(d->click_window), TRUE); \
    }
    if (at == ANIM_TYPE_NONE) {
        if (d->is_opening) {
            d->current_x = d->target_x;
            d->current_opacity = render_options.opacity;
            MOVE_WIN; SET_OP; SHOW_WIN; SHOW_CLICK;
        } else {
            d->current_x = d->start_x;
            d->current_opacity = 0.0;
            HIDE_WIN; HIDE_CLICK;
        }
        d->is_animating = FALSE;
        gtk_widget_queue_draw(d->window);
        return FALSE;
    }
    if (at == ANIM_TYPE_SLIDE) {
        if (d->is_opening) {
            if (d->current_x > d->target_x + 5) {
                d->current_x -= d->pas_anim;
                if (d->current_x < d->target_x) d->current_x = d->target_x;
            } else {
                d->current_x = d->target_x;
                d->is_animating = FALSE;
            }
        } else {
            if (d->current_x < d->target_x - 5) {
                d->current_x += d->pas_hide;
                if (d->current_x > d->target_x) d->current_x = d->target_x;
            } else {
                d->current_x = d->target_x;
                d->is_animating = FALSE;
                HIDE_WIN; HIDE_CLICK;
            }
        }
        MOVE_WIN;
    } else if (at == ANIM_TYPE_FADE) {
        const float fs = 0.02f;
        if (d->is_opening) {
            if (d->current_opacity < d->target_opacity - fs)
                d->current_opacity += fs;
            else {
                d->current_opacity = d->target_opacity;
                d->is_animating = FALSE;
            }
            if (d->current_x != d->target_x) {
                d->current_x = d->target_x;
                MOVE_WIN;
            }
        } else {
            if (d->current_opacity > fs)
                d->current_opacity -= fs;
            else {
                d->current_opacity = 0.0;
                d->is_animating = FALSE;
                HIDE_WIN; HIDE_CLICK;
            }
        }
        SET_OP;
    } else if (at == ANIM_TYPE_SLFD) {
        const float fs = 0.02f;
        gboolean sd = FALSE, fd = FALSE;
        if (d->is_opening) {
            if (d->current_x > d->target_x + 5) {
                d->current_x -= d->pas_anim;
                if (d->current_x < d->target_x) d->current_x = d->target_x;
            } else {
                d->current_x = d->target_x;
                sd = TRUE;
            }
            if (d->current_opacity < d->target_opacity - fs)
                d->current_opacity += fs;
            else {
                d->current_opacity = d->target_opacity;
                fd = TRUE;
            }
            MOVE_WIN; SET_OP;
            if (sd && fd) d->is_animating = FALSE;
        } else {
            if (d->current_x < d->start_x - 5) {
                d->current_x += d->pas_hide;
                if (d->current_x > d->start_x) d->current_x = d->start_x;
            } else {
                d->current_x = d->start_x;
                sd = TRUE;
            }
            if (d->current_opacity > fs)
                d->current_opacity -= fs;
            else {
                d->current_opacity = 0.0;
                fd = TRUE;
            }
            MOVE_WIN; SET_OP;
            if (sd && fd) {
                d->is_animating = FALSE;
                HIDE_WIN; HIDE_CLICK;
            }
        }
    }
    gtk_widget_queue_draw(d->window);
    return TRUE;
}
#pragma GCC pop_options





#pragma GCC push_options
#pragma GCC optimize ("O3")
gboolean animate_window_ease(gpointer ud) {
    AnimationData *d = (AnimationData *)ud;
    if (!d->is_animating) return FALSE;
    if (d->is_opening && confirmation_dialog) {
        gtk_widget_destroy(confirmation_dialog);
        confirmation_dialog = NULL;
    }
    int at = render_options.anim_type;
    float ef = 0.05f, ms = 1.0f, mos = 0.005f;
    gboolean anim = TRUE;
    if (at == ANIM_TYPE_NONE) {
        if (d->is_opening) {
            d->current_x = d->target_x;
            d->current_opacity = render_options.opacity;
            gtk_window_move(GTK_WINDOW(d->window), d->current_x, d->y_position);
            gtk_widget_set_opacity(d->window, d->current_opacity);
            gtk_widget_show(d->window);
            if (d->click_window) {
                gtk_widget_show(d->click_window);
                if ((sidebar_flags & FLAG_TRANSPARENT_CLICK_MODE) && transparent_click_type) {
                    gtk_window_set_keep_above(GTK_WINDOW(d->click_window), TRUE);
            }
}
        } else {
            d->current_x = d->start_x;
            d->current_opacity = 0.0f;
            gtk_widget_hide(d->window);
            if (d->click_window) gtk_widget_hide(d->click_window);
        }
        d->is_animating = FALSE;
        gtk_widget_queue_draw(d->window);
        return FALSE;
    }
#define MOVE_WIN gtk_window_move(GTK_WINDOW(d->window), d->current_x, d->y_position)
#define SET_OP gtk_widget_set_opacity(d->window, d->current_opacity)
#define HIDE_WIN gtk_widget_hide(d->window)
#define SHOW_WIN gtk_widget_show(d->window)
    if (at == ANIM_TYPE_SLIDE) {
        float dist;
        if (d->is_opening) {
            dist = d->current_x - d->target_x;
            if (dist > 0.5f) {
                float step = dist * ef;
                if (step < ms) step = ms;
                d->current_x -= step;
                if (d->current_x < d->target_x) d->current_x = d->target_x;
            } else { d->current_x = d->target_x; anim = FALSE; }
        } else {
            dist = d->start_x - d->current_x;
            if (dist > 0.5f) {
                float step = dist * ef;
                if (step < ms) step = ms;
                d->current_x += step;
                if (d->current_x > d->start_x) d->current_x = d->start_x;
            } else {
                d->current_x = d->start_x; anim = FALSE; HIDE_WIN;
                if (d->click_window) gtk_widget_hide(d->click_window);
            }
        }
        MOVE_WIN;
        d->is_animating = anim;
        gtk_widget_queue_draw(d->window);
        return anim;
    }
    if (at == ANIM_TYPE_FADE) {
        float od;
        if (d->is_opening) {
            od = d->target_opacity - d->current_opacity;
            if (od > 0.002f) {
                float step = od * ef;
                if (step < mos) step = mos;
                d->current_opacity += step;
                if (d->current_opacity > d->target_opacity) d->current_opacity = d->target_opacity;
            } else { d->current_opacity = d->target_opacity; anim = FALSE; }
            if (d->current_x != d->target_x) { d->current_x = d->target_x; MOVE_WIN; }
        } else {
            od = d->current_opacity;
            if (od > 0.002f) {
                float step = od * ef;
                if (step < mos) step = mos;
                d->current_opacity -= step;
                if (d->current_opacity < 0.0f) d->current_opacity = 0.0f;
            } else { d->current_opacity = 0.0f; anim = FALSE; HIDE_WIN;
                if (d->click_window) gtk_widget_hide(d->click_window);
            }
        }
        SET_OP;
        d->is_animating = anim;
        gtk_widget_queue_draw(d->window);
        return anim;
    }
    if (at == ANIM_TYPE_SLFD) {
        gboolean sd = FALSE, fd = FALSE;
        float dist, od;
        if (d->is_opening) {
            dist = d->current_x - d->target_x;
            if (dist > 0.5f) {
                float step = dist * ef;
                if (step < ms) step = ms;
                d->current_x -= step;
                if (d->current_x < d->target_x) d->current_x = d->target_x;
            } else { d->current_x = d->target_x; sd = TRUE; }
            od = d->target_opacity - d->current_opacity;
            if (od > 0.002f) {
                float step = od * ef;
                if (step < mos) step = mos;
                d->current_opacity += step;
                if (d->current_opacity > d->target_opacity) d->current_opacity = d->target_opacity;
            } else { d->current_opacity = d->target_opacity; fd = TRUE; }
            MOVE_WIN; SET_OP;
            if (sd && fd) anim = FALSE;
        } else {
            dist = d->start_x - d->current_x;
            if (dist > 0.5f) {
                float step = dist * ef;
                if (step < ms) step = ms;
                d->current_x += step;
                if (d->current_x > d->start_x) d->current_x = d->start_x;
            } else { d->current_x = d->start_x; sd = TRUE; }
            od = d->current_opacity;
            if (od > 0.002f) {
                float step = od * ef;
                if (step < mos) step = mos;
                d->current_opacity -= step;
                if (d->current_opacity < 0.0f) d->current_opacity = 0.0f;
            } else { d->current_opacity = 0.0f; fd = TRUE; }
            MOVE_WIN; SET_OP;
            if (sd && fd) { anim = FALSE; HIDE_WIN;
                if (d->click_window) gtk_widget_hide(d->click_window);
            }
        }
        d->is_animating = anim;
        gtk_widget_queue_draw(d->window);
        return anim;
    }
    d->is_animating = FALSE;
    gtk_widget_queue_draw(d->window);
    return FALSE;
}
#pragma GCC pop_options




gboolean animate_window(gpointer ud) {
    if (sidebar_flags & FLAG_EASE_EFFECT) {
        return animate_window_ease(ud);
    } else {
        return animate_window_normal(ud);
    }
}




gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation a;
    gtk_widget_get_allocation(widget, &a);
    if (widget == window) {
    gboolean need_background;
    if (sidebar_flags & FLAG_IS_BACKGROUND) {
      if (sidebar_flags & FLAG_PANEL_SOLIDBACKGROUND) {
            need_background = (background_width < a.width) || (background_height < a.height);
        } else {
            need_background = FALSE;
        }
    } else {
        need_background = TRUE;
    }
        if (need_background) {
            cairo_set_source_rgba(cr, render_options.tint_r, render_options.tint_g, render_options.tint_b, render_options.opacity);
            cairo_rectangle(cr, 0, 0, a.width, a.height);
            cairo_fill(cr);
        }
          if (sidebar_flags & FLAG_IS_BACKGROUND) {
            int win_width = a.width;
            int win_height = a.height;
            int crop_x = 0, crop_y = 0;
            if (background_width > win_width)
                crop_x = (background_width - win_width) / 2;
            if (background_height > win_height)
                crop_y = (background_height - win_height) / 2;
            cairo_set_source_surface(cr, background_source, -crop_x, -crop_y);
            cairo_paint(cr);
        }
    }
    else if (widget == notification_popup) {
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_rectangle(cr, 0, 0, a.width, a.height);
        cairo_fill(cr);
    } else {
        cairo_set_source_rgba(cr, render_options.tint_r, render_options.tint_g, render_options.tint_b, render_options.opacity);
        cairo_rectangle(cr, 0, 0, a.width, a.height);
        cairo_fill(cr);
    }
    return FALSE;
}





