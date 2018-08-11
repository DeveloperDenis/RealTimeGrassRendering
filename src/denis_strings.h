#ifndef DENIS_STRINGS_H_
#define DENIS_STRINGS_H_

#include "denis_types.h"

#define IS_LOWER_CASE(c) ((c) >= 'a' && (c) <= 'z')
#define IS_UPPER_CASE(c) ((c) >= 'A' && (c) <= 'Z')
#define IS_LETTER(c) (IS_LOWER_CASE(c) || IS_UPPER_CASE(c))

#define IS_NUMBER(c) ((c) >= '0' && (c) <= '9')

static inline bool charInString(char c, char* string)
{
	bool result = false;

	if (string)
	{
		for (int i = 0; string[i] != 0 && !result; ++i)
		{
			if (string[i] == c)
				result = true;
		}
	}
	
	return result;
}

//NOTE(denis): can be used if the strings are not valid C strings
static inline bool charArraysEqual(char* array1, int size1, char* array2, int size2)
{
	bool areEqual = TRUE;
	int arraySize = size1 < size2 ? size1 : size2;

	int i;
	for (i = 0; i < arraySize && areEqual; ++i)
	{
		if (array1[i] != array2[i])
			areEqual = FALSE;
	}

	return areEqual;
}

static inline bool stringStartsWith(char* line, char* identifier)
{
	u32 length;
	for (length = 0; identifier[length] != 0; ++length)
		;
	
	return charArraysEqual(line, length, identifier, length);
}

//NOTE(denis): assumes that string1 and string2 are valid C strings
static inline bool stringsEqual(char* string1, char* string2)
{
	bool areEqual = TRUE;
	
	int i = 0;
	while (string1[i] != 0 && string2[i] != 0 && areEqual)
	{
		if (string1[i] != string2[i])
			areEqual = FALSE;

		++i;
	}

	if (string1[i] != string2[i])
		areEqual = FALSE;
	
	return areEqual;
}

static inline bool stringsEqualIgnoreCase(char* string1, char* string2)
{
	bool areEqual = TRUE;

	
	int i = 0;
	while (string1[i] != 0 && string2[i] != 0 && areEqual)
	{
		if (string1[i] != string2[i])
		{
			if (IS_LOWER_CASE(string1[i]) && IS_UPPER_CASE(string2[i]))
			{
				areEqual = (string1[i] - 'a') == (string2[i] - 'A');
			}
			else if (IS_UPPER_CASE(string1[i]) && IS_LOWER_CASE(string2[i]))
			{
				areEqual = (string1[i] - 'A') == (string2[i] - 'a');
			}
			else
				areEqual = FALSE;
		}

		++i;
	}

	if ((string1[i] == 0 && string2[i] != 0) ||
		(string1[i] != 0 && string2[i] == 0))
	{
		areEqual = FALSE;
	}
	
	return areEqual;
}

//NOTE(denis): returns a pointer that is equal to string, > than string
// by the number non-blank spaces in the beginning, or NULL if the string contains
// only blanks
// trims spaces, newline characters, and tabs
static char* trimString(char* string)
{
	char* result = string;
	
	if (string)
	{
		int i = 0;
		while (string[i] != 0 && (string[i] == ' ' || string[i] == '\n' || string[i] == '\t'))
			++i;
	
		if (string[i] != 0)
		{
			result = string + i;

			int lastNonBlankIndex = i;
			while (string[i] != 0)
			{
				if (string[i] != ' ' && string[i] != '\n' && string[i] != '\t')
					lastNonBlankIndex = i;

				++i;
			}
		
			if (lastNonBlankIndex < i - 1)
				string[lastNonBlankIndex+1] = 0;
		}
		else
			result = NULL;
	}

	return result;
}

//NOTE(denis): does not include the /0 in the string length
static inline int getStringSize(char* string)
{
	int result = 0;

	while (string && string[result] != 0)
	{
		++result;
	}
	
	return result;
}

//NOTE(denis): string returned from this function must be freed by the user
static inline char* createStringFromArray(char* array, int size)
{
	char* result = NULL;
 
	if (array)
	{
		result = (char*)HEAP_ALLOC(size+1);
	
		int i;
		for (i = 0; i < size; ++i)
		{
			result[i] = array[i];
		}
		result[size] = 0;
	}
	return result;
}

static inline char* duplicateString(char *string)
{
	char *result = 0;
	
	if (string)
	{
		u32 numChars = getStringSize(string);

		result = (char*)HEAP_ALLOC(numChars+1);

		for (u32 i = 0; i < numChars; ++i)
		{
			result[i] = string[i];
		}
	}

	return result;
}

