//
//  Emote.c
//  Emote
//
//  Created by Paweł Gajewski on 3/7/12.
//  Copyright (c) 2012 something.pl. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include "Emote.h"
#include <ftd2xx.h>

#define BUF_SIZE 16

FT_HANDLE ftHandle = NULL;
pthread_t posixThreadID;
pthread_mutex_t historyMutex;

char eeError[255];

int initSuccess = 0;
int monitoring = 0;
int heartRate = 0;  // beats per minute
int referencePulse = 70;
float averagePulse = 0.f;

typedef struct {
    int *buffer;
    int lenght;
    int start;
} Emote_CircularBuffer;

Emote_CircularBuffer historyBuffer;

static void * Emote_monitoringThreadMain(void* data);
static void * Emote_calibratingThreadMain(void* data);
static void Emote_resetCircularBuffer(Emote_CircularBuffer* buff);
static void Emote_setCircularBufferLength(Emote_CircularBuffer* buff, unsigned int lenght);
static void Emote_writeToCircularBuffer(Emote_CircularBuffer* buff, int value);
static int Emote_readFromCircularBuffer(Emote_CircularBuffer* buff, int index);
static float Emote_calculateAveragePulse(void);
static void Emote_printCircularBuffer(Emote_CircularBuffer* buff);

int Emote_init(void)
{
    FT_STATUS   ftStatus;
    int         iport = 0;
    
    sprintf(eeError, "");
    
    ftStatus = FT_Open(iport, &ftHandle);
    if(ftStatus != FT_OK) {
		sprintf(eeError, "Could not connect to the device on port %d", iport);
		return 1;
	}
    
    FT_SetBaudRate(ftHandle, 9600);
    
    unsigned char ucMode = 0x00;
    
    ftStatus = FT_GetBitMode(ftHandle, &ucMode);
	if(ftStatus != FT_OK) {
		printf("Failed to get bit mode\n");
	}
	else {
		printf("ucMode = 0x%X\n", ucMode);	
	}
    
    Emote_resetCircularBuffer(&historyBuffer);
    
    pthread_mutex_init(&historyMutex, NULL);
    
    initSuccess = 1;
    
    Emote_printCircularBuffer(&historyBuffer);
    
    return 0;
}

void Emote_shutdown(void)
{
    pthread_join(posixThreadID, NULL);
    pthread_mutex_destroy(&historyMutex);
    
    if(ftHandle != NULL) {
		FT_Close(ftHandle);
		ftHandle = NULL;
	}
    
    Emote_resetCircularBuffer(&historyBuffer);
}

void Emote_printErrorMessage(void)
{
    printf("Emote Error: %s\n", eeError);
}

void Emote_startMonitoring(void)
{
    if (initSuccess == 0) {
        sprintf(eeError, "Can't start monitoring after failed initialization.");
        Emote_printErrorMessage();
        return;
    }
    
    // Create the thread using POSIX routines.
    pthread_attr_t  attr;
    int             returnVal;
    
    returnVal = pthread_attr_init(&attr);
    assert(!returnVal);
    returnVal = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    assert(!returnVal);
    
    monitoring = 1;
    
    int threadError = pthread_create(&posixThreadID, &attr, &Emote_monitoringThreadMain, NULL);
    
    returnVal = pthread_attr_destroy(&attr);
    assert(!returnVal);
    if (threadError != 0)
    {
        // Report an error.
    }
}

void Emote_stopMonitoring(void)
{
    monitoring = 0;
}

