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
 * Driver interface for QN8035 FM tuner.                                         *
 *                                                                               *
 *********************************************************************************/

#ifndef _GTK_FM_TUNER_QN8035_INTERFACE_HEADER_
#define _GTK_FM_TUNER_QN8035_INTERFACE_HEADER_

#include <stdint.h>
#include "tuner.h"

// Size of the RDS buffer.
#define RDS_INFO_MAX_SIZE 16

uint8_t qn8035_tuner_init(void);
uint8_t qn8035_tuner_shutdown(void);

uint8_t qn8035_tuner_set_frequency(double frequency);
double qn8035_tuner_get_frequency(void);
uint8_t qn8035_tuner_scan(ScanDirection direction);

uint8_t qn8035_set_volume(uint16_t level);
uint16_t qn8035_get_volume(void);
uint16_t qn8035_change_volume(VolumeDirection direction);

StereoMPXState qn8035_get_stereo_mpx_status(void);
int16_t qn8035_get_snr(void);
int16_t qn8035_get_rssi(void);

extern char *qn8035RDSInfo;

#endif /* _GTK_FM_TUNER_QN8035_INTERFACE_HEADER_ */