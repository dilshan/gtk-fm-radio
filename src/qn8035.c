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

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "defconfig.h"
#include "defmain.h"
#include "qn8035.h"
#include "qn8035intf.h"

// https://github.com/WiringPi/WiringPi
#include <wiringPiI2C.h>

#define SET_REG(r,v)    wiringPiI2CWriteReg8(fd,r,v)
#define GET_REG(r)      wiringPiI2CReadReg8(fd,r)

#define FREQ_TO_WORD(f) ((uint16_t)((f - 60) / 0.05))
#define WORD_TO_FREQ(w) (((double)w * 0.05) + 60)

static GMutex tunerMutex;

int fd;
uint16_t currentFreq;
uint8_t volumeLevel;

pthread_t rdsDecoderThread;
char *qn8035RDSInfo;
RDSProcessContext rdsContext;

uint8_t qn8035_tuner_init()
{
#ifdef DEBUG_LOGS    
    g_message("Init QN8035 tuner");
#endif

    fd = wiringPiI2CSetup(QN8035_ADDRESS);
    if(fd < 0)
    {
        // I2C setup error, may be I2C connection to QN8035 is faulty?
#ifdef DEBUG_LOGS        
        g_message("Unable to initialize the QN8035 receiver: %s", strerror(errno));
#endif
        return RESULT_FAIL;
    }

    // Check for valid QN8035 tuner.
    if(GET_REG(REG_CID2) != QN8035_ID)
    {
        // Invalid chip ID detected!
#ifdef DEBUG_LOGS        
        g_message("Invalid/unsupported QN8035 chip ID");
#endif
        return RESULT_FAIL;
    }

#ifdef DEBUG_LOGS
    g_message("Connected to QN8035 and initializing QN8035 tuner...");
#endif

    // Reset all registers of QN8035 tuner.
    SET_REG(REG_SYSTEM1, REG_SYSTEM1_SWRST);
    usleep(1500000);

    // Set tuner frequency and volume to the defaults.
    qn8035_tuner_set_frequency(TUNER_MIN_FREQUENCY);
    qn8035_set_volume(REG_VOL_CTL_MAX_ANALOG_GAIN);

    // Start RDS decoder thread.
    qn8035_init_rds_decoder();

    return RESULT_SUCCESS;
}

uint8_t qn8035_tuner_shutdown()
{
#ifdef DEBUG_LOGS    
    g_message("Shutdown QN8035 tuner");
#endif    

    // Stop RDS capture and process thread.
    rdsContext.state = RD_END;

    g_mutex_lock(&tunerMutex);

    // Reset and recalibrate the receiver.
    SET_REG(REG_SYSTEM1, REG_SYSTEM1_RECAL | REG_SYSTEM1_SWRST);
    usleep(100);

    // Enter tuner into the standby mode.
    SET_REG(REG_SYSTEM1, REG_SYSTEM1_STNBY);

    g_mutex_unlock(&tunerMutex);

    // Release RDS output buffer.
    if(qn8035RDSInfo != NULL)
    {
        free(qn8035RDSInfo);
        qn8035RDSInfo = NULL;
    }

    return RESULT_SUCCESS;
}

uint8_t qn8035_tuner_set_frequency(double frequency)
{
    uint16_t tuneFreq;

    tuneFreq = FREQ_TO_WORD(frequency);
    rdsContext.state = RD_IDLE;

#ifdef DEBUG_LOGS    
    g_message("Set QN8035 tuner frequency = %d", tuneFreq);    
#endif     

    g_mutex_lock(&tunerMutex);

    SET_REG(REG_CH, (tuneFreq & 0xFF));                // Lo
    SET_REG(REG_CH_STEP, ((tuneFreq >> 8) & 0x03));    // Hi

    usleep(100);
    SET_REG(REG_SYSTEM1, REG_SYSTEM1_CCA_CH_DIS | REG_SYSTEM1_RXREQ | REG_SYSTEM1_RDSEN);

    g_mutex_unlock(&tunerMutex);

    currentFreq = tuneFreq;
    rdsContext.state = RD_CLEAR;

    return RESULT_SUCCESS;
}

