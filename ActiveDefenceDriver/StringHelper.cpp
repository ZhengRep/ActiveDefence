#include "StringHelper.h"

//Check end
BOOLEAN IsUnistrEndWithWcs(IN PUNICODE_STRING UnicodeString, IN PWCHAR Wchar)
{
	if (!UnicodeString || !Wchar || !UnicodeString->Length || !UnicodeString->Buffer)
	{
		return FALSE;
	}

	USHORT usUnstCharicterCount = UnicodeString->Length / sizeof(WCHAR);
	USHORT usWcharCharicterCount = (USHORT)wcslen(Wchar);

	if (usUnstCharicterCount < usWcharCharicterCount)
	{
		return FALSE;
	}

	PWCHAR UnicodeStringEnd = UnicodeString->Buffer + (usUnstCharicterCount - usWcharCharicterCount);
	if (!_wcsnicmp(UnicodeStringEnd, Wchar, usWcharCharicterCount))
	{
		return TRUE;
	}

	return FALSE;
}