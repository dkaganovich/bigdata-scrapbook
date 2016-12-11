#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <algorithm>

namespace {

	const char *INPUT_FILENAME = "input.bin";
	const char *OUTPUT_FILENAME = "output.bin";

	void generateSeq(const char* filename, uint64_t N) {
		FILE* os = fopen(filename, "wb");
		if (os != NULL) {
			fwrite(&N, sizeof(uint64_t), 1, os);
			srand(time(NULL));
			uint64_t item;
			for (uint64_t i = 0; i < N; ++i) {
				item = rand() % UINT64_MAX;
				fwrite(&item, sizeof(uint64_t), 1, os);
			}
			fclose(os);
		}
	}

	void outputSeq(const char* filename, FILE* os) {
		FILE* is = fopen(filename, "rb");
		fseek(is, 0L, SEEK_END);
		size_t size = ftell(is);
		rewind(is);
		uint64_t item;
		for (uint64_t i = 0; i < size / sizeof(uint64_t); ++i) {
			fread(&item, sizeof(uint64_t), 1, is);
			fprintf(os, "%" PRIu64 "\n", item);
		}
	}

	int compare_ull(const void* a, const void* b) {
		double aa = *(const uint64_t*)a;
		double bb = *(const uint64_t*)b;
		return (aa < bb) ? -1 : (aa == bb) ? 0 : 1;
	}

	int merge_group(int numSplits, FILE** splitFiles, FILE* output, int numBlockItems) // merge a single group
	{
		int* heads = new int[numSplits];
		int* reads = new int[numSplits];
		uint64_t* blocks = new uint64_t[numSplits * numBlockItems];
		for (int i = 0; i < numSplits; ++i) {
			reads[i] = fread(blocks + i * numBlockItems, sizeof(uint64_t), numBlockItems, splitFiles[i]);
			heads[i] = 0;
		}

		uint64_t* result = new uint64_t[numBlockItems];

		int done = 0;
		int resultHead = 0;
		while (done < numSplits) 
		{
			uint64_t min;
			int minpos;
			for (int i = 0; i < numSplits; ++i) {
				if (heads[i] != -1) {// at least one exists until done
					min = blocks[i * numBlockItems + heads[i]];
					minpos = i;
					break;
				}
			}

			for (int i = 0; i < numSplits; ++i) {
				if (heads[i] == -1) {
					continue;
				}
				if (min > blocks[i * numBlockItems + heads[i]]) {
					min = blocks[i * numBlockItems + heads[i]];
					minpos = i;
				}
			}

			++heads[minpos];
			if (heads[minpos] == reads[minpos]) {
				if (reads[minpos] < numBlockItems) {
					++done;
					heads[minpos] = -1;// signal eof
				}
				else {
					reads[minpos] = fread(blocks + minpos * numBlockItems, sizeof(uint64_t), numBlockItems, splitFiles[minpos]);
					if (reads[minpos] == 0) {
						++done;
						heads[minpos] = -1;// signal eof
					}
					else {
						heads[minpos] = 0;
					}
				}
			}

			result[resultHead++] = min;
			if (done == numSplits || resultHead == numBlockItems) {
				fwrite(result, sizeof(uint64_t), resultHead, output);
				resultHead = 0;
			}
		}
		delete[] heads;
		delete[] reads;
		delete[] blocks;
		delete[] result;

		return 0;
	}

