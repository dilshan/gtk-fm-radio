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
 * Driver for QN8035 FM tuner.                                                   *
 *                                                                               *
 *********************************************************************************/

#ifndef _GTK_FM_TUNER_QN8035_HEADER_
#define _GTK_FM_TUNER_QN8035_HEADER_

#include <stdint.h>
#include "tuner.h"
#include "defmain.h"

// I2C address of the QN8035 tuner.
#define QN8035_ADDRESS  0x10

// Chip ID related to QN8035 tuner.
#define QN8035_ID       0x84

#define REG_SYSTEM1     0x00    // Device modes.
#define REG_CCA         0x01    // CCA parameters.
#define REG_SNR         0x02    // Estimate RF input CNR value.
#define REG_RSSISIG     0x03    // In-band signal RSSI value.
#define REG_STATUS1     0x04    // System status.
#define REG_CID1        0x05    // Device ID numbers.
#define REG_CID2        0x06    // Device ID numbers.
#define REG_CH          0x07    // Lower 8 bits of 10-bit channel index.
#define REG_CH_START    0x08    // Lower 8 bits of 10-bit channel scan start channel index.
#define REG_CH_STOP     0x09    // Lower 8 bits of 10-bit channel scan stop channel index.
#define REG_CH_STEP     0x0A    // Channel scan frequency step. Highest 2 bits of channel indexes.
#define REG_RDSD0       0x0B    // RDS data byte 0.
#define REG_RDSD1       0x0C    // RDS data byte 1.
#define REG_RDSD2       0x0D    // RDS data byte 2.
#define REG_RDSD3       0x0E    // RDS data byte 3.
#define REG_RDSD4       0x0F    // RDS data byte 4.
#define REG_RDSD5       0x10    // RDS data byte 5.
#define REG_RDSD6       0x11    // RDS data byte 6.
#define REG_RDSD7       0x12    // RDS data byte 7.
#define REG_STATUS2     0x13    // RDS status indicators.
#define REG_VOL_CTL     0x14    // Audio controls.
#define REG_XTAL_DIV0   0x15    // Frequency select of reference clock source.
#define REG_XTAL_DIV1   0x16    // Frequency select of reference clock source.
#define REG_XTAL_DIV2   0x17    // Frequency select of reference clock source.
#define REG_INT_CTRL    0x18    // RDS control.

// Undocumented registers (based on https://github.com/ukrtrip/QN8035-qn8035-FM-chip-library/).
#define REG_CCA_SNR_TH_1    0x39
#define REG_CCA_SNR_TH_2    0x3A
#define REG_NCCFIR3         0x40

// Scanning steps for REG_CH_STEP.
#define REG_CH_STEP_50KHZ   0x00
#define REG_CH_STEP_100KHZ  0x40
#define REG_CH_STEP_200KHZ  0x80

// Bit definitions of REG_SYSTEM1.
#define REG_SYSTEM1_CCA_CH_DIS      0x01    // CH (channel index) selection method. 0 - CH is determined by internal CCA; 1 - CH is determined by the content in CH[9:0].
#define REG_SYSTEM1_CHSC            0x02    // Channel Scan mode enable. 0 - Normal operation; 1 - Channel Scan mode operation.
#define REG_SYSTEM1_FORCE_MO        0x04    // Force receiver in MONO mode, 0 - Auto, 1 - Forced in MONO mode.
#define REG_SYSTEM1_RDSEN           0x08    // RDS enable.
#define REG_SYSTEM1_RXREQ           0x10    // Receiving request. 0 - Non RX mode, 1 - Enter receive mode.
#define REG_SYSTEM1_STNBY           0x20    // Request immediately to enter Standby mode.
#define REG_SYSTEM1_RECAL           0x40    // Reset the state to initial states and recalibrate all blocks.
#define REG_SYSTEM1_SWRST           0x80    // Reset all registers to default values.

// Bit definitions of REG_STATUS1.
#define REG_STATUS1_ST_MO_RX        0x01    // Stereo receiving status.
#define REG_STATUS1_RXAGC           0x02    // AGC error status.
#define REG_STATUS1_RXAGCSET        0x04    // AGC settling status.
#define REG_STATUS1_RXCCA_FAIL      0x08    // RXCCA Status Flag.
#define REG_STATUS1_FSM             0x70    // FSM state indicator.

// Volume control settings
#define REG_VOL_CTL_MAX_ANALOG_GAIN 0x07
#define REG_VOL_CTL_MIN_ANALOG_GAIN 0x00

// Default auto scan (CCA) level.
#define CCA_LEVEL   0x10

// RDS group definitions.
#define RDS_GROUP       0xF800
#define RDS_GROUP_A0    0x0000
#define RDS_GROUP_B0    0x0080

void qn8035_scan_frequency_down();
void qn8035_scan_frequency_up();

void qn8035_init_rds_decoder();
void *qn8035_rds_process_thread(void *threadStruct);

typedef struct RDSProcessContext
{
    int *ioHandle;
    RDSProcessState state;
    char **rdsBuffer;
} RDSProcessContext;

#endif /* _GTK_FM_TUNER_QN8035_HEADER_ */