#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

namespace {

	const char *INPUT_FILENAME = "input.bin";
	const char *OUTPUT_FILENAME = "output.bin";

	void genInput(const char* filename, int a, int b) {
		FILE* os = fopen(filename, "wb");
		fwrite(&a, 1, sizeof(uint32_t), os);
		fwrite(&b, 1, sizeof(uint32_t), os);
		fclose(os);
	}

	void outputInput(const char* filename, FILE* os) {
		uint32_t a, b;
		FILE* is = fopen(filename, "rb");
		fread(&a, sizeof(uint32_t), 1, is);
		fread(&b, sizeof(uint32_t), 1, is);
		fprintf(os, "%" PRIu32 " %" PRIu32 "\n", a, b);
		fclose(is);
	}

	void outputRes(const char* filename, FILE* os) {
		FILE* is = fopen(filename, "rb");
		uint8_t c;
		fread(&c, sizeof(uint32_t), 1, is);
		fprintf(os, "%" PRIu32 "\n", c);
		fclose(is);
	}

}

int main(int argc, char* argv[])
{
	//genInput(INPUT_FILENAME, 1, 2);
	//outputInput(INPUT_FILENAME, stdout);

	FILE* is = fopen(INPUT_FILENAME, "rb");
	FILE* os = fopen(OUTPUT_FILENAME, "wb");

	uint32_t a, b, c;

	fread(&a, sizeof(uint32_t), 1, is);
	fread(&b, sizeof(uint32_t), 1, is);

	c = a + b;

	fwrite(&c, sizeof(uint32_t), 1, os);

	fclose(is);
	fclose(os);

	/*printf("\n");
	outputRes(OUTPUT_FILENAME, stdout);*/

	return 0;
}