double qn8035_tuner_get_frequency()
{
    if(g_mutex_trylock(&tunerMutex))
    {
        double result = WORD_TO_FREQ((uint16_t)(GET_REG(REG_CH) | ((GET_REG(REG_CH_STEP) & 0x03) << 8)));

        g_mutex_unlock(&tunerMutex);

        return result;
    }
    else
    {
        // QN8035 tuner is in use by another thread!
        return -1;
    }
}

uint8_t qn8035_tuner_scan(ScanDirection direction)
{
    uint8_t timeout, isFound, freqFix;
    uint16_t newFreq;
    
#ifdef DEBUG_LOGS      
    g_message("Scan QN8035 tuner in direction = %d", direction);
#endif    

    rdsContext.state = RD_IDLE;

    g_mutex_lock(&tunerMutex);

    if((direction == SCAN_UP) && (currentFreq < FREQ_TO_WORD(HIGH_FREQ)))
    {
        qn8035_scan_frequency_up();
    }
    else if((direction == SCAN_DOWN) && (currentFreq > FREQ_TO_WORD(LOW_FREQ)))
    {
        qn8035_scan_frequency_down();
    }

    // Wait for end of scanning.
    timeout = 25;
    isFound = 0;

    do
    {
        // Check for end of auto scan operation.
        if((GET_REG(REG_SYSTEM1) & REG_SYSTEM1_CHSC) == 0)
        {
#ifdef DEBUG_LOGS             
            g_message("Scan completed");
#endif
            isFound = 1;
            break;
        }
            
        timeout--;
        usleep(5000);        
    } 
    while (timeout != 0);

    if(isFound)
    {
        // If scan completes, get the new frequency from the QN8035 tuner.
        newFreq = GET_REG(REG_CH) | ((GET_REG(REG_CH_STEP) & 0x03) << 8);  
        freqFix = 0;

#ifdef DEBUG_LOGS         
        g_message("Scanner stopped in frequency = %d", newFreq);        
#endif

        // Fix: In some cases we notice receiver jump to 85MHz/111MHz if scanner goes beyond 98.25MHz or 98.4MHz.
        if((newFreq < FREQ_TO_WORD(LOW_FREQ)) && (currentFreq > FREQ_TO_WORD(LOW_FREQ)) && (currentFreq < FREQ_TO_WORD(98.3)))
        {
            printf("CCC\n");
            newFreq = FREQ_TO_WORD(98.4);
            freqFix = 1;
        }
        else if((newFreq > FREQ_TO_WORD(HIGH_FREQ)) && (currentFreq > FREQ_TO_WORD(98.3)) && (currentFreq < FREQ_TO_WORD(HIGH_FREQ)))
        {
            printf("DDD\n");
            newFreq = FREQ_TO_WORD(98.2);
            freqFix = 1;
        }

        if(freqFix)
        {
            // Scanner reset occure, set frequency above 98.25MHz!
            SET_REG(REG_CH, (newFreq & 0xFF));                // Lo
            SET_REG(REG_CH_STEP, ((newFreq >> 8) & 0x03));    // Hi

            usleep(100);
            SET_REG(REG_SYSTEM1, REG_SYSTEM1_CCA_CH_DIS | REG_SYSTEM1_RXREQ | REG_SYSTEM1_RDSEN);
        }

        // Verify limits and set new frequency as a default frequency.
        if((newFreq < FREQ_TO_WORD(HIGH_FREQ)) && (newFreq > FREQ_TO_WORD(LOW_FREQ)))
        {
            currentFreq = newFreq;
        }
    }

    g_mutex_unlock(&tunerMutex);

    rdsContext.state = RD_CLEAR;

    return isFound ? RESULT_SUCCESS : RESULT_FAIL;
}

uint8_t qn8035_set_volume(uint16_t level)
{
    uint8_t volReg;
    
#ifdef DEBUG_LOGS     
    g_message("Set QN8035 volume = %d", level);
#endif

    // Check for valid volume level.
    if((level >= REG_VOL_CTL_MIN_ANALOG_GAIN) && (level <= REG_VOL_CTL_MAX_ANALOG_GAIN))
    {
        g_mutex_lock(&tunerMutex);

        volReg = (GET_REG(REG_VOL_CTL) & 0xF8) | level;
        SET_REG(REG_VOL_CTL, volReg);

        g_mutex_unlock(&tunerMutex);

        volumeLevel = level;
        return RESULT_SUCCESS;
    }
    
    // Specified volume level is invalid or unsupported.
    return RESULT_FAIL;
}

