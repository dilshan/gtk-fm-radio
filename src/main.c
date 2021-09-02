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
 * Main source file and application entry point.                                 *
 *                                                                               *
 *********************************************************************************/

#include <gtk/gtk.h>
#include <glib.h>
#include <pthread.h>

#include "main.h"
#include "freqedit.h"
#include "defmain.h"
#include "defconfig.h"

#include "tuner.h"

#if TUNER == TUNER_QN8035
#include "qn8035intf.h"
#endif

// Core global data structures.
MainWindow mainWindow;
GdkPixbuf *appLogo;
static Tuner fmtuner;

// Resources for station scanner thread.
static ScanContext channelScanParam;
pthread_t scanThread;

double appFrequency;

int main(int argc, char *argv[])
{
    GtkBuilder *builder;     
    StatusControls indControls;

    appFrequency = TUNER_MIN_FREQUENCY;

#if TUNER == TUNER_QN8035
    // Assign QN8035 functions into the tuner.
    fmtuner.init = qn8035_tuner_init;
    fmtuner.shutdown = qn8035_tuner_shutdown;

    fmtuner.set_frequency = qn8035_tuner_set_frequency;
    fmtuner.get_frequency = qn8035_tuner_get_frequency;
    fmtuner.scan_channel = qn8035_tuner_scan;

    fmtuner.set_volume = qn8035_set_volume;
    fmtuner.get_volume = qn8035_get_volume;
    fmtuner.change_volume = qn8035_change_volume;

    fmtuner.stereo_mpx = qn8035_get_stereo_mpx_status;
    fmtuner.snr = qn8035_get_snr;
    fmtuner.rssi = qn8035_get_rssi;
#endif    

    // Initialize GTK and loading main window from glade file.
    gtk_init(&argc, &argv);
    builder = gtk_builder_new_from_file("glade/gtkfmtuner.glade");

    mainWindow.window = GTK_WIDGET(gtk_builder_get_object(builder, "gtk-fm-tuner-app"));
    mainWindow.frequencyDisplay = GTK_LABEL(gtk_builder_get_object(builder, "lblFreq"));
    mainWindow.stereoStatus = GTK_LABEL(gtk_builder_get_object(builder, "lblStereo"));
    mainWindow.SNR = GTK_LABEL(gtk_builder_get_object(builder, "lblSNR"));
    mainWindow.RSSI = GTK_LABEL(gtk_builder_get_object(builder, "lblRSSI"));
    mainWindow.RDSText = GTK_LABEL(gtk_builder_get_object(builder, "lblRDS"));

    // Setup events and release builder.
    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    // Initialize the FM tuner.
#ifdef DEBUG_LOGS    
    g_message("Initializing FM tuner..."); 
#endif   

    if(fmtuner.init() == RESULT_FAIL)
    {
        // Tuner initialization fail.
        GtkWidget *dlgError;
        dlgError = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Unable to initialize the FM tuner");
        gtk_window_set_title(GTK_WINDOW(dlgError), APPLICATION_TITLE);
        gtk_dialog_run(GTK_DIALOG(dlgError));
        gtk_widget_destroy(dlgError);
        dlgError = NULL;

        // Close the application.
        return 0;
    }

    // Update indicator controls structure
    indControls.frequencyDisplay = mainWindow.frequencyDisplay;
    indControls.stereoStatus = mainWindow.stereoStatus;
    indControls.SNR = mainWindow.SNR;
    indControls.RSSI = mainWindow.RSSI;
    indControls.RDSText = mainWindow.RDSText;

    // Clear text values in unsupported fields.
    if(fmtuner.stereo_mpx == NULL)
    {
        gtk_label_set_text(indControls.stereoStatus, "");
    }

    if(fmtuner.snr == NULL)
    {
        gtk_label_set_text(indControls.SNR, "");
    }

    if(fmtuner.rssi == NULL)
    {
        gtk_label_set_text(indControls.RSSI, "");
    }
    
    // Display main application window.
    appLogo = gdk_pixbuf_new_from_resource("/com/jayakody2000lk/gtkfmtunericon/icon.png", NULL);
    gtk_window_set_icon (GTK_WINDOW(mainWindow.window), appLogo);
    gtk_window_set_title(GTK_WINDOW(mainWindow.window), APPLICATION_TITLE);
    gtk_widget_show(mainWindow.window); 

    // Assign RDS buffer into the tuner.
#if TUNER == TUNER_QN8035
    fmtuner.rdsData = qn8035RDSInfo;
#endif

    // Get tuner information to display on application window.
    update_tuner_information(&indControls);
    g_timeout_add_full(G_PRIORITY_DEFAULT, DISPLAY_INFO_UPDATE_RATE, (GSourceFunc)on_display_refresh_handler, &indControls, NULL);

    // Create station scan worker.
    create_scan_worker();
    
    gtk_main();
    return 0;
}

