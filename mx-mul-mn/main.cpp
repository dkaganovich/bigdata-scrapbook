#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

namespace {

	const char *INPUT_FILENAME = "input.bin";
	const char *OUTPUT_FILENAME = "output.bin";

	void genInput(const char* filename, int N, int M) {
		FILE* os = fopen(filename, "wb");
		fwrite(&N, 1, sizeof(uint32_t), os);
		fwrite(&M, 1, sizeof(uint32_t), os);
		//srand(time(NULL));
		uint8_t t;
		for (int i = 0; i < N * M; ++i) {
			//t = rand() % 256;
			t = i % 256;
			fwrite(&t, 1, sizeof(uint8_t), os);
		}
		for (int i = 0; i < M; ++i) {
			//t = rand() % 256;
			t = i % 256;
			fwrite(&t, 1, sizeof(uint8_t), os);
		}
		fclose(os);
	}

	void outputInput(const char* filename, FILE* os) {
		uint32_t N, M;
		FILE* is = fopen(filename, "rb");
		fread(&N, sizeof(uint32_t), 1, is);
		fprintf(os, "%" PRIu32 "\n", N);
		fread(&M, sizeof(uint32_t), 1, is);
		fprintf(os, "%" PRIu32 "\n", M);
		fprintf(os, "\n");
		uint8_t t;
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < M; ++j) {
				fread(&t, sizeof(uint8_t), 1, is);
				fprintf(os, "%d ", t);
			}
			fprintf(os, "\n");
		}
		fprintf(os, "\n");
		for (int i = 0; i < M; ++i) {
			fread(&t, sizeof(uint8_t), 1, is);
			fprintf(os, "%d\n", t);
		}
		fclose(is);
	}

	void outputRes(const char* filename, FILE* os) {
		FILE* is = fopen(filename, "rb");
		fseek(is, 0L, SEEK_END);
		size_t size = ftell(is);
		rewind(is);
		uint8_t t;
		for (int i = 0; i < size / sizeof(uint8_t); ++i) {
			fread(&t, sizeof(uint8_t), 1, is);
			fprintf(os, "%d\n", t);
		}
		fclose(is);
	}

}

int main(int argc, char* argv[])
{
	//genInput(INPUT_FILENAME, 30, 3);
	//outputInput(INPUT_FILENAME, stdout);

	const int BLOCK_SIZE = 256;//5;//256;// B;

	FILE* is = fopen(INPUT_FILENAME, "rb");
	FILE* os = fopen(OUTPUT_FILENAME, "wb");

	uint32_t N, M;

	fread(&N, sizeof(uint32_t), 1, is);
	fread(&M, sizeof(uint32_t), 1, is);

	int aChunkWidth, aChunkHeight;
	if (N > BLOCK_SIZE && M <= BLOCK_SIZE) {
		aChunkWidth = M;
		aChunkHeight = BLOCK_SIZE * BLOCK_SIZE / M;
	}
	else if (N <= BLOCK_SIZE && M > BLOCK_SIZE) {
		aChunkWidth = BLOCK_SIZE * BLOCK_SIZE / N;
		aChunkHeight = N;
	}
	else if (N <= BLOCK_SIZE && M <= BLOCK_SIZE) {
		aChunkWidth = M;
		aChunkHeight = N;
	}
	else {
		aChunkWidth = BLOCK_SIZE;
		aChunkHeight = BLOCK_SIZE;
	}

	/*printf("aChunkWidth: %d\n", aChunkWidth);
	printf("aChunkHeight: %d\n", aChunkHeight);*/

	bool aHParted = M > aChunkWidth;
	bool aVParted = N > aChunkHeight;

	int bChunkHeight = aHParted ? aChunkHeight * aChunkWidth : aChunkWidth;
	int cChunkHeight = aVParted ? aChunkHeight * aChunkWidth : aChunkHeight;

	/*printf("bChunkHeight: %d\n", bChunkHeight);
	printf("cChunkHeight: %d\n", cChunkHeight);*/

	uint8_t* a = new uint8_t[aChunkHeight * aChunkWidth];
	uint8_t* b = new uint8_t[bChunkHeight];
	uint8_t* c = new uint8_t[cChunkHeight];

	if (!aHParted) {
		fseek(is, 2 * sizeof(uint32_t) + N * M * sizeof(uint8_t), SEEK_SET);
		fread(b, sizeof(uint8_t), bChunkHeight, is);
	}

	for (int i = 0; i < N; i += aChunkHeight)
	{
		int arows = (i + aChunkHeight - 1 < N ? aChunkHeight : N - i);

		for (int k = 0; k < M; k += aChunkWidth)
		{
			int acols = (k + aChunkWidth - 1 < M ? aChunkWidth : M - k);
			int brows = acols;

			size_t offs = 2 * sizeof(uint32_t) + (i * M + k) * sizeof(uint8_t);
			if (!aHParted) {
				fseek(is, offs, SEEK_SET);
				fread(a, sizeof(uint8_t), arows * acols, is);
			}
			else {
				for (int l = 0; l < arows; ++l) {
					fseek(is, offs + l * M * sizeof(uint8_t), SEEK_SET);
					fread(a + l * acols, sizeof(uint8_t), acols, is);
				}
			}

			if (aHParted) {
				if (k % bChunkHeight == 0) {
					fseek(is, 2 * sizeof(uint32_t) + (N * M + k) * sizeof(uint8_t), SEEK_SET);
					fread(b, sizeof(uint8_t), bChunkHeight, is);
				}
			}

			for (int ib = 0; ib < arows; ++ib) {
				if (k == 0) {
					c[i % cChunkHeight + ib] = 0.f;
				}
				for (int kb = 0; kb < acols; ++kb) {
					c[i % cChunkHeight + ib] = (c[i % cChunkHeight + ib] + (uint32_t)a[ib * acols + kb] * b[k % bChunkHeight + kb]) % 256;
				}
			}
		}

		/*printf("i: %d\n", i);
		printf("arows: %d\n", arows);*/

		if ((i + arows) % cChunkHeight == 0 || (i + arows) == N) {
			int ord = i / cChunkHeight;
			fseek(os, cChunkHeight * ord * sizeof(uint8_t), SEEK_SET);
			fwrite(c, i + arows - cChunkHeight * ord, sizeof(uint8_t), os);
		}
	}

	delete[] a;
	delete[] b;
	delete[] c;

	fclose(is);
	fclose(os);

	//printf("\n");
	//outputRes(OUTPUT_FILENAME, stdout);

	return 0;
}
