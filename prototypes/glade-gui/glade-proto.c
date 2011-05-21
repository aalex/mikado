/*
 * Copyright (C) 2007 Micah Carrick <email@micahcarrick.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------------------
 *
 * This file is part of the tutorial: Linux GUI Programming with GTK+ and Glade3
 * http://www.micahcarrick.com/12-24-2007/gtk-glade-tutorial-part-1.html
 *
 * Compile using:
 * gcc -Wall -g -o tutorial main.c -export-dynamic `pkg-config gtk+-2.0 libglade-2.0 --cflags --libs`      
*/

#include <gtk/gtk.h>

/* location of UI XML file relative to path in which program is running */
#define BUILDER_XML_FILE "tutorial.xml"

/* store the widgets which may need to be accessed in a typedef struct */
typedef struct
{
        GtkWidget               *window;
        GtkWidget               *statusbar;
        GtkWidget               *text_view;
        guint                   statusbar_context_id;
        gchar                   *filename;
} TutorialTextEditor;

/* window callback prototypes */
void on_window_destroy (GtkObject *object, TutorialTextEditor *editor);
gboolean on_window_delete_event (GtkWidget *widget, GdkEvent *event,
                                 TutorialTextEditor *editor);

/* file menu callbacks prototypes */
void on_new_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);
void on_open_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);
void on_save_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);
void on_save_as_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);
void on_quit_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);

/* edit menu callback prototypes */
void on_cut_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);
void on_copy_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);
void on_paste_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);
void on_delete_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);

/* help menu callback prototypes */
void on_about_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor);

/* misc. function prototypes */
void error_message (const gchar *message);
gboolean init_app (TutorialTextEditor *editor);
gboolean check_for_save (TutorialTextEditor *editor);
gchar* get_open_filename (TutorialTextEditor *editor);
gchar* get_save_filename (TutorialTextEditor *editor);
void load_file (TutorialTextEditor *editor, gchar *filename);
void write_file (TutorialTextEditor *editor, gchar *filename);
void reset_default_status (TutorialTextEditor *editor);

int
main (int argc, char *argv[])
{
        TutorialTextEditor      *editor;

        /* allocate the memory needed by our TutorialTextEditor struct */
        editor = g_slice_new (TutorialTextEditor);

        /* initialize GTK+ libraries */
        gtk_init (&argc, &argv);

        if (init_app (editor) == FALSE) return 1; /* error loading UI */

        /* show the window */   
        gtk_widget_show (editor->window);

        /* enter GTK+ main loop */                   
        gtk_main ();

        /* free memory we allocated for TutorialTextEditor struct */
        g_slice_free (TutorialTextEditor, editor);
        
        return 0;
}

/*
We call error_message() any time we want to display an error message to the
user. It will both show an error dialog and log the error to the terminal
window.
*/
void
error_message (const gchar *message)
{
        GtkWidget               *dialog;
        
        /* log to terminal window */
        g_warning (message);
        
        /* create an error message dialog and display modally to the user */
        dialog = gtk_message_dialog_new (NULL, 
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         message);
        
        gtk_window_set_title (GTK_WINDOW (dialog), "Error!");
        gtk_dialog_run (GTK_DIALOG (dialog));      
        gtk_widget_destroy (dialog);         
}

