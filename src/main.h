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

#ifndef _GTK_FM_TUNER_MAIN_HEADER_
#define _GTK_FM_TUNER_MAIN_HEADER_

#include "defmain.h"

// Refresh rate of frequency and other channel/tuner information in ms.
#define DISPLAY_INFO_UPDATE_RATE 500

void on_window_main_destroy(void);
void on_btnMinFreq_clicked(void);
void on_btnScanDown_clicked(void);
void on_btnEditFreq_clicked(void);
void on_btnScanUp_clicked(void);
void on_btnMaxFreq_clicked(void);
void on_btnVolUp_clicked(void);
void on_btnVolDown_clicked(void);

void update_tuner_information(StatusControls *indicatorControls);
gboolean on_display_refresh_handler(StatusControls *indicatorControls);

void *tuner_channel_scan_thread(void *threadStruct);
void create_scan_worker(void);

#endif /* _GTK_FM_TUNER_MAIN_HEADER_ */