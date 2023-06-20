#pragma once

/*	Comments:
 *		Hold on:
 *			y vlave
 */

#include <cstring>
#include <cctype>
#include <cstdio>

#define Q_snprintf snprintf
#define Q_strlen strlen
#define Q_memcpy memcpy
#define Q_strncpy strncpy
#define Q_atoi64 atoll
#define Q_UnicodeToUTF8 V_UnicodeToUTF8
#define Q_UTF8ToUnicode V_UTF8ToUnicode

#define V_snprintf snprintf
#define V_wcslen wcslen

int Q_stricmp(const char* a, const char* b);
int V_UnicodeToUTF8( const wchar_t *pUnicode, char *pUTF8, int cubDestSizeInBytes );
int V_UTF8ToUnicode( const char *pUTF8, wchar_t *pwchDest, int cubDestSizeInBytes );