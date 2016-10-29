#include <stdint.h>
#include <stdio.h>

namespace {

	const char *INPUT_FILENAME = "input.bin";
	const char *OUTPUT_FILENAME = "output.bin";

}

int main(int argc, char* argv[])
{
	FILE* is = fopen(INPUT_FILENAME, "rb");
	FILE* os = fopen(OUTPUT_FILENAME, "wb");

	const int BLOCK_SIZE = 400;

	if (is != NULL && os != NULL)
	{
		uint32_t N;// square matrices: a x b = c
		uint8_t a[BLOCK_SIZE * BLOCK_SIZE];
		uint8_t b[BLOCK_SIZE * BLOCK_SIZE];
		uint8_t c[BLOCK_SIZE * BLOCK_SIZE];

		fread(&N, sizeof(uint32_t), 1, is);

		fwrite(&N, sizeof(uint32_t), 1, os);
		fwrite(&N, sizeof(uint32_t), 1, os);
		for (int i = 0; i < N; i += BLOCK_SIZE)
		{
			int arows = (i + BLOCK_SIZE - 1 < N ? BLOCK_SIZE : N - i);
			for (int j = 0; j < N; j += BLOCK_SIZE)
			{
				int bcols = (j + BLOCK_SIZE - 1 < N ? BLOCK_SIZE : N - j);

				for (int k = 0; k < N; k += BLOCK_SIZE) 
				{
					int acols = (k + BLOCK_SIZE - 1 < N ? BLOCK_SIZE : N - k);
					int brows = acols;

					size_t offs = 2 * sizeof(uint32_t) + (i * N + k) * sizeof(uint8_t);
					for (int l = 0; l < arows; ++l) {
						fseek(is, offs + l * N * sizeof(uint8_t), SEEK_SET);
						fread(a + l * acols, sizeof(uint8_t), acols, is);
					}

					offs = 4 * sizeof(uint32_t) + (N * N + k * N + j) * sizeof(uint8_t);
					for (int l = 0; l < brows; ++l) {
						fseek(is, offs + l * N * sizeof(uint8_t), SEEK_SET);
						fread(b + l * bcols, sizeof(uint8_t), bcols, is);
					}

					for (int ib = 0; ib < arows; ++ib) {
						if (k == 0) {
							for (int jb = 0; jb < bcols; ++jb) {
								c[ib * bcols + jb] = 0.f;
							}
						}
						for (int kb = 0; kb < acols; ++kb) {
							for (int jb = 0; jb < bcols; ++jb) {
								c[ib * bcols + jb] = (c[ib * bcols + jb] + (uint32_t)a[ib * acols + kb] * b[kb * bcols + jb]) % 256;
							}
						}
					}
				}

				int offs = 2 * sizeof(uint32_t) + (i * N + j) * sizeof(uint8_t);
				for (int m = 0; m < arows; ++m) {
					fseek(os, offs + m * N * sizeof(uint8_t), SEEK_SET);
					fwrite(c + m * bcols, bcols, sizeof(uint8_t), os);
				}
			}
		}

		fclose(is);
		fclose(os);
	}

	return 0;
}
