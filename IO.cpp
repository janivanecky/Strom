#include "IO.h"
#include <Windows.h>

File ReadFile_(char *fileName)
{
	HANDLE fileHandle = 0;
	fileHandle = CreateFileA(fileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL, NULL);
	File result = { 0 };
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
		if (GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileAttributes))
		{
			uint32_t fileSize = fileAttributes.nFileSizeLow;
			HANDLE heap = GetProcessHeap();
			result.data = HeapAlloc(heap, HEAP_ZERO_MEMORY, fileSize);
			if (result.data)
			{
				DWORD bytesReadFromFile = 0;
				if (ReadFile(fileHandle, result.data, fileSize, &bytesReadFromFile, NULL) &&
					bytesReadFromFile == fileSize)
				{
					result.size = bytesReadFromFile;
				}
				else
				{
					HeapFree(heap, NULL, result.data);
					OutputDebugStringA("Error reading file\n");
				}
			}
			else
			{
				OutputDebugStringA("Allocation error\n");
			}
		}
		else
		{
			OutputDebugStringA("Error getting file attributes\n");
		}
		CloseHandle(fileHandle);
	}
	else
	{
		DWORD errorMessageID = ::GetLastError();
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		OutputDebugStringA("Error opening file, SYSTEM MSG: ");
		OutputDebugStringA(messageBuffer);
		LocalFree(messageBuffer);
	}
	return result;
}

uint32_t WriteFile(char *filename, void *data, uint32_t size)
{
	HANDLE fileHandle = 0;
	fileHandle = CreateFileA(filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS,
							 FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD bytesWritten = 0;
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		WriteFile(fileHandle, data, size, &bytesWritten, NULL);
		CloseHandle(fileHandle);
	}
	else
	{
		DWORD errorMessageID = ::GetLastError();
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		OutputDebugStringA("Error opening file, SYSTEM MSG: ");
		OutputDebugStringA(messageBuffer);
		LocalFree(messageBuffer);
	}
	return bytesWritten;
}

void ReleaseFile(File file)
{
	HANDLE heap = GetProcessHeap();
	HeapFree(heap, NULL, file.data);
}

bool CompareStrings(char *s1, char *s2)
{
	while (*s1 && *s2)
	{
		if (*s1 != *s2)
		{
			return false;
		}
		s1++;
		s2++;
	}
	if (*s1 != *s2)
		return false;

	return true;
}

File Unpack(char *packerPath, char *filename)
{
	File pack = ReadFile_(packerPath);
	uint32_t packedFilesCount = *((uint32_t *)pack.data);
	char *packedNames = (char *)pack.data + sizeof(uint32_t);
	uint32_t *packedSizes = (uint32_t *)(packedNames + packedFilesCount * 100);
	uint32_t *packedPositions = packedSizes + packedFilesCount;
	File result = {};
	for (uint32_t i = 0; i < packedFilesCount; ++i)
	{
		char *packedName = packedNames + i * 100;
		if (CompareStrings(packedName, filename))
		{
			HANDLE heap = GetProcessHeap();
			result.data = HeapAlloc(heap, HEAP_ZERO_MEMORY, packedSizes[i]);
			memcpy(result.data, (char *)pack.data + packedPositions[i], packedSizes[i]);
			result.size = packedSizes[i];
		}
	}
	ReleaseFile(pack);
	if (result.size == 0)
	{
		result = ReadFile_(filename);
	}
	return result;
}

//#define PACKED_DATA
File ReadFile(char *fileName)
{
#ifdef PACKED_DATA
	return Unpack("data", fileName);
#else
	return ReadFile_(fileName);
#endif
}