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
 * Tuner interface definitions and related data structures.                      *
 *                                                                               *
 *********************************************************************************/

#ifndef _GTK_FM_TUNER_BASETUNER_HEADER_
#define _GTK_FM_TUNER_BASETUNER_HEADER_

#include <gtk/gtk.h>
#include <stdint.h>

typedef enum 
{
    SCAN_DOWN,
    SCAN_UP
} ScanDirection;

typedef enum 
{
    VOLUME_DOWN,
    VOLUME_UP
} VolumeDirection;

typedef enum 
{
    MPXS_STEREO,
    MPXS_MONO,
    MPXS_UNKNOWN
} StereoMPXState;

// Core tuner functions.

// Initialize the FM tuner.
typedef uint8_t (*init_tuner)(void);
// Shutdown FM tuner.
typedef uint8_t (*shutdown_tuner)(void);

// Set tuner frequency (freq * 100).
typedef uint8_t (*set_tuner_frequency)(double frequency);
// Get tuner frequency (freq * 100).
typedef double (*get_tuner_frequency)(void);
// Scan for new channel. (SCAN_DIRECTION_UP/SCAN_DIRECTION_DOWN)
typedef uint8_t (*tuner_scan_channel)(ScanDirection direction);

// Set tuner volume control.
typedef uint8_t (*tuner_set_volume)(uint16_t level);
// Get tuner volume control.
typedef uint16_t (*tuner_get_volume)(void);
// Change tuner volume in to specified direction.
typedef uint16_t (*tuner_change_volume)(VolumeDirection direction);

// Optional tuner functions.

// Current SNR value from the tuner.
typedef int16_t (*get_tuner_snr)(void);
// Stereo/Mono status of the current channel.
typedef StereoMPXState (*get_tuner_stereo_mpx_status)(void);
// Current RSSI (Received Signal Strength Indicator) value from the tuner.
typedef int16_t (*get_tuner_rssi)(void);

typedef struct Tuner 
{
    init_tuner init;
    shutdown_tuner shutdown;

    set_tuner_frequency set_frequency;
    get_tuner_frequency get_frequency;
    tuner_scan_channel scan_channel;

    tuner_set_volume set_volume;
    tuner_get_volume get_volume;
    tuner_change_volume change_volume;

    get_tuner_snr snr;
    get_tuner_stereo_mpx_status stereo_mpx;
    get_tuner_rssi rssi;

    char *rdsData;
} Tuner;

#endif /* _GTK_FM_TUNER_BASETUNER_HEADER_ */