uint16_t qn8035_get_volume()
{
    if(g_mutex_trylock(&tunerMutex))
    {
        volumeLevel = GET_REG(REG_VOL_CTL) & 0x07;

        g_mutex_unlock(&tunerMutex);
    }

    return volumeLevel;
}

uint16_t qn8035_change_volume(VolumeDirection direction)
{
#ifdef DEBUG_LOGS     
    g_message("Change QN8035 volume in direction = %d", direction);
#endif

    if(direction == VOLUME_UP)
    {
        // Increase analog volume level.
        volumeLevel += (volumeLevel >= REG_VOL_CTL_MAX_ANALOG_GAIN) ? 0 : 1;
    }
    else
    {
        // Decrease analog volume level.
        volumeLevel -= (volumeLevel == REG_VOL_CTL_MIN_ANALOG_GAIN) ? 0 : 1;
    }

    // Apply new volume level into QN8035 tuner.
    qn8035_set_volume(volumeLevel);
    return volumeLevel;
}

StereoMPXState qn8035_get_stereo_mpx_status()
{
    StereoMPXState mpxStatus = MPXS_UNKNOWN;

    if(g_mutex_trylock(&tunerMutex))
    {        
        mpxStatus = ((GET_REG(REG_STATUS1) & REG_STATUS1_ST_MO_RX) ? MPXS_MONO : MPXS_STEREO);

        g_mutex_unlock(&tunerMutex);
    }

    return mpxStatus;
}

int16_t qn8035_get_snr()
{
    int16_t snrValue = -1;

    if(g_mutex_trylock(&tunerMutex))
    {
        snrValue = (int16_t)GET_REG(REG_SNR);

        g_mutex_unlock(&tunerMutex);
    }

    return snrValue;
}

int16_t qn8035_get_rssi()
{
    int16_t rssiValue = -1;

    if(g_mutex_trylock(&tunerMutex))
    {
        rssiValue = (int16_t)GET_REG(REG_RSSISIG);

        g_mutex_unlock(&tunerMutex);
    }

    return rssiValue;
}

void qn8035_scan_frequency_down()
{
    uint16_t freqEnd;
    
    SET_REG(REG_CCA_SNR_TH_1, 0x00);
    SET_REG(REG_CCA_SNR_TH_2, 0x05);
    SET_REG(REG_NCCFIR3, 0x05);
    
    freqEnd = FREQ_TO_WORD(LOW_FREQ);

    // Set start frequency with -200kHz offset with current frequency.
    SET_REG(REG_CH_START, (currentFreq - 4) & 0xFF);

    // Set stop frequency.
    SET_REG(REG_CH_STOP, freqEnd & 0xFF);
    
    SET_REG(REG_CH_STEP, (REG_CH_STEP_200KHZ | ((currentFreq >> 8) & 0x03) | ((currentFreq >> 6) & 0x0C) | ((freqEnd >> 4) & 0x30)));    

    SET_REG(REG_CCA, CCA_LEVEL);

    // Initiate scan down.
    SET_REG(REG_SYSTEM1, REG_SYSTEM1_RXREQ | REG_SYSTEM1_CHSC | REG_SYSTEM1_RDSEN);
}

void qn8035_scan_frequency_up()
{
    uint16_t freqEnd;    
    
    SET_REG(REG_CCA_SNR_TH_1, 0x00);
    SET_REG(REG_CCA_SNR_TH_2, 0x05);
    SET_REG(REG_NCCFIR3, 0x05);
    
    freqEnd = FREQ_TO_WORD(HIGH_FREQ);

    // Set start frequency with +200kHz offset with current frequency.
    SET_REG(REG_CH_START, (currentFreq + 4) & 0xFF);

    // Set stop frequency.
    SET_REG(REG_CH_STOP, freqEnd & 0xFF);
    
    SET_REG(REG_CH_STEP, (REG_CH_STEP_200KHZ | ((currentFreq >> 8) & 0x03) | ((currentFreq >> 6) & 0x0C) | ((freqEnd >> 4) & 0x30)));    

    SET_REG(REG_CCA, CCA_LEVEL);

    // Initiate scan up.
    SET_REG(REG_SYSTEM1, REG_SYSTEM1_RXREQ | REG_SYSTEM1_CHSC | REG_SYSTEM1_RDSEN);
}

