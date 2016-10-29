#include <stdio.h>
#include <stdint.h>
#include <time.h>

namespace {

	const char *INPUT_FILENAME = "input.bin";
	const char *OUTPUT_FILENAME = "output.bin";

    // void generateBinMx(const char* filename, int N, int M) {
    //     std::basic_ofstream<uint8_t> os(filename, std::ios::binary);
    //     if (os.is_open()) {
    //         os.write((uint8_t*)&N, sizeof(uint32_t));
    //         os.write((uint8_t*)&M, sizeof(uint32_t));
    //         uint8_t el;
    //         for (int i = 0; i < N * M; ++i) {
    //             el = i % 256;
    //             os.write(&el, sizeof(uint8_t));
    //         }
    //         os.close();
    //     }
    // }

    // void writeoutBinMx(const char* ifilename, const char* ofilename) {
    //     std::basic_ifstream<uint8_t> is(ifilename, std::ios::binary);
    //     std::ofstream of(ofilename);
    //     if (is.is_open() && of.is_open()) {
    //         uint32_t N, M;
    //         is.read((uint8_t*)&N, sizeof(uint32_t));
    //         is.read((uint8_t*)&M, sizeof(uint32_t));
    //         of << N << std::endl;
    //         of << M << std::endl;
    //         uint8_t el;
    //         for (int i = 0; i < N; ++i) {
    //             for (int j = 0; j < M; ++j) {
    //                 is.read(&el, sizeof(uint8_t));
    //                 of << (uint32_t)el << " ";
    //             }
    //             of << "\n";
    //         }
    //         is.close();
    //         of.close();
    //     }
    // }
    
}

int main(int argc, char* argv[]) 
{
	/*std::basic_ifstream<uint8_t> is(INPUT_FILENAME, std::ios::binary);
	std::basic_ofstream<uint8_t> os(OUTPUT_FILENAME, std::ios::binary);*/

	FILE* is = fopen(INPUT_FILENAME, "rb");
	FILE* os = fopen(OUTPUT_FILENAME, "wb");

	const int BLOCK_SIZE = 600;

	//if (is.is_open() && os.is_open()) 
	if (is != NULL && os != NULL)
	{
		uint32_t N, M;
		uint8_t chunk[BLOCK_SIZE * BLOCK_SIZE];

		/*is.read((uint8_t*)&N, sizeof(uint32_t));
		is.read((uint8_t*)&M, sizeof(uint32_t));*/
		fread(&N, sizeof(uint32_t), 1, is);
		fread(&M, sizeof(uint32_t), 1, is);
		
		int chunkWidth, chunkHeight;// adjust chunk size
		if (N > BLOCK_SIZE && M <= BLOCK_SIZE) {
			chunkWidth = M;
			chunkHeight = BLOCK_SIZE * BLOCK_SIZE / M;
		}
		else if (N <= BLOCK_SIZE && M > BLOCK_SIZE) {
			chunkWidth = BLOCK_SIZE * BLOCK_SIZE / N;
			chunkHeight = N;
		}
		else {
			chunkWidth = BLOCK_SIZE;
			chunkHeight = BLOCK_SIZE;
		}

		bool hParted = M > chunkWidth;
		bool vParted = N > chunkHeight;

		uint8_t* colbuf = new uint8_t[chunkHeight];

		//os.write((uint8_t*)&M, sizeof(uint32_t));// output dimensions
		//os.write((uint8_t*)&N, sizeof(uint32_t));
		fwrite(&M, sizeof(uint32_t), 1, os);
		fwrite(&N, sizeof(uint32_t), 1, os);
		for (int i = 0; i < N; i += chunkHeight) 
		{
			int nrows = (i + chunkHeight - 1 < N ? chunkHeight : N - i);

			for (int j = 0; j < M; j += chunkWidth) 
			{
				int ncols = (j + chunkWidth - 1 < M ? chunkWidth : M - j);

				// read chunk at a time
				size_t offs = 2 * sizeof(uint32_t) + (i * M + j) * sizeof(uint8_t);
				if (!hParted) {
					/*is.seekg(offs, is.beg);
					is.read(chunk, nrows * ncols * sizeof(uint8_t));*/
					fseek(is, offs, SEEK_SET);
					fread(chunk, sizeof(uint8_t), nrows * ncols, is);
				}
				else {
					for (int k = 0; k < nrows; ++k) {
						/*is.seekg(offs + k * M * sizeof(uint8_t), is.beg);
						is.read(chunk + k * ncols, ncols * sizeof(uint8_t));*/
						fseek(is, offs + k * M * sizeof(uint8_t), SEEK_SET);
						fread(chunk + k * ncols, sizeof(uint8_t), ncols, is);
					}
				}

				// transpose & output it
				offs = 2 * sizeof(uint32_t) + (j * N + i) * sizeof(uint8_t);
				//os.seekp(offs, os.beg);
				fseek(os, offs, SEEK_SET);
				if (ncols == 1 || N == 1) {
					//os.write(chunk, ncols * nrows * sizeof(uint8_t));
					fwrite(chunk, sizeof(uint8_t), ncols * nrows, os);
				}
				else {
					for (int tj = 0; tj < ncols; ++tj) {
					  if (vParted) {
					      //os.seekp(offs + tj * N * sizeof(uint8_t), os.beg);
						  fseek(os, offs + tj * N * sizeof(uint8_t), SEEK_SET);
					  }
					  for (int ti = 0; ti < nrows; ++ti) {
					      colbuf[ti] = chunk[ti * ncols + tj];
					  }
					  //os.write(colbuf, nrows * sizeof(uint8_t));
					  fwrite(colbuf, sizeof(uint8_t), nrows, os);
					}
				}
			}
		}

		delete[] colbuf;

		/*is.close();
		os.close();*/
		fclose(is);
		fclose(os);
	}

	return 0;
}
