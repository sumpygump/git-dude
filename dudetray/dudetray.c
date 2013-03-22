#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <dirent.h>
#include <gtk/gtk.h> 
#include <gdk/gdk.h> 

#include "dudetray.h"
#include "media/git-logo-16.c"
#include "media/git-logo-grey-16.c"

#define MAX_MESSAGES 10

/* The GTK tray object */
GtkStatusIcon *tray_icon; 

/* The GTK menu object */
GtkWidget *tray_menu = NULL;

/* The most recent message parsed */
gchar *latest_message;

/* A list of notification_messages */
struct Message *notification_messages[MAX_MESSAGES];

/* The path where the notification files will live */
gchar notifications_path[1024];

/* The current index where to place new messages */
int notification_cursor = 0;

void create_tray_icon()
{ 
    tray_icon = gtk_status_icon_new(); 
    g_signal_connect(G_OBJECT(tray_icon), "activate",
        G_CALLBACK(tray_icon_on_click), NULL
    );

    g_signal_connect(G_OBJECT(tray_icon), "popup-menu",
        G_CALLBACK(tray_icon_on_popup), NULL
    );

    g_timeout_add(1000, tray_icon_heartbeat, NULL);

    update_icon(FALSE);
    update_tooltip("Git dude monitor");
    gtk_status_icon_set_visible(tray_icon, TRUE); 
} 

static void update_icon(gboolean has_message)
{
    GError *gerror = NULL; 
    GdkPixbuf *a_pixbuf; 

    if (has_message) {
        a_pixbuf = gdk_pixbuf_new_from_inline(-1, git_logo_16_gdkp, FALSE, &gerror);
    } else {
        a_pixbuf = gdk_pixbuf_new_from_inline(-1, git_logo_grey_16_gdkp, FALSE, &gerror);
    }

    if (!a_pixbuf) {
        fprintf(stderr, "%s\n", gerror->message);
        g_error_free(gerror);
    }

    gtk_status_icon_set_from_pixbuf(tray_icon, a_pixbuf); 
}

static void update_tooltip(gchar *message)
{
    gtk_status_icon_set_tooltip(tray_icon, message); 
}

void tray_icon_on_click(GtkStatusIcon *status_icon, 
    gpointer user_data) 
{ 
    printf("Clicked on tray icon\n");
    update_icon(TRUE);
    //system("example.sh"); 
}

void tray_icon_on_popup(GtkStatusIcon *status_icon,
    guint button,
    guint activate_time,
    gpointer user_data)
{
    if (!tray_menu) {
        GtkWidget *item_clear;
        GtkWidget *item_quit;
        GtkWidget *item_separator;
        tray_menu = gtk_menu_new();

        item_clear = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLEAR, NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL (tray_menu), item_clear);
        g_signal_connect(G_OBJECT(item_clear), "activate",
            G_CALLBACK(tray_menu_clear_on_click),
            GUINT_TO_POINTER(TRUE)
        );

        item_quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL (tray_menu), item_quit);
        g_signal_connect(G_OBJECT(item_quit), "activate",
            G_CALLBACK(tray_menu_quit_on_click),
            GUINT_TO_POINTER(TRUE)
        );

        item_separator = gtk_separator_menu_item_new();
        gtk_menu_shell_prepend(GTK_MENU_SHELL (tray_menu), item_separator);

        int i;
        struct Message *message;
        for (i = 0; i < MAX_MESSAGES; i++) {
            message = notification_messages[i];
            if (message) {
                add_menu_item(message);
            }
        }
    }

    gtk_widget_show_all(tray_menu);

    gtk_menu_popup(GTK_MENU(tray_menu),
        NULL,
        NULL,
        gtk_status_icon_position_menu,
        status_icon,
        button,
        activate_time
    );
} 

gboolean tray_icon_heartbeat(gpointer data)
{
    gboolean status;
    gboolean save_status;
    DIR *dir;
    struct dirent *ent;

    dir = opendir(notifications_path);
    if (dir == NULL) {
        // no notifications
        closedir(dir);
        return FALSE;
    }

    while ((ent = readdir (dir)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
            continue;
        }

        gchar filepath[1024];
        strcpy(filepath, notifications_path);
        strcat(filepath, ent->d_name);

        status = read_status_file(filepath);

        if (status) {
            struct Message *new_message = parse_full_message(latest_message);
            message_print(new_message);

            save_status = save_notification(new_message);
            if (!save_status) {
                // If it didn't save, get out.
                return FALSE;
            }

            // Remove the file so we don't read it again.
            remove(filepath);

            update_tooltip("Git dude: repositories updated");
            update_icon(TRUE);
        }
    }
    closedir(dir);
    
    return TRUE;
}