//NOTE(denis): user must free result of this function
static char** tokenizeStringInPlace(char* string, int maxTokens, char separator)
{
	char** tokenArray = (char**)HEAP_ALLOC(maxTokens*sizeof(char*));
	
	int i;
	for (i = 0; i < maxTokens; ++i)
		tokenArray[i] = 0;

	int charIndex = 0;
	for (i = 0; i < maxTokens; ++i)
	{
		while(string[charIndex] != 0 && string[charIndex] == separator)
			++charIndex;

		if (string[charIndex] == 0)
			break;

		tokenArray[i] = string + charIndex;
		++charIndex;

		while(string[charIndex] != 0 && string[charIndex] != separator)
			++charIndex;

		if (string[charIndex] != 0)
			string[charIndex++] = 0;
		else
			break;
	}

	return tokenArray;
}

 
//NOTE(denis): assumes that destination is big enough to hold all of source's
// characters
static inline void copyIntoString(char *destination, char *source)
{
	if (destination && source)
	{
		for (u32 i = 0; source[i] != 0; ++i)
		{
			destination[i] = source[i];
		}
	}
}
 
//NOTE(denis): copies a portion of source into destination,
// the first character copied is index = beginning
// and the last character copied is at index = end
static inline void copyIntoString(char *destination, char *source,
								  u32 beginning, u32 end)
{
	u32 destinationIndex = 0;
	 
	if (destination && source && beginning < end)
	{
		for (u32 i = beginning; i <= end; ++i)
		{
			destination[destinationIndex++] = source[i];
		}
	}
}
 
//NOTE(denis): returns a new string which is a+b
static char* concatStrings(char *a, char *b)
{
	u32 sizeOfA = 0;
	u32 sizeOfB = 0;
	char *result = 0;
 
	if (a && b)
	{
		for (u32 i = 0; a[i] != 0; ++i)
		{
			++sizeOfA;
		}
 
		for (u32 i = 0; b[i] != 0; ++i)
		{
			++sizeOfB;
		}
 
		result = (char*)HEAP_ALLOC(sizeOfA+sizeOfB+1);
 
		copyIntoString(result, a);
		copyIntoString(result+sizeOfA, b);
	}
	 
	return result;
}

//TODO(denis): these functions do not handle negative numbers properly
static bool toString(s32 num, char* buffer, u32 maxLength)
{
	bool success = false;
	u32 length = 0;
	s32 temp = num;
	while (temp > 0)
	{
		temp /= 10;
		++length;
	}
	
	if (maxLength > 0 && length > 0 && length <= maxLength)
	{
		temp = num;
		for (u32 i = 0; i < length && temp > 0; ++i, temp /= 10)
		{
			buffer[(length - 1) - i] = '0' + (temp % 10);
		}

		success = true;
	}

	return success;
}

static char* toString(s32 num)
{
	char* result = 0;
	u32 length = 0;

	//TODO(denis): probably not a huge deal, but this is computed twice
	s32 temp = num;
	while (temp > 0)
	{
		temp /= 10;
		++length;
	}

	if (length > 0)
	{
		result = (char*)HEAP_ALLOC(length+1);
		if (!toString(num, result, length))
		{
			HEAP_FREE(result);
			result = 0;
		}
	}
	
	return result;
}

//TODO(denis): make alternative to this?
#include <cmath>

static f32 parseF32String(char* string)
{
	s32 intPart = 0;
	s32 realPart = 0;
	u32 numDecimalDigits = 0;
	
	bool readingDecimalPart = false;
	bool isNegative = false;

	if (!string)
		return 0;
	
	u32 i = 0;
	while (string[i] != 0)
	{
		if (string[i] == '-')
			isNegative = true;
		else if (string[i] == '.')
			readingDecimalPart = true;
		else if (string[i] >= '0' && string[i] <= '9')
		{
			if (!readingDecimalPart)
			{
				intPart *= 10;
				intPart += string[i] - '0';
			}
			else
			{
				realPart *= 10;
				realPart += string[i] - '0';
				++numDecimalDigits;
			}
		}
		else
			break;

		++i;
	}

	f32 result = 0.0f;
	result += intPart;
	result += (f32)realPart/pow(10.0f, (f32)numDecimalDigits);

	if (isNegative)
		result = -result;
	
	return result;
}

static u32 parseU32String(char* string)
{
	u32 result = 0;

	if (!string)
		return result;
	
	u32 i = 0;
	while (string[i] != 0)
	{
		if (string[i] >= '0' && string[i] <= '9')
		{
			result *= 10;
			result += string[i] - '0';
		}
		else
			break;

		++i;
	}
	
	return result;
}

static s32 parseS32String(char* string)
{
	s32 result = 0;
	bool isNegative = false;

	if (!string)
		return result;
	
	u32 i = 0;
	while (string[i] != 0)
	{
		if (string[i] == '-')
			isNegative = true;
		else if (string[i] >= '0' && string[i] <= '9')
		{
			result *= 10;
			result += string[i] - '0';
		}
		else
			break;

		++i;
	}

	if (isNegative)
		result = -result;
	
	return result;
}

#endif
