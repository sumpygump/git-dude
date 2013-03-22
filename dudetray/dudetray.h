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
 * @param gchar *message The message
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
 * Callback for the popup-menu signal
 *
 * @param GtkStatusIcon *status_icon The status icon instance
 * @param guint button
 * @param guint activate_time 
 * @param gpointer user_data User data from event 
 */
void tray_icon_on_popup(GtkStatusIcon *status_icon,
    guint button,
    guint activate_time,
    gpointer user_data);

/**
 * The heartbeat that runs on GTK's timer
 *
 * @param gpointer data Event data from GTK
 */
gboolean tray_icon_heartbeat(gpointer data);

/**
 * Callback for clicking the clear menu item
 *
 * @param GtkStatusIcon *status_icon The status icon instance
 * @param gpointer user_data Event data from GTK
 */
void tray_menu_clear_on_click(GtkStatusIcon *status_icon, gpointer user_data);

/**
 * Callback for clicking on the quit menu item
 *
 * @param GtkStatusIcon *status_icon The status icon instance
 * @param gpointer user_data Event data from GTK
 */
void tray_menu_quit_on_click(GtkStatusIcon *status_icon, gpointer user_data);

/**
 * Save notification message into internal memory
 *
 * @param Message *message The message object
 */
gboolean save_notification(struct Message *message);

/**
 * Add a menu item
 *
 * @param Message *message The message object to add to menu
 */
void add_menu_item(struct Message *message);

/**
 * Callback to trigger the display of a GTK message dialog
 *
 * @param GtkMenuItem *menu_item The menu object
 * @param Message *message The message object
 */
void trigger_display_message_dialog(GtkMenuItem *menu_item, struct Message *message);

/**
 * Display a GTK message dialog with the message object
 *
 * @param Message *message The message object
 */
void display_message_dialog(struct Message *message);

/**
 * Read a file that comprises a new notification
 *
 * @param char *filename
 */
gboolean read_status_file(char *filename);

/**
 * Print a message (for debugging)
 *
 * @param Message *msg Message object
 */
void message_print(struct Message *msg);

/**
 * Parse a full message
 *
 * @param gchar *full_message The full message string
 */
struct Message *parse_full_message(gchar *full_message);

/**
 * Creates a new message object from some inputs
 *
 * @param gchar *date A data representation (string)
 * @param gchar *subject The message subject
 * @param gchar *description The message description
 * @param gboolean is_new Whether this notification is new (currently unused)
 */
struct Message *message_create(gchar *date, gchar *subject, gchar *description, gboolean is_new);

/**
 * Destroy a message object (struct)
 *
 * @param Message *msg Message object
 */
void message_destroy(struct Message *msg);

/**
 * Print out a message object (struct)
 *
 * @param Message *msg Message object
 */
void message_destroy(struct Message *msg);

#endif /* _DUDETRAY_H_ */