/*
We call init_app() when our program is starting to load our TutorialTextEditor struct
with references to the widgets we need. This is done using GtkBuilder to read
the XML file we created using Glade.
*/
gboolean 
init_app (TutorialTextEditor *editor)
{
        GtkBuilder              *builder;
        GError                  *err=NULL;      
        guint                   id;
        PangoFontDescription    *font_desc;
        
        /* use GtkBuilder to build our interface from the XML file */
        builder = gtk_builder_new ();
        if (gtk_builder_add_from_file (builder, BUILDER_XML_FILE, &err) == 0)
        {
                error_message (err->message);
                g_error_free (err);
                return FALSE;
        }
        
        /* get the widgets which will be referenced in callbacks */
        editor->window = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                             "window"));
        editor->statusbar = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                                "statusbar"));
        editor->text_view = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                                "text_view"));
                                                                
        /* connect signals, passing our TutorialTextEditor struct as user data */
        gtk_builder_connect_signals (builder, editor);
                    
        /* free memory used by GtkBuilder object */
        g_object_unref (G_OBJECT (builder));
        
        /* set the text view font */
        font_desc = pango_font_description_from_string ("monospace 10");
        gtk_widget_modify_font (editor->text_view, font_desc);     
        pango_font_description_free (font_desc); 
        
        /* set the default icon to the GTK "edit" icon */
        gtk_window_set_default_icon_name (GTK_STOCK_EDIT);
        
        /* setup and initialize our statusbar */
        id = gtk_statusbar_get_context_id (GTK_STATUSBAR (editor->statusbar),
                                           "Tutorial GTK+ Text Editor");
        editor->statusbar_context_id = id;
        reset_default_status (editor);
        
        /* set filename to NULL since we don't have an open/saved file yet */
        editor->filename = NULL;
        
        return TRUE;
}

/*
We call get_open_filename() when we want to get a filename to open from the
user. It will present the user with a file chooser dialog and return the 
newly allocated filename or NULL.
*/
gchar *
get_open_filename (TutorialTextEditor *editor)
{
        GtkWidget               *chooser;
        gchar                   *filename=NULL;
                
        chooser = gtk_file_chooser_dialog_new ("Open File...",
                                               GTK_WINDOW (editor->window),
                                               GTK_FILE_CHOOSER_ACTION_OPEN,
                                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                               NULL);
                                               
        if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
        {
                filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
        }
        
        gtk_widget_destroy (chooser);
        return filename;
}

/*
We call get_save_filename() when we want to get a filename to save from the
user. It will present the user with a file chooser dialog and return the 
newly allocated filename or NULL.
*/
gchar *
get_save_filename (TutorialTextEditor *editor)
{
        GtkWidget               *chooser;
        gchar                   *filename=NULL;
                
        chooser = gtk_file_chooser_dialog_new ("Save File...",
                                               GTK_WINDOW (editor->window),
                                               GTK_FILE_CHOOSER_ACTION_SAVE,
                                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_SAVE, GTK_RESPONSE_OK,
                                               NULL);
                                               
        if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK)
        {
                filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
        }
        
        gtk_widget_destroy (chooser);
        return filename;
}

/* 
We call load_file() when we have a filename and want to load it into the buffer
for the GtkTextView. The previous contents are overwritten.
*/
void 
load_file (TutorialTextEditor *editor, gchar *filename)
{
        GError                  *err=NULL;
        gchar                   *status;
        gchar                   *text;
        gboolean                result;
        GtkTextBuffer           *buffer;
        
        /* add Loading message to status bar and  ensure GUI is current */
        status = g_strdup_printf ("Loading %s...", filename);
        gtk_statusbar_push (GTK_STATUSBAR (editor->statusbar),
                            editor->statusbar_context_id, status);
        g_free (status);
        while (gtk_events_pending()) gtk_main_iteration();
        
        /* get the file contents */
        result = g_file_get_contents (filename, &text, NULL, &err);
        if (result == FALSE)
        {
                /* error loading file, show message to user */
                error_message (err->message);
                g_error_free (err);
                g_free (filename);
        }
        
        /* disable the text view while loading the buffer with the text */    
        gtk_widget_set_sensitive (editor->text_view, FALSE);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        gtk_text_buffer_set_text (buffer, text, -1);
        gtk_text_buffer_set_modified (buffer, FALSE);
        gtk_widget_set_sensitive (editor->text_view, TRUE);
        g_free (text); 
        
        /* now we can set the current filename since loading was a success */
        if (editor->filename != NULL) g_free (editor->filename);
        editor->filename = filename;
        
        /* clear loading status and restore default  */
        gtk_statusbar_pop (GTK_STATUSBAR (editor->statusbar),
                           editor->statusbar_context_id);
        reset_default_status (editor);
}

