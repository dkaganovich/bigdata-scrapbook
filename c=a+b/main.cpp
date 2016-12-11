#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <algorithm>

namespace {

	const char *INPUT_FILENAME = "input.bin";
	const char *OUTPUT_FILENAME = "output.bin";

	void genInput(const char* filename, int n, int m) {
		FILE* os = fopen(filename, "wb");
		fwrite(&n, sizeof(uint32_t), 1, os);
		uint8_t t = 9;
		fwrite(&t, sizeof(uint8_t), 1, os);
		srand(time(NULL));
		for (int i = 1; i < n; ++i) {
			t = rand() % 10;
			fwrite(&t, sizeof(uint8_t), 1, os);
		}
		fwrite(&m, sizeof(uint32_t), 1, os);
		t = 1;
		fwrite(&t, sizeof(uint8_t), 1, os);
		for (int i = 1; i < m; ++i) {
			t = rand() % 10;
			fwrite(&t, sizeof(uint8_t), 1, os);
		}
		fclose(os);
	}

	void outputInput(const char* filename, FILE* os) {
		uint32_t n, m;
		FILE* is = fopen(filename, "rb");
		fread(&n, sizeof(uint32_t), 1, is);
		uint8_t t;
		for (int i = 0; i < n; ++i) {
			fread(&t, sizeof(uint8_t), 1, is);
			fprintf(os, "%d", t);
		}
		fprintf(os, "\n");
		fread(&m, sizeof(uint32_t), 1, is);
		for (int i = 0; i < m; ++i) {
			fread(&t, sizeof(uint8_t), 1, is);
			fprintf(os, "%d", t);
		}
		fclose(is);
	}

	void outputRes(const char* filename, FILE* os) {
		uint32_t k;
		FILE* is = fopen(filename, "rb");
		fread(&k, sizeof(uint32_t), 1, is);
		uint8_t t;
		for (int i = 0; i < k; ++i) {
			fread(&t, sizeof(uint8_t), 1, is);
			fprintf(os, "%d", t);
		}
	}

	template <class T>
	class BufferedReader
	{
	public:
		BufferedReader(FILE* file, int bufferSize) : file(file), numBufferItems(bufferSize / sizeof(T)) {
			buffer = new T[numBufferItems];
			readCnt = fread(buffer, sizeof(T), numBufferItems, file);
			isEmpty = false;
			offset = 0;
		}

		BufferedReader(const char* filename, const char* mode, int bufferSize) : numBufferItems(bufferSize / sizeof(T)) {
			file = fopen(filename, mode);// error handling here
			buffer = new T[numBufferItems];
			readCnt = fread(buffer, sizeof(T), numBufferItems, file);// >0 assumption
			isEmpty = false;
			offset = 0;
		}

		~BufferedReader() {
			if (file != NULL) {
				fclose(file);
			}
			delete[] buffer;
		}

		const T& get() {
			if (isEmpty) {
				return buffer[readCnt - 1];
			}
			return buffer[offset];
		}

		bool seekNext() {
			if (isEmpty) {
				return false;
			}
			++offset;
			if (offset == readCnt) {
				if (readCnt < numBufferItems) {
					isEmpty = true;
				}
				else {
					readCnt = fread(buffer, sizeof(T), numBufferItems, file);
					if (readCnt == 0) {
						isEmpty = true;
					}
					else {
						offset = 0;
					}
				}
			}
			return !isEmpty;
		}

		void reload() {
			readCnt = fread(buffer, sizeof(T), numBufferItems, file);
			isEmpty = (readCnt == 0);
			offset = 0;
		}

		void reset() {
			rewind(file);
			readCnt = fread(buffer, sizeof(T), numBufferItems, file);
			isEmpty = false;
			offset = 0;
		}

		void close() {
			fclose(file);
			file = NULL;
		}

	public:
		FILE* file;
		bool isEmpty;

	private:
		T* buffer;
		int numBufferItems;
		int offset;
		int readCnt;
	};

	template <class T>
	class BufferedWriter
	{
	public:
		BufferedWriter(FILE* file, int bufferSize) : file(file), numBufferItems(bufferSize / sizeof(T)) {
			buffer = new T[numBufferItems];
			offset = 0;
		}

		BufferedWriter(const char* filename, const char* mode, int bufferSize) : numBufferItems(bufferSize / sizeof(T)) {
			file = fopen(filename, mode);// error handling here
			buffer = new T[numBufferItems];
			offset = 0;
		}

		~BufferedWriter() {
			if (file != NULL) {
				fclose(file);
			}
			delete[] buffer;
		}

		void put(const T& t) {
			buffer[offset++] = t;
			if (offset == numBufferItems) {
				flush();
			}
		}

		void flush() {
			fwrite(buffer, sizeof(T), offset, file);
			offset = 0;
		}

		void reset() {
			rewind(file);
			offset = 0;
		}

		void close() {
			flush();
			fclose(file);
			file = NULL;
		}

	public:
		FILE* file;

	private:
		T* buffer;
		int numBufferItems;
		int offset;
	};

	template <class T>
	class InverseBufferedReader
	{
	public:
		InverseBufferedReader(FILE* file, int bufferSize) : file(file), numBufferItems(bufferSize / sizeof(T)) {
			buffer = new T[numBufferItems];

			fseek(file, 0L, SEEK_END);
			fileOffset = ftell(file) / sizeof(T);

			fseek(file, std::max(fileOffset - numBufferItems, 0) * sizeof(T), SEEK_SET);
			bufferOffset = fread(buffer, sizeof(T), numBufferItems, file);// >0

			isEmpty = false;
		}

