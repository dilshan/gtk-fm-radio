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
 * System wide data structures and constants.                                    *
 *                                                                               *
 *********************************************************************************/

#ifndef _GTK_FM_TUNER_DEFMAIN_HEADER_
#define _GTK_FM_TUNER_DEFMAIN_HEADER_

#include <gtk/gtk.h>
#include "tuner.h"

#define RESULT_SUCCESS  0
#define RESULT_FAIL     1

typedef enum 
{
    ST_IDLE,    // Scanner is in idle state.
    ST_START,   // Start scanning on configured frequency range.
    ST_BUSY,    // Scanner is busy with finding the next station.
    ST_END      // Terminate scan thread.
} ScanThreadState;

typedef enum
{
    RD_IDLE,    // RDS processing thread is in idle state.
    RD_CAPTURE, // Capture and decode RDS data.
    RD_CLEAR,   // Clear RDS result and capture buffers.
    RD_END      // Terminate RDS processing thread.
} RDSProcessState;

typedef struct MainWindow
{
    GtkWidget *window;
    GtkLabel *frequencyDisplay;
    GtkLabel *stereoStatus;
    GtkLabel *SNR;
    GtkLabel *RSSI;
    GtkLabel *RDSText;
} MainWindow;

typedef struct FreqWindow 
{
    GtkWidget *window;
    GtkEntry *freqEntry;
    GtkButton *defaultButton;
} FreqWindow;

typedef struct StatusControls
{
    GtkLabel *frequencyDisplay;
    GtkLabel *stereoStatus;
    GtkLabel *SNR;
    GtkLabel *RSSI;
    GtkLabel *RDSText;
} StatusControls;

typedef struct ScanContext
{
    Tuner *tunerRef;
    ScanDirection scanDirection;
    ScanThreadState state;
} ScanContext;

#endif /* _GTK_FM_TUNER_DEFMAIN_HEADER_ */