/* 
We call write_file() when we have a filename and want to load the current text
buffer for the GtkTextView into the file.
*/
void 
write_file (TutorialTextEditor *editor, gchar *filename)
{
        GError                  *err=NULL;
        gchar                   *status;
        gchar                   *text;
        gboolean                result;
        GtkTextBuffer           *buffer;
        GtkTextIter             start, end;
        
        /* add Saving message to status bar and ensure GUI is current */
        if (filename != NULL)
            status = g_strdup_printf ("Saving %s...", filename);
        else
            status = g_strdup_printf ("Saving %s...", editor->filename);
            
        gtk_statusbar_push (GTK_STATUSBAR (editor->statusbar),
                            editor->statusbar_context_id, status);
        g_free (status);
        while (gtk_events_pending()) gtk_main_iteration();
        
        /* disable text view and get contents of buffer */ 
        gtk_widget_set_sensitive (editor->text_view, FALSE);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        gtk_text_buffer_get_start_iter (buffer, &start);
        gtk_text_buffer_get_end_iter (buffer, &end);
        text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);       
        gtk_text_buffer_set_modified (buffer, FALSE);
        gtk_widget_set_sensitive (editor->text_view, TRUE);
        
        /* set the contents of the file to the text from the buffer */
        if (filename != NULL)        
                result = g_file_set_contents (filename, text, -1, &err);
        else
                result = g_file_set_contents (editor->filename, text, -1, &err);
                
        if (result == FALSE)
        {
                /* error saving file, show message to user */
                error_message (err->message);
                g_error_free (err);
        }        
        
        /* don't forget to free that memory! */ 
        g_free (text); 
        
        if (filename != NULL) 
        {
                /* we need to free the memory used by editor->filename and set 
                 it to the new filename instead */
                if (editor->filename != NULL) g_free (editor->filename);
                editor->filename = filename;
        }
        
        /* clear saving status and restore default */
        gtk_statusbar_pop (GTK_STATUSBAR (editor->statusbar),
                           editor->statusbar_context_id);
        reset_default_status (editor);   
}

/*
We call reset_default_status() when we want to remove the last default status
and set it again based on the current file. This is typically after a file is
opened, saved, or a new file is created. "Default" simply means a non-temporary
status. It's typically something like "File: filename.txt"
*/
void 
reset_default_status (TutorialTextEditor *editor)
{
        gchar           *file;
        gchar           *status;
        
        if (editor->filename == NULL)
        {
                file = g_strdup ("(UNTITLED)");
        }
        else file = g_path_get_basename (editor->filename);
        
        status = g_strdup_printf ("File: %s", file);
        gtk_statusbar_pop (GTK_STATUSBAR (editor->statusbar),
                           editor->statusbar_context_id);
        gtk_statusbar_push (GTK_STATUSBAR (editor->statusbar),
                            editor->statusbar_context_id, status);
        g_free (status);
        g_free (file);
}
        
/* 
This function will check to see if the text buffer has been
modified and prompt the user to save if it has been modified.
*/
gboolean
check_for_save (TutorialTextEditor *editor)
{
        gboolean                ret = FALSE;
        GtkTextBuffer           *buffer;
        
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        
        if (gtk_text_buffer_get_modified (buffer) == TRUE)
        {
                /* we need to prompt for save */
                
                GtkWidget       *dialog;
                
                const gchar *msg  = "Do you want to save the changes you have made?";
                
                dialog = gtk_message_dialog_new (NULL, 
                                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 msg);
        
                gtk_window_set_title (GTK_WINDOW (dialog), "Save?");
                if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_NO)
                {
                        ret = FALSE;
                }      
                else ret = TRUE;
                
                gtk_widget_destroy (dialog);      
        }     
        
        return ret;
}

/* 
When our window is destroyed, we want to break out of the GTK main loop. We do
this by calling gtk_main_quit(). We could have also just specified gtk_main_quit
as the handler in Glade!
*/
void 
on_window_destroy (GtkObject *object, TutorialTextEditor *editor)
{
        gtk_main_quit();
}

/*
When the window is requested to be closed, we need to check if they have 
unsaved work. We use this callback to prompt the user to save their work before
they exit the application. From the "delete-event" signal, we can choose to
effectively cancel the close based on the value we return.
*/
gboolean 
on_window_delete_event (GtkWidget *widget, GdkEvent *event, 
                        TutorialTextEditor *editor)
{
        if (check_for_save (editor) == TRUE)
        {
              on_save_menu_item_activate (NULL, editor);  
        }
        return FALSE;   /* propogate event */
}