void tray_menu_clear_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
    update_icon(FALSE);
    update_tooltip("Git dude monitor");

    // Clear out all the notification_messages
    memset(&notification_messages[0], 0, sizeof(notification_messages));

    // Hide the menu and then remove each of the children
    gtk_widget_hide_all(tray_menu);

    GList *children;
    GList *list;

    children = gtk_container_get_children (GTK_CONTAINER (tray_menu));
    for (list = children; list; list = g_list_next (list)) {
        GtkWidget *menu_item = GTK_WIDGET (list->data);
        gtk_container_remove(GTK_CONTAINER (tray_menu), menu_item);
    }

    // Null out the menu so it is created anew next time it's needed
    tray_menu = NULL;

    // Reset the cursor
    notification_cursor = 0;
}

void tray_menu_quit_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
    gtk_main_quit();
}

gboolean save_notification(struct Message *message)
{
    notification_messages[notification_cursor] = message;
    notification_cursor++;

    if (notification_cursor > MAX_MESSAGES) {
        return FALSE;
    }

    if (tray_menu) {
        add_menu_item(message);
    }

    return TRUE;
}

void add_menu_item(struct Message *message)
{
    GtkWidget *item;

    item = gtk_menu_item_new_with_label(message->subject);
    gtk_menu_shell_prepend(GTK_MENU_SHELL (tray_menu), item);

    g_signal_connect(G_OBJECT(item), "activate",
        G_CALLBACK(trigger_display_message_dialog), (gpointer) message);

    gtk_widget_show_all(tray_menu);
}

void trigger_display_message_dialog(GtkMenuItem *menu_item, struct Message *message)
{
    display_message_dialog(message);
}

void display_message_dialog(struct Message *message)
{
    GtkWidget *dialog;
    gchar *description;

    description = malloc(
        (strlen(message->date) + strlen(message->description)) * sizeof(gchar*)
        + (sizeof(gchar*) * 3)
    );
    sprintf(description, "%s\n%s", message->date, message->description);

    dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_CLOSE,
        message->subject, NULL
    );

    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG (dialog), description, NULL);

    gtk_dialog_run(GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);

    free(description);
}

gboolean read_status_file(char *filename)
{
    FILE *input = fopen(filename, "r");
    gchar *buffer;
    int fileLen;

    if (input == NULL) {
        //fprintf(stderr, "#fail Cannot open file `%s'.\n", filename);
        return FALSE;
    }

    //Get file length
    fseek(input, 0, SEEK_END);
    fileLen = ftell(input);
    fseek(input, 0, SEEK_SET);

    buffer = (gchar *) malloc(fileLen * sizeof(gchar*) + sizeof(gchar*));
    latest_message = (gchar *) malloc(fileLen * sizeof(gchar*) + sizeof(gchar*));

    if (!buffer) {
        fprintf(stderr, "#fail Cannot allocate enough memory to store contents of file `%s'.\n", filename);
        return FALSE;
    }

    size_t fread_result;
    fread_result = fread(buffer, fileLen, 1, input);

    if (!fread_result) {
        return FALSE;
    }

    strcpy(latest_message, buffer);

    fclose(input);
    free(buffer);

    return TRUE;
}

struct Message *parse_full_message(gchar *full_message)
{
    gchar *date;
    gchar *subject;
    gchar *description;

    date = strtok(full_message, "\n");
    subject = strtok(NULL, "\n");
    description = strtok(NULL, "\0");

    return message_create(
        date,
        subject,
        description,
        TRUE
    );
}

struct Message *message_create(gchar *date, gchar *subject, gchar *description, gboolean is_new)
{
    struct Message *msg = malloc(sizeof(struct Message));
    assert(msg != NULL);

    msg->date = strdup(date);
    msg->subject = strdup(subject);

    if (!description) {
        description = "No description";
    }
    msg->description = strdup(description);

    msg->is_new = is_new;

    return msg;
}

void message_destroy(struct Message *msg)
{
    assert(msg != NULL);

    free(msg->date);
    free(msg->subject);
    free(msg->description);
    free(msg);
}

void message_print(struct Message *msg)
{
    printf("Date: %s\n", msg->date);
    printf("Subject: %s\n", msg->subject);
    printf("Isnew: %d\n", msg->is_new);
    printf("Description:\n %s\n", msg->description);
}

int main(int argc, char **argv)
{ 
    gchar notifications_dir[1024];
    gchar home_path[64];

    strcpy(home_path, getenv("HOME"));

    strcpy(notifications_dir, "/.dude-notifications");
    strcpy(notifications_path, home_path);
    strcat(notifications_path, notifications_dir);
    strcat(notifications_path, "/");

    gtk_init(&argc, &argv);
    create_tray_icon();
    gtk_main();

    return 0; 
} 
