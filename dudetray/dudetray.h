#ifndef _DUDETRAY_H_
#define _DUDETRAY_H_

struct Message {
    gchar *date;
    gchar *subject;
    gchar *description;
    gboolean is_new;
};

/**
 * Create the tray icon (GtkStatusIcon)
 */
void create_tray_icon();

/**
 * Update the icon
 *
 * This effectively toggles the icon from "greyed-out" to colored, meaning
 * there are messages
 *
 * @param gboolean has_message Whether a message is present
 */
static void update_icon(gboolean has_message);

/**
 * Update the tooltip message
 *
 * @param gchar message The message
 */
static void update_tooltip(gchar *message);

/**
 * Callback for the 'activate' signal
 *
 * @param GtkStatusIcon *status_icon The status icon instance
 * @param gpointer user_data User data from click event
 */
void tray_icon_on_click(GtkStatusIcon *status_icon,
    gpointer user_data);

/**
 *
 */
void tray_icon_on_popup(GtkStatusIcon *status_icon,
    guint button,
    guint activate_time,
    gpointer user_data);

/**
 *
 */
void add_menu_item(struct Message *message);

void trigger_display_message_dialog(GtkMenuItem *menu_item, struct Message *message);
void display_message_dialog(struct Message *message);

/**
 *
 */
gboolean save_notification(struct Message *message);

/**
 *
 */
struct Message *parse_full_message(gchar *full_message);

/**
 *
 */
void tray_menu_clear_on_click(GtkStatusIcon *status_icon, gpointer user_data);

/**
 *
 */
void tray_menu_quit_on_click(GtkStatusIcon *status_icon, gpointer user_data);

gboolean tray_icon_heartbeat(gpointer data);

gboolean read_status_file(char *filename);

#endif /* _DUDETRAY_H_ */