void update_tuner_information(StatusControls *indicatorControls)
{
    char currentFreq[15];
    char infoBuffer[25];
    double tempFreq;
    uint8_t mpxState;
    int16_t snr, rssi;

    // Update current frequency.
    tempFreq = fmtuner.get_frequency();
    if(tempFreq > 0)
    {
        // Display only the valid frequency readings from the tuner.
        sprintf(currentFreq, "%.2lf MHz", tempFreq);
        gtk_label_set_text(indicatorControls->frequencyDisplay, currentFreq);
    }

    // Update current FM stereo multiplexing status.
    if(fmtuner.stereo_mpx != NULL)
    {
        mpxState = fmtuner.stereo_mpx();
        if(mpxState != MPXS_UNKNOWN)
        {
            gtk_label_set_text(indicatorControls->stereoStatus, ((mpxState == MPXS_MONO) ? "MONO" : "STEREO"));
        }
    }

    // Update current SNR reading from the tuner.
    if(fmtuner.snr != NULL)
    {
        snr = fmtuner.snr();
        if(snr >= 0)
        {
            sprintf(infoBuffer, "SNR: %d", snr);
            gtk_label_set_text(indicatorControls->SNR, infoBuffer);
        }
    }

    // Update current received signal strength indicator value.
    if(fmtuner.rssi != NULL)
    {
        rssi = fmtuner.rssi();
        if(rssi >= 0)
        {
            sprintf(infoBuffer, "RSSI: %d", rssi);
            gtk_label_set_text(indicatorControls->RSSI, infoBuffer);
        }
    }

    if(fmtuner.rdsData != NULL)
    {
        gtk_label_set_text(indicatorControls->RDSText, fmtuner.rdsData);
    }
}

// Raise when window is closed.
void on_window_main_destroy()
{
    // Shutdown station scanner thread.
    channelScanParam.state = ST_END;

    // Shutdown FM tuner.
    fmtuner.shutdown();

    // Terminate application.
    gtk_main_quit();
}

// Click event handler for minimum frequency button.
void on_btnMinFreq_clicked()
{
    fmtuner.set_frequency(LOW_FREQ);
}

// Click event handler for scan down button.
void on_btnScanDown_clicked()
{
    channelScanParam.scanDirection = SCAN_DOWN;
    channelScanParam.state = ST_START;
}

// Click event handler to frequency edit button.
void on_btnEditFreq_clicked()
{
    uint8_t tryCount = 0;
    
    // Get current frequency from the tuner.
    do
    {
        appFrequency = fmtuner.get_frequency();
        gtk_main_iteration();

    } while (((++tryCount) < 10) && (appFrequency < 0) && gtk_events_pending());
    
    // Check for valid frequency range.
    if((appFrequency < LOW_FREQ) || (appFrequency > HIGH_FREQ))
    {
        // Invalid frequency range. Reset app frequency to lower frequency limit.
        appFrequency = LOW_FREQ;
    }
    
    if(show_frequency_edit_window(mainWindow.window, &appFrequency) == RESULT_SUCCESS)
    {
        fmtuner.set_frequency(appFrequency);
    }
}

// Click event handler for scan up button.
void on_btnScanUp_clicked()
{
    channelScanParam.scanDirection = SCAN_UP;
    channelScanParam.state = ST_START;
}

// Click event handler for maximum frequency button. 
void on_btnMaxFreq_clicked()
{
    fmtuner.set_frequency(HIGH_FREQ);
}

// Click event handler for volume up button. 
void on_btnVolUp_clicked()
{
    fmtuner.change_volume(VOLUME_UP);
}

// Click event handler for volume down button. 
void on_btnVolDown_clicked()
{
    fmtuner.change_volume(VOLUME_DOWN);
}

gboolean on_display_refresh_handler(StatusControls *indicatorControls)
{
    update_tuner_information(indicatorControls);
    return TRUE;
}

void create_scan_worker()
{
    // Create scanner thread and switch it to IDLE state.
    channelScanParam.tunerRef = &fmtuner;
    channelScanParam.scanDirection = SCAN_DOWN;
    channelScanParam.state = ST_IDLE;

    // Station scanner thread.
    pthread_create(&scanThread, NULL, tuner_channel_scan_thread, (void*)(&channelScanParam));
}

void *tuner_channel_scan_thread(void *threadStruct)
{
    ScanContext *channelScanParam = (ScanContext*)threadStruct;

    // Sleep time structure to keep CPU happy.
    struct timespec threadSleeper;
    threadSleeper.tv_sec = 0;
    threadSleeper.tv_nsec = 7500000;

    // Thread service loop.
    while (channelScanParam->state != ST_END)
    {
        if(channelScanParam->state == ST_START)
        {
            // Start new scan job!
            channelScanParam->state = ST_BUSY;
            channelScanParam->tunerRef->scan_channel(channelScanParam->scanDirection); 
            channelScanParam->state = ST_IDLE;
        }

        // Allow CPU to release from this task!
        nanosleep(&threadSleeper, NULL);
    }
    
    // End of scanner thread.
    return NULL;
}

void on_mnuClose_activate()
{
    gtk_window_close(GTK_WINDOW(mainWindow.window));
}

void on_mnuAbout_activate()
{
    const gchar *copyright = "Copyright \xc2\xa9 2021 Dilshan Jayakody";

    const gchar *authors[] = {
        "Dilshan R Jayakody <jayakody2000lk@gmail.com>",
        NULL
    };

    const gchar *contributors[] = {
        "Radio icon by Icons8 <https://icons8.com>",
        NULL
    };

    gtk_show_about_dialog(GTK_WINDOW(mainWindow.window), 
    "name", APPLICATION_TITLE,
    "program-name", APPLICATION_TITLE,
    "logo", appLogo,
    "version", "1.0.0",
    "copyright", copyright,
    "authors", authors,
    "artists", contributors,
    "website", "https://github.com/dilshan",
    "license-type", GTK_LICENSE_MIT_X11,
    "title", "About",
    "screen", gtk_widget_get_screen(mainWindow.window),
    NULL);
}