void qn8035_init_rds_decoder()
{    
    // Create and reset RDS data buffer.
    qn8035RDSInfo = (char*)malloc(RDS_INFO_MAX_SIZE);

    memset(qn8035RDSInfo, ' ', (RDS_INFO_MAX_SIZE - 1));
    qn8035RDSInfo[RDS_INFO_MAX_SIZE - 1] = 0x00;

    // Create RDS capture context on CAPTURE state.
    rdsContext.ioHandle = &fd;
    rdsContext.state = RD_IDLE;
    rdsContext.rdsBuffer = &qn8035RDSInfo;

    // Create RDS capture and process thread.
    pthread_create(&rdsDecoderThread, NULL, qn8035_rds_process_thread, (void*)(&rdsContext));
}

void *qn8035_rds_process_thread(void *threadStruct)
{
    RDSProcessContext *rdsContext = (RDSProcessContext *)threadStruct;
    char *rdsBufferTemp = *(rdsContext->rdsBuffer);
    
    uint16_t rdsB, rdsD;
    uint16_t groupB;
    char char1, char2;
    uint8_t offset;
    char rdsCaptureBufferTemp[RDS_INFO_MAX_SIZE];

    // Sleep time structure to keep CPU happy.
    struct timespec threadSleeper;
    threadSleeper.tv_sec = 0;
    threadSleeper.tv_nsec = 15000000;

    // Thread service loop.
    while(rdsContext->state != RD_END)
    {
        // Allow CPU to release from this task!
        nanosleep(&threadSleeper, NULL);
        
        if(rdsContext->state == RD_CLEAR)
        {
            // Clear RDS data buffers.
            memset(rdsBufferTemp, ' ', (RDS_INFO_MAX_SIZE - 1));
            rdsBufferTemp[RDS_INFO_MAX_SIZE - 1] = 0x00;

            memset(rdsCaptureBufferTemp, ' ', (RDS_INFO_MAX_SIZE - 1));
            rdsCaptureBufferTemp[RDS_INFO_MAX_SIZE - 1] = 0x00;
        
            rdsContext->state = RD_CAPTURE;
        }
        else if(rdsContext->state == RD_CAPTURE)
        {
            // RDS capture and decode.
            if(g_mutex_trylock(&tunerMutex))
            {
                rdsB = GET_REG(REG_RDSD3) | GET_REG(REG_RDSD2) << 8;
                rdsD = GET_REG(REG_RDSD7) | GET_REG(REG_RDSD6) << 8;

                g_mutex_unlock(&tunerMutex);

                groupB = rdsB & RDS_GROUP;
                if((groupB == RDS_GROUP_A0) || (groupB == RDS_GROUP_B0))
                {
                    offset = (rdsB & 0x03) << 1;
                    char1 = (char)(rdsD >> 8);
                    char2 = (char)(rdsD & 0xFF);

                    // Fill extracted characters and buffer offsets into primary and secondary arrays.
                    if(offset < RDS_INFO_MAX_SIZE)
                    {
                        if (rdsCaptureBufferTemp[offset] == char1) 
                        {
                            // 1st character verification is successful.
                            rdsBufferTemp[offset] = char1;                          
                        } 
                        else if(isprint(char1))
                        {
                            rdsCaptureBufferTemp[offset] = char1;
                        }

                        if (rdsCaptureBufferTemp[offset + 1] == char2) 
                        {
                            // 2nd character verification is successful.
                            rdsBufferTemp[offset + 1] = char2;                                                        
                        } 
                        else if(isprint(char2))
                        {
                            rdsCaptureBufferTemp[offset + 1] = char2;
                        }
                    }       
                }
            }
        }
    }

    return NULL;
}