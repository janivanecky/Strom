#pragma once
#include <stdint.h>

struct File
{
	void *data;
	uint32_t size;
};

File ReadFile(char *fileName);
void ReleaseFile(File file);
uint32_t WriteFile(char *filename, void *data, uint32_t size);
