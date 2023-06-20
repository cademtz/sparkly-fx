#include "strtools.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define AssertValidReadPtr(x) (void)(x)
#define AssertValidWritePtr(x) (void)(x)
#define AssertValidStringPtr(x) (void)(x)

int Q_stricmp(const char* a, const char* b)
{
    for (size_t i = 0; a[i] && b[i]; ++i) {
        int diff = std::tolower(a[i]) - std::tolower(b[i]);
        if (diff != 0)
            return diff;
    }
    return 0;
}

int V_UnicodeToUTF8( const wchar_t *pUnicode, char *pUTF8, int cubDestSizeInBytes )
{
	//AssertValidStringPtr(pUTF8, cubDestSizeInBytes); // no, we are sometimes pasing in NULL to fetch the length of the buffer needed.
	AssertValidReadPtr(pUnicode);

	if ( cubDestSizeInBytes > 0 )
	{
		pUTF8[0] = 0;
	}

#ifdef _WIN32
	int cchResult = WideCharToMultiByte( CP_UTF8, 0, pUnicode, -1, pUTF8, cubDestSizeInBytes, NULL, NULL );
#elif POSIX
	int cchResult = 0;
	if ( pUnicode && pUTF8 )
		cchResult = wcstombs( pUTF8, pUnicode, cubDestSizeInBytes ) + 1;
#endif

	if ( cubDestSizeInBytes > 0 )
	{
		pUTF8[cubDestSizeInBytes - 1] = 0;
	}

	return cchResult;
}

int V_UTF8ToUnicode( const char *pUTF8, wchar_t *pwchDest, int cubDestSizeInBytes )
{
	// pwchDest can be null to allow for getting the length of the string
	if ( cubDestSizeInBytes > 0 )
	{
		AssertValidWritePtr(pwchDest);
		pwchDest[0] = 0;
	}

	if ( !pUTF8 )
		return 0;

	AssertValidStringPtr(pUTF8);

#ifdef _WIN32
	int cchResult = MultiByteToWideChar( CP_UTF8, 0, pUTF8, -1, pwchDest, cubDestSizeInBytes / sizeof(wchar_t) );
#elif POSIX
	int cchResult = mbstowcs( pwchDest, pUTF8, cubDestSizeInBytes / sizeof(wchar_t) ) + 1;
#endif

	if ( cubDestSizeInBytes > 0 )
	{
		pwchDest[(cubDestSizeInBytes / sizeof(wchar_t)) - 1] = 0;
	}

	return cchResult;
}