		InverseBufferedReader(const char* filename, const char* mode, int bufferSize) : numBufferItems(bufferSize / sizeof(T)) {
			file = fopen(filename, mode);

			buffer = new T[numBufferItems];

			fseek(file, 0L, SEEK_END);
			fileOffset = ftell(file) / sizeof(T);

			fseek(file, std::max(fileOffset - numBufferItems, 0) * sizeof(T), SEEK_SET);
			bufferOffset = fread(buffer, sizeof(T), numBufferItems, file);// >0

			isEmpty = false;
		}

		~InverseBufferedReader() {
			if (file != NULL) {
				fclose(file);
			}
			delete[] buffer;
		}

		const T& get() {
			if (isEmpty) {
				return buffer[0];
			}
			return buffer[bufferOffset - 1];
		}

		bool seekNext() {
			if (isEmpty) {
				return false;
			}
			--fileOffset;
			--bufferOffset;
			if (bufferOffset == 0) {
				if (fileOffset == 0) {
					isEmpty = true;
				}
				else {
					fseek(file, std::max(fileOffset - numBufferItems, 0) * sizeof(T), SEEK_SET);
					bufferOffset = fread(buffer, sizeof(T), std::min(fileOffset, numBufferItems), file);
				}
			}
			return !isEmpty;
		}

		void close() {
			fclose(file);
			file = NULL;
		}

	public:
		FILE* file;
		bool isEmpty;

	private:
		T* buffer;
		int numBufferItems;
		int fileOffset;
		int bufferOffset;
	};

	void outputSeq(const char* filename, FILE* os) {
		FILE* is = fopen(filename, "rb");
		fseek(is, 0L, SEEK_END);
		size_t size = ftell(is);
		rewind(is);
		uint8_t item;
		for (int i = 0; i < size / sizeof(uint8_t); ++i) {
			fread(&item, sizeof(uint8_t), 1, is);
			fprintf(os, "%" PRIu8, item);
		}
		fclose(is);
	}

}

int main(int argc, char* argv[])
{
	//genInput(INPUT_FILENAME, 7, 5);
	//outputInput(INPUT_FILENAME, stdout);
	
	const int DEFAULT_BLOCK_SIZE = 32 * 1024;//2;

	FILE* is = fopen(INPUT_FILENAME, "rb");

	uint32_t n, m;

	fread(&n, sizeof(uint32_t), 1, is);
	BufferedReader<uint8_t> inputReader(is, DEFAULT_BLOCK_SIZE);
	BufferedWriter<uint8_t> nWriter("n", "wb", DEFAULT_BLOCK_SIZE);
	for (int i = 0; i < n; ++i) {
		nWriter.put(inputReader.get());
		inputReader.seekNext();
	}
	nWriter.close();
	
	BufferedWriter<uint8_t> mWriter("m", "wb", DEFAULT_BLOCK_SIZE);
	fseek(is, sizeof(uint32_t) + n * sizeof(uint8_t), SEEK_SET);
	fread(&m, sizeof(uint32_t), 1, is);
	inputReader.reload();
	for (int i = 0; i < m; ++i) {
		mWriter.put(inputReader.get());
		inputReader.seekNext();
	}
	mWriter.close();
	inputReader.close();

	//printf("\n");
	//outputSeq("n", stdout);
	//printf("\n");
	//outputSeq("m", stdout);

	//printf("\n");
	InverseBufferedReader<uint8_t> nReader("n", "rb", DEFAULT_BLOCK_SIZE);
	InverseBufferedReader<uint8_t> mReader("m", "rb", DEFAULT_BLOCK_SIZE);

	BufferedWriter<uint8_t> tmpWriter("tmp", "wb", DEFAULT_BLOCK_SIZE);

	uint8_t one = 0;
	do {
		uint8_t dig = nReader.isEmpty ? 0 : nReader.get();
		nReader.seekNext();
		uint8_t dig2 = mReader.isEmpty ? 0 : mReader.get();
		mReader.seekNext();
		uint8_t dig3r = (dig + dig2 + one) % 10;
		uint8_t dig3q = (dig + dig2 + one) / 10;
		one = dig3q;

		tmpWriter.put(dig3r);

		//printf("%d + %d = %d (%d)\n", dig, dig2, dig3r, one);

	} while (!nReader.isEmpty || !mReader.isEmpty);
	if (one) {
		tmpWriter.put(one);
	}
	nReader.close();
	mReader.close();
	tmpWriter.close();

	FILE* tmp = fopen("tmp", "rb");
	fseek(tmp, 0, SEEK_END);
	uint32_t k = ftell(tmp) / sizeof(uint8_t);
	rewind(tmp);
	//printf("%d\n", k);

	FILE* result = fopen(OUTPUT_FILENAME, "wb");
	fwrite(&k, sizeof(uint32_t), 1, result);

	InverseBufferedReader<uint8_t> tmpReader(tmp, DEFAULT_BLOCK_SIZE);
	BufferedWriter<uint8_t> resultWriter(result, DEFAULT_BLOCK_SIZE);

	do {
		//printf("%d\n", tmpReader.get());
		resultWriter.put(tmpReader.get());
	} while (tmpReader.seekNext());
	tmpReader.close();
	resultWriter.close();

	//printf("\n");
	//outputRes(OUTPUT_FILENAME, stdout);

	return 0;
}
