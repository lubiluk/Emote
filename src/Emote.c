//
//  Emote.c
//  Emote
//
//  Created by Paweł Gajewski on 3/7/12.
//  Copyright (c) 2012 something.pl. All rights reserved.
//

#include "Emote.h"
#include "ftd2xx.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#define BUF_SIZE 1

FT_HANDLE   ftHandle = NULL;
pthread_t   posixThreadID;

char eeError[255];

void* Emote_monitoringThreadMain(void* data);

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
    
    return 0;
}

void Emote_shutdown(void)
{
    pthread_join(posixThreadID, NULL);
    
    if(ftHandle != NULL) {
		FT_Close(ftHandle);
		ftHandle = NULL;
	}
}

void Emote_printErrorMessage(void)
{
    printf("%s\n", eeError);
}

void Emote_startMonitoringHeart(void)
{
    // Create the thread using POSIX routines.
    pthread_attr_t  attr;
    int             returnVal;
    
    returnVal = pthread_attr_init(&attr);
    assert(!returnVal);
    returnVal = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    assert(!returnVal);
    
    int threadError = pthread_create(&posixThreadID, &attr, &Emote_monitoringThreadMain, NULL);
    
    returnVal = pthread_attr_destroy(&attr);
    assert(!returnVal);
    if (threadError != 0)
    {
        // Report an error.
    }
}

void Emote_stopMonitoringHeart(void)
{
    
}

void* Emote_monitoringThreadMain(void* data)
{
    char cBufRead[BUF_SIZE];
	DWORD dwBytesRead;
    
    for(int i = 0; i < 10 ; i++) {
		FT_Read(ftHandle, cBufRead, BUF_SIZE, &dwBytesRead);
		if(dwBytesRead != BUF_SIZE)
			printf("Timeout %d\n", i);
		else
			printf("Read %d\n", dwBytesRead);
	}
    
    return NULL;
}


