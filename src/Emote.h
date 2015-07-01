//
//  Emote.h
//  Emote
//
//  Created by Paweł Gajewski on 3/7/12.
//  Copyright (c) 2012 something.pl. All rights reserved.
//

#ifndef Emote_Emote_h
#define Emote_Emote_h

#ifdef EMOTE_EXPORTS
#define EMOTEDLL_API __declspec(dllexport) 
#else
#define EMOTEDLL_API __declspec(dllimport) 
#endif

#ifdef __cplusplus
extern "C" {
#endif

	EMOTEDLL_API int     Emote_init(void);
	EMOTEDLL_API void    Emote_shutdown(void);
	EMOTEDLL_API void    Emote_printErrorMessage(void);
	EMOTEDLL_API  void    Emote_startMonitoring(void);
	EMOTEDLL_API void    Emote_stopMonitoring(void);
	EMOTEDLL_API int     Emote_getHeartRate(void);
	EMOTEDLL_API void    Emote_setHistoryLength(unsigned int);
	EMOTEDLL_API int     Emote_getHistory(int index);
	EMOTEDLL_API void    Emote_setHistory(int value);
	EMOTEDLL_API float   Emote_getStressLevel(void);
	EMOTEDLL_API int     Emote_getReferenceHeartRate(void);
	EMOTEDLL_API float   Emote_getAverageHeartRate(void);
	EMOTEDLL_API void    Emote_calibrate(void);
    
#ifdef __cplusplus
}
#endif

#endif