void * Emote_monitoringThreadMain(void* data)
{
    char cBufRead[BUF_SIZE];
	DWORD dwBytesRead;
    DWORD dwBytesInQueue = 0;
    
    memset(cBufRead, 0, BUF_SIZE);
    
    FT_SetTimeouts(ftHandle, 1000, 1000);
    
//    double delta = 0.0f;
    
//    clock_t start = clock();
    
    while(monitoring) {
//        start = clock();
        
        FT_Write(ftHandle, "G1\r", 3, &dwBytesInQueue);
		FT_Read(ftHandle, cBufRead, BUF_SIZE, &dwBytesRead);
		
//        if(dwBytesRead != BUF_SIZE)
//			printf("Timeout %d\n", i++);
//		else
//			printf("Read %d\n", dwBytesRead);
        
        int status, count, hr;
        
        sscanf(cBufRead, "%d %d %d", &status, &count, &hr);
        
        heartRate = hr;
        
        Emote_setHistory(heartRate);
        
        averagePulse = Emote_calculateAveragePulse();
        
//        printf("Data: %s\n", cBufRead);
//        printf("Pulse: %d\n", hr);
        
//        printf("Time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	}
    
    return NULL;
}

int Emote_getHeartRate(void)
{
    printf("Heart: %d\n", heartRate);
    return heartRate;
}

void Emote_resetCircularBuffer(Emote_CircularBuffer* buff)
{
    if (buff->buffer) {
        free(buff->buffer);
    }
    
    buff->buffer = NULL;
    buff->lenght = 0;
    buff->start = 0;
}

void Emote_setCircularBufferLength(Emote_CircularBuffer* buff, unsigned int lenght)
{
    Emote_resetCircularBuffer(buff);
    
    buff->buffer = (int *)malloc(lenght * sizeof(int));
    memset(buff->buffer, 0, sizeof(int) * lenght);
    buff->lenght = lenght;
}

void Emote_writeToCircularBuffer(Emote_CircularBuffer* buff, int value)
{
    if (buff->lenght > 0) {
        buff->buffer[buff->start] = value;
        buff->start = (buff->start + 1) % buff->lenght;
    }
}

int Emote_readFromCircularBuffer(Emote_CircularBuffer* buff, int index)
{
    if (buff->lenght == 0) {
        return -1;
    }
    
    return buff->buffer[(buff->start + index) % buff->lenght];
}

void Emote_printCircularBuffer(Emote_CircularBuffer* buff)
{
    printf("Buffer: ");
    
    for(int i = 0; i < buff->lenght; i++) {
        printf("%d ", buff->buffer[i]);
    }
    
    printf("\n");
}

void Emote_setHistoryLength(unsigned int lenght)
{
    pthread_mutex_lock(&historyMutex);
    Emote_setCircularBufferLength(&historyBuffer, lenght);
    pthread_mutex_unlock(&historyMutex);
    
    Emote_printCircularBuffer(&historyBuffer);
}

int Emote_getHistory(int index)
{
    pthread_mutex_lock(&historyMutex);
    int value = Emote_readFromCircularBuffer(&historyBuffer, index);
    pthread_mutex_unlock(&historyMutex);
    
    return value;
}

void Emote_setHistory(int value)
{
        Emote_writeToCircularBuffer(&historyBuffer, value);
}

float Emote_calculateAveragePulse(void)
{
    float avg = 0.f;
    
    printf("Average \n");
    Emote_printCircularBuffer(&historyBuffer);
    
    int useful = 0;
    
    for (int i = 0; i < historyBuffer.lenght; i++) {
        int p = Emote_getHistory(i);
        
        if (p == 0) {
            continue;
        }
        
        avg += p;
        useful += 1;
    }
    
    avg /= useful;
    
    return avg;
}

float Emote_getStressLevel(void) {
    static const float threshold = 15.f;
    float avg = averagePulse;
    
    float stress = 0.f;
    
    if (avg > referencePulse) {
        stress = (avg - referencePulse) / threshold;
    }
    
    printf("stress: %f\n", stress);
    
    if (stress > 1.f) {
        stress = 1.f;
    }
    
    return stress;
}

int Emote_getReferenceHeartRate(void) {
    return referencePulse;
}

float Emote_getAverageHeartRate(void) {
    return averagePulse;
}

void Emote_calibrate(void) {
    if (initSuccess == 0) {
        sprintf(eeError, "Can't start calibration after failed initialization.");
        Emote_printErrorMessage();
        return;
    }
    
    // Create the thread using POSIX routines.
    pthread_attr_t  attr;
    int             returnVal;
    
    returnVal = pthread_attr_init(&attr);
    assert(!returnVal);
    returnVal = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    assert(!returnVal);
    
    int threadError = pthread_create(&posixThreadID, &attr, &Emote_calibratingThreadMain, NULL);
    
    returnVal = pthread_attr_destroy(&attr);
    assert(!returnVal);
    if (threadError != 0)
    {
        // Report an error.
    }
}

void * Emote_calibratingThreadMain(void* data)
{
    int waitTime = historyBuffer.lenght;
    printf("Calibration started\n");
    printf("Reference pulse %d\n", referencePulse);
    sleep(waitTime);
    
    referencePulse = averagePulse;
    
    printf("Calibration finished\n");
    printf("Reference pulse %d\n", referencePulse);
    
    return NULL;
}