/*
Called when the user clicks the 'New' menu. We need to prompt for save if the
file has been modified, and then delete the buffer and clear the modified flag.
*/
void on_new_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        GtkTextBuffer           *buffer;
        
        if (check_for_save (editor) == TRUE)
        {
              on_save_menu_item_activate (NULL, editor);  
        }
        
        /* clear editor for a new file */
        editor->filename = NULL;
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        gtk_text_buffer_set_text (buffer, "", -1);
        gtk_text_buffer_set_modified (buffer, FALSE);
        
        reset_default_status (editor);
}

/*
Called when the user clicks the 'Open' menu. We need to prompt for save if the
file has been modified, allow the user to choose a file to open, and then call
load_file() on that file.
*/
void on_open_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        gchar                   *filename;
        
        if (check_for_save (editor) == TRUE)
        {
              on_save_menu_item_activate (NULL, editor);  
        }
        
        filename = get_open_filename (editor);
        if (filename != NULL) load_file (editor, filename); 
}

/*
Called when the user clicks the 'Save' menu. We need to allow the user to choose 
a file to save if it's an untitled document, and then call write_file() on that 
file.
*/
void on_save_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        gchar                   *filename;
        
        if (editor->filename == NULL) 
        {
                filename = get_save_filename (editor);
                if (filename != NULL) write_file (editor, filename); 
        }
        else write_file (editor, NULL);
        
}

/*
Called when the user clicks the 'Save As' menu. We need to allow the user to 
choose a file to save and then call write_file() on that file.
*/
void on_save_as_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        gchar                   *filename;
        
        filename = get_save_filename (editor);
        if (filename != NULL) write_file (editor, filename); 
}

/*
Called when the user clicks the 'Quit' menu. We need to prompt for save if the
file has been modified and then break out of the GTK+ main loop.
*/
void on_quit_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        if (check_for_save (editor) == TRUE)
        {
              on_save_menu_item_activate (NULL, editor);  
        }
        gtk_main_quit();
}

/*
Called when the user clicks the 'Cut' menu. 
*/
void on_cut_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        GtkTextBuffer           *buffer;
        GtkClipboard            *clipboard;
        
        clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        gtk_text_buffer_cut_clipboard (buffer, clipboard, TRUE);
}

/*
Called when the user clicks the 'Copy' menu. 
*/
void on_copy_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        GtkTextBuffer           *buffer;
        GtkClipboard            *clipboard;
        
        clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        gtk_text_buffer_copy_clipboard (buffer, clipboard);
}

/*
Called when the user clicks the 'Paste' menu. 
*/
void on_paste_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        GtkTextBuffer           *buffer;
        GtkClipboard            *clipboard;
        
        clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        gtk_text_buffer_paste_clipboard (buffer, clipboard, NULL, TRUE);
}

/*
Called when the user clicks the 'Delete' menu. 
*/
void on_delete_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        GtkTextBuffer           *buffer;

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->text_view));
        gtk_text_buffer_delete_selection (buffer, FALSE, TRUE);
}

/*
Called when the user clicks the 'About' menu. We use gtk_show_about_dialog() 
which is a convenience function to show a GtkAboutDialog. This dialog will
NOT be modal but will be on top of the main application window.
*/
void on_about_menu_item_activate (GtkMenuItem *menuitem, TutorialTextEditor *editor)
{
        static const gchar * const authors[] = {
		"Micah Carrick <email@micahcarrick.com>",
		NULL
	};

	static const gchar copyright[] = \
		"Copyright \xc2\xa9 2007 Micah Carrick";

	static const gchar comments[] = "GTK+ and Glade3 GUI Programming Tutorial";

	gtk_show_about_dialog (GTK_WINDOW (editor->window),
			       "authors", authors,
			       "comments", comments,
			       "copyright", copyright,
			       "version", "0.1",
			       "website", "http://www.micahcarrick.com",
			       "program-name", "GTK+ Text Editor",
			       "logo-icon-name", GTK_STOCK_EDIT,
			       NULL); 
}