	int merge(int numSplits, int numWays, int numBlockItems, int beg) 
	{
		if (numSplits == 1) {
			return 0;
		}
		int numGroups = (int)std::ceil((double)numSplits / numWays);
		char* filename = new char[10];
		int prebeg = 0;
		for (int i = 0; i < numGroups; ++i) 
		{
			int numGroupSplits = (int)std::min(numWays, numSplits - i * numWays);
			if (numGroupSplits == 1) {
				prebeg = 1;
				break;
			}
			FILE** splitFiles = new FILE*[numGroupSplits];
			for (int j = 0; j < numGroupSplits; ++j) {
				itoa(beg + i * numWays + j, filename, 10);
				splitFiles[j] = fopen(filename, "rb");
				if (splitFiles[j] == NULL) {
					fprintf(stderr, "I/O failure: merge");
					return 1;
				}
			}

			FILE* result;
			if (numSplits <= numWays) {
				result = fopen(OUTPUT_FILENAME, "ab");
			}
			else {
				itoa(beg + numSplits + i, filename, 10);
				result = fopen(filename, "wb");
			}
			if (result == NULL) {
				fprintf(stderr, "I/O failure: merge");
				return 1;
			}

			merge_group(numGroupSplits, splitFiles, result, numBlockItems);

			for (int j = 0; j < numGroupSplits; ++j) {
				fclose(splitFiles[j]);
			}
			delete[] splitFiles;
			fclose(result);
		}
		delete[] filename;

		return merge(numGroups, numWays, numBlockItems, beg + numSplits - prebeg);
	}

	int splitup(int numItems, int numSplitItems, int* numSplits) 
	{
		FILE* input = fopen(INPUT_FILENAME, "rb");
		if (input == NULL) {
			fprintf(stderr, "I/O failure: input");
			return 1;
		}
		fseek(input, sizeof(uint64_t), SEEK_SET);

		uint64_t* split = new uint64_t[numSplitItems];
		*numSplits = (int)std::ceil((double)numItems / numSplitItems);

		char* filename = new char[10];
		for (int i = 0; i < *numSplits; ++i) {
			int readCnt = fread(split, sizeof(uint64_t), numSplitItems, input);
			std::sort(split, split + readCnt);
			//qsort(split, readCnt, sizeof(uint64_t), compare_ull);
			FILE* result;
			if (numItems <= numSplitItems) {// single write for the whole file
				result = fopen(OUTPUT_FILENAME, "ab");
			}
			else {
				itoa(i, filename, 10);
				result = fopen(filename, "wb");
			}
			if (result == NULL) {
				fprintf(stderr, "I/O failure: splitup");
				return 1;
			}
			fwrite(split, sizeof(uint64_t), readCnt, result);
			fclose(result);
		}
		delete[] filename;
		delete[] split;
		fclose(input);

		return 0;
	}
}

int main(int argc, char* argv[])
{
	//generateSeq(INPUT_FILENAME, 1238731);//1280000);
	//FILE* outputTxt = fopen("input.txt", "w");
	//outputSeq(INPUT_FILENAME, outputTxt);
	//fclose(outputTxt);

	const int TOTAL_MEMORY = 512 * 1024;//10240008
	const int BLOCK_SIZE = 128 * 1024;

	const int SPLIT_SIZE = TOTAL_MEMORY;
	const int NUM_WAYS = TOTAL_MEMORY / BLOCK_SIZE - 1;// TOTAL_MEMORY > 2 * BLOCK_SIZE

	FILE* input = fopen(INPUT_FILENAME, "rb");
	if (input == NULL) {
		fprintf(stderr, "I/O failure: input");
		return 1;
	}
	int N;
	fread(&N, sizeof(uint64_t), 1, input);
	fclose(input);

	FILE* output = fopen(OUTPUT_FILENAME, "wb");
	if (output == NULL) {
		fprintf(stderr, "I/O failure: output");
		return 1;
	}
	fwrite(&N, sizeof(uint64_t), 1, output);
	fclose(output);

	//clock_t start = clock();

	// generate sorted splits
	int numSplits;
	splitup(N, SPLIT_SIZE / sizeof(uint64_t), &numSplits);
	fclose(input);

	//clock_t fin = clock();
	//fprintf(stdout, "Time split: %f\n", float(fin - start) / CLOCKS_PER_SEC);

	// merge splits
	merge(numSplits, NUM_WAYS, BLOCK_SIZE / sizeof(uint64_t), 0);

	//fprintf(stdout, "Time merge: %f\n", float(clock() - fin) / CLOCKS_PER_SEC);
	//fprintf(stdout, "Time elapsed: %f\n", float(clock() - start) / CLOCKS_PER_SEC);


	/*outputTxt = fopen("output.txt", "w");
	outputSeq(OUTPUT_FILENAME, outputTxt);
	fclose(outputTxt);*/

	return 0;
}