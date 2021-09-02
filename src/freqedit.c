/*********************************************************************************
 * Copyright 2021 Dilshan R Jayakody. [jayakody2000lk@gmail.com]                 *
 *                                                                               *
 * Permission is hereby granted, free of charge, to any person obtaining a       *
 * copy of this software and associated documentation files (the "Software"),    *
 *  to deal in the Software without restriction, including without limitation    *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,      *
 * and/or sell copies of the Software, and to permit persons to whom the         *
 * Software is furnished to do so, subject to the following conditions:          *
 *                                                                               *
 * The above copyright notice and this permission notice shall be included in    *
 * all copies or substantial portions of the Software.                           *
 *                                                                               *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     *
 * THE SOFTWARE.                                                                 *
 * *******************************************************************************
 *                                                                               *
 * GTK FM Radio                                                                  *
 * Frequency Edit dialog box handler.                                            *
 *                                                                               *
 *********************************************************************************/

#include <gtk/gtk.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "freqedit.h"
#include "defmain.h"

FreqWindow freqEditor;

uint8_t show_frequency_edit_window(GtkWidget *parent, double *freq)
{
    char freqStr[12];
    char *newFreq, *dummy;
    GtkBuilder *builder;
    gint result;

    // Load frequency editor dialog box from glade file.
    builder = gtk_builder_new_from_file("glade/gtkfmtuner.glade");

    freqEditor.window = GTK_WIDGET(gtk_builder_get_object(builder, "freq-edit"));
    freqEditor.freqEntry = GTK_ENTRY(gtk_builder_get_object(builder, "txtFreqEditInput"));
    freqEditor.defaultButton = GTK_BUTTON(gtk_builder_get_object(builder, "btnFreqEditOK"));

    // Setup events and release builder.
    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);
    
    // Set specified frequency to the edit field.
#ifdef DEBUG_LOGS
    g_message("Setup default frequency value = %lf", *freq); 
#endif

    sprintf(freqStr, "%.2lf", (*freq));
    gtk_entry_set_text(freqEditor.freqEntry, freqStr);

    // Show frequency editor dialog box.
    gtk_widget_set_can_default(GTK_WIDGET(freqEditor.defaultButton), TRUE);
    gtk_widget_grab_default(GTK_WIDGET(freqEditor.defaultButton));

    gtk_window_set_transient_for(GTK_WINDOW(freqEditor.window), GTK_WINDOW(parent));
    gtk_window_set_title(GTK_WINDOW(freqEditor.window), "Change Frequency");

    result = gtk_dialog_run(GTK_DIALOG(freqEditor.window));

    // Update frequency based on user action.
    if(result == GTK_RESPONSE_OK)
    {        
        newFreq = (char*)gtk_entry_get_text(freqEditor.freqEntry);
        *freq = strtod(newFreq, &dummy);     
#ifdef DEBUG_LOGS
        g_message("Updating new frequency value = %lf", *freq);   
#endif
    }

    // Release frequency editor window.
    gtk_widget_destroy(freqEditor.window);
    freqEditor.window = NULL;

    return (result == GTK_RESPONSE_OK) ? RESULT_SUCCESS : RESULT_FAIL;
}

void on_btnFreqEditOK_clicked()
{
    char *userInput;
    double userDoubleResult;
    char *userStrResult;
    uint8_t validationStatus = RESULT_FAIL;

    userInput = (char*)gtk_entry_get_text(freqEditor.freqEntry);
    if((userInput != NULL) && (strlen(userInput) > 1))
    {
        // Try to convert specified value into double for validations.
        errno = 0;
        userDoubleResult = strtod(userInput, &userStrResult);
        
        // Check for valid double.
        if((userDoubleResult == 0) || ((errno != 0) || (userInput == userStrResult)))
        {
            // Specified string is not double.
            validationStatus = RESULT_FAIL;
        }
        else
        {
            // Perform range check!
            if((userDoubleResult >= LOW_FREQ) && (userDoubleResult <= HIGH_FREQ))
            {
                // Specified value is a valid number.
                validationStatus = RESULT_SUCCESS;
            }
        }
    }

    if(validationStatus == RESULT_SUCCESS)
    {
        // All validations are successful.
        gtk_dialog_response(GTK_DIALOG(freqEditor.window), GTK_RESPONSE_OK);
    }
    else
    {
        // Input validation(s) fail!
        GtkWidget *dlgError;
        dlgError = gtk_message_dialog_new(GTK_WINDOW(freqEditor.window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Invalid or unsupported frequency!");
        gtk_window_set_title(GTK_WINDOW(dlgError), APPLICATION_TITLE);
        gtk_dialog_run(GTK_DIALOG(dlgError));
        gtk_widget_destroy(dlgError);
        dlgError = NULL;
    }
}

void on_btnFreqEditCancel_clicked()
{
    gtk_dialog_response(GTK_DIALOG(freqEditor.window), GTK_RESPONSE_CANCEL);
}

gboolean on_txtFreqEditInput_key_press_event(GtkWidget* widget, GdkEventKey* event)
{
    if(event->keyval == GDK_KEY_Return)
    {
        on_btnFreqEditOK_clicked();
        return TRUE;
    }

    return FALSE;
}