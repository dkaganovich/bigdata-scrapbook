#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>
#include <vector>
#include <algorithm>

//#define NDEBUG

namespace {

	const char *INPUT_FILENAME = "input.bin";
	const char *OUTPUT_FILENAME = "output.bin";

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
	struct Tuple2 {
		T left;
		T right;
	};

	template <class T>
	struct Tuple3 {
		T left;
		T middle;
		T right;
	};

	template <class T>
	bool tuple2Lcmp(Tuple2<T> a, Tuple2<T> b) {
		return a.left < b.left;
	}

	template <class T>
	bool tuple2Rcmp(Tuple2<T> a, Tuple2<T> b) {
		return a.right < b.right;
	}

	template <class T>
	bool tuple3Lcmp(Tuple3<T> a, Tuple3<T> b) {
		return a.left < b.left;
	}

	template <class T>
	bool tuple3Mcmp(Tuple3<T> a, Tuple3<T> b) {
		return a.middle < b.middle;
	}

	template <class T>
	bool tuple3Rcmp(Tuple3<T> a, Tuple3<T> b) {
		return a.right < b.right;
	}

	void genInput(const char* filename, int N) {
		FILE* os = fopen(filename, "wb");
		if (os != NULL) {
			fwrite(&N, 1, sizeof(uint32_t), os);
			Tuple2<uint32_t> item;
			for (int i = 0; i < N; ++i) {
				item.left = i;
				if (i == N - 1) {
					item.right = 0;
				}
				else {
					item.right = i + 1;
				}
				fwrite(&item, sizeof(Tuple2<uint32_t>), 1, os);
			}
			/*N = 5;
			fwrite(&N, 1, sizeof(uint32_t), os);
			Tuple2<uint32_t> item = { 4, 5 };
			fwrite(&item, sizeof(Tuple2<uint32_t>), 1, os);
			item = { 5, 1 };
			fwrite(&item, sizeof(Tuple2<uint32_t>), 1, os);
			item = { 1, 2 };
			fwrite(&item, sizeof(Tuple2<uint32_t>), 1, os);
			item = { 3, 4 };
			fwrite(&item, sizeof(Tuple2<uint32_t>), 1, os);
			item = { 2, 3 };
			fwrite(&item, sizeof(Tuple2<uint32_t>), 1, os);*/
			fclose(os);
		}
	}

	void outputSeq(const char* filename, FILE* os) {
		FILE* is = fopen(filename, "rb");
		fseek(is, 0L, SEEK_END);
		size_t size = ftell(is);
		rewind(is);
		uint32_t item;
		for (int i = 0; i < size / sizeof(uint32_t); ++i) {
			fread(&item, sizeof(uint32_t), 1, is);
			fprintf(os, "%" PRIu32 "\n", item);
		}
	}

	void outputTuple2(const char* filename, FILE* os) {
		FILE* is = fopen(filename, "rb");
		fseek(is, 0L, SEEK_END);
		size_t size = ftell(is);
		rewind(is);
		Tuple2<uint32_t> item;
		for (int i = 0; i < size / sizeof(Tuple2<uint32_t>); ++i) {
			fread(&item, sizeof(Tuple2<uint32_t>), 1, is);
			fprintf(os, "%" PRIu32 " %" PRIu32 "\n", item.left, item.right);
		}
	}

	template <class T>
	int mergeGroup(int numSplits, FILE** splitFiles, FILE* tgtFile, int blockSize, bool(*cmpfunc)(T, T)) // merge a single group
	{
		std::vector<BufferedReader<T>* > splitReaders;
		for (int i = 0; i < numSplits; ++i) {
			splitReaders.push_back(new BufferedReader<T>(splitFiles[i], blockSize));
		}

		BufferedWriter<T> tgtWriter(tgtFile, blockSize);

		int doneSplits = 0;
		while (doneSplits < numSplits)
		{
			T min;
			int minpos;
			for (int i = 0; i < numSplits; ++i) {
				if (!splitReaders[i]->isEmpty) {
					min = splitReaders[i]->get();
					minpos = i;
					break;
				}
			}

			for (int i = 0; i < numSplits; ++i) {
				if (splitReaders[i]->isEmpty) {
					continue;
				}
				if ((*cmpfunc)(splitReaders[i]->get(), min)) {
					min = splitReaders[i]->get();
					minpos = i;
				}
			}

			if (!splitReaders[minpos]->seekNext()) {
				++doneSplits;
			}

			tgtWriter.put(min);
		}
		tgtWriter.close();

		for (int i = 0; i < numSplits; ++i) {
			splitReaders[i]->close();
			delete splitReaders[i];
		}

		return 0;
	}

	template <class T>
	int merge(const char* tgtFilename, int numSplits, int numWays, int blockSize, int initSplitId, bool(*cmpfunc)(T, T))
	{
		if (numSplits == 1) {
			return 0;
		}

		int numGroups = (int)std::ceil((double)numSplits / numWays);
		char* splitFilename = new char[16];
		int splitIdGap = 0;

		for (int i = 0; i < numGroups; ++i)
		{
			int numGroupSplits = (int)std::min(numWays, numSplits - i * numWays);
			if (numGroupSplits == 1) {// last group with a single split - no need to merge
				splitIdGap = 1;
				break;
			}

			FILE** splitFiles = new FILE*[numGroupSplits];
			for (int j = 0; j < numGroupSplits; ++j) {
				sprintf(splitFilename, "part%d", initSplitId + i * numWays + j);
				splitFiles[j] = fopen(splitFilename, "rb");
			}

			FILE* tgtFile;
			if (numSplits <= numWays) {
				tgtFile = fopen(tgtFilename, "wb");
			}
			else {
				sprintf(splitFilename, "part%d", initSplitId + numSplits + i);
				tgtFile = fopen(splitFilename, "wb");
			}

			mergeGroup<T>(numGroupSplits, splitFiles, tgtFile, blockSize, cmpfunc);
		}
		delete[] splitFilename;

		return merge<T>(tgtFilename, numGroups, numWays, blockSize, initSplitId + numSplits - splitIdGap, cmpfunc);
	}

	template <class T>
	int split(const char* filename, const char* tgtFilename, int fileSize, int splitSize, bool(*cmpfunc)(T, T), int* numSplits)
	{
		FILE* file = fopen(filename, "rb");

		int numItems = fileSize / sizeof(T);
		int numSplitItems = splitSize / sizeof(T);

		*numSplits = (int)std::ceil((double)fileSize / splitSize);

		T* splitBuf = new T[numSplitItems];
		char* splitFilename = new char[16];

		for (int i = 0; i < *numSplits; ++i) 
		{
			int readCnt = fread(splitBuf, sizeof(T), numSplitItems, file);
			std::sort(splitBuf, splitBuf + readCnt, cmpfunc);

			FILE* splitFile;
			if (fileSize <= splitSize) {// single write for the whole file
				splitFile = fopen(tgtFilename, "wb");
			}
			else {
				sprintf(splitFilename, "part%d", i);
				splitFile = fopen(splitFilename, "wb");
			}

			fwrite(splitBuf, sizeof(T), readCnt, splitFile);
			fclose(splitFile);
		}

		delete[] splitFilename;
		delete[] splitBuf;
		fclose(file);
		return 0;
	}
}

// utility method
int calcFileSize(const char* filename) {
	FILE* file = fopen(filename, "rb");
	fseek(file, 0L, SEEK_END);
	int fileSize = ftell(file);
	fclose(file);
	return fileSize;
}

template <class T>
int mergesort(const char* filename, const char* tgtFilename, int totalMemory/*B*/, int blockSize/*B*/, bool (*cmpfunc)(T, T)) 
{
	/*assert(totalMemory > 2 * blockSize);
	assert(totalMemory % sizeof(T) == 0);
	assert(blockSize % sizeof(T) == 0);*/

	int fileSize = calcFileSize(filename);
	//assert(fileSize > 0 && fileSize % sizeof(T) == 0);

	int numSplits;
	if (split<T>(filename, tgtFilename, fileSize, totalMemory, cmpfunc, &numSplits) == 1) {
		fprintf(stderr, "split failure:(");
		return 1;
	}
	
	int numWays = totalMemory / blockSize - 1;// k-ways merge
	assert(numWays > 1);
	if (merge<T>(tgtFilename, numSplits, numWays, blockSize, 0, cmpfunc) == 1) {
		fprintf(stderr, "merge failure:(");
		return 1;
	}

	return 0;
}

// writeout (u, 1)
template <class T>
int initWeights(const char* srcFilename, const char* tgtWeightFilename, int totalMemory, int blockSize) 
{
	mergesort<Tuple2<T> >(srcFilename, "tmp0", totalMemory, blockSize, tuple2Lcmp<T>);
	BufferedReader<Tuple2<T> > leftSortedReader("tmp0", "rb", blockSize);// (/u, n(u))

	BufferedWriter<Tuple2<T> > weightWriter(tgtWeightFilename, "wb", blockSize);

	do {
		Tuple2<T> t = { leftSortedReader.get().left, 1 };
		weightWriter.put(t);
	} while (leftSortedReader.seekNext());
	leftSortedReader.close();
	weightWriter.close();

	return 0;
}

// calculate independent set
// writeout (u, n(u)) : n(u) <- independent set
template <class T>// col type
int calcIndependentSet(const char* srcFilename, const char* tgtIndsetFilename, int totalMemory/*B*/, int blockSize/*B*/) 
{
	// flip a coin for each node
	mergesort<Tuple2<T> >(srcFilename, "tmp0", totalMemory, blockSize, tuple2Lcmp<T>);
	BufferedReader<Tuple2<T> > leftSortedReader("tmp0", "rb", blockSize);

	BufferedWriter<Tuple2<T> > coinWriter("tmp1", "wb", blockSize);

	srand(time(NULL));
	do {
		Tuple2<T> t = { leftSortedReader.get().left, rand() % 2 };// (/u, {0|1})
		coinWriter.put(t);
	} while (leftSortedReader.seekNext());
	coinWriter.close();

	// filter (u, n(u)) against weight(u) = 0
	leftSortedReader.reset();
	BufferedReader<Tuple2<T> > coinReader("tmp1", "rb", blockSize);

	BufferedWriter<Tuple2<T> > join0x1Writer("tmp2", "wb", blockSize);

	do {
		if (coinReader.get().right == 0) {
			join0x1Writer.put(leftSortedReader.get());
		}
	} while (leftSortedReader.seekNext() && coinReader.seekNext());
	leftSortedReader.close();
	join0x1Writer.close();

	// filter (u, n(u)) gotten against weight(n(u)) = 1
	mergesort<Tuple2<T> >("tmp2", "tmp3", totalMemory, blockSize, tuple2Rcmp<T>);
	BufferedReader<Tuple2<T> > join0x1Reader("tmp3", "rb", blockSize);
	coinReader.reset();

	BufferedWriter<Tuple2<T> > indsetWriter(tgtIndsetFilename, "wb", blockSize);

	int indsetCnt = 0;
	do {
		Tuple2<T> t = join0x1Reader.get();
		do {
			if (t.right == coinReader.get().left) {
				if (coinReader.get().right == 1) {
					indsetWriter.put(t);
					++indsetCnt;
				}
				break;
			}
		} while (t.right >= coinReader.get().left && coinReader.seekNext());
	} while (join0x1Reader.seekNext());
	coinReader.close();
	join0x1Reader.close();
	indsetWriter.close();

	return indsetCnt;
}

template <class T>
int dropNodes(const char* srcFilename, const char* srcIndsetFilename, const char* srcWeightFilename, const char* tgtFilename, const char* tgtWeightFilename, int totalMemory/*B*/, int blockSize/*B*/)
{
	// filter (u, n(u)) against u </- independent  set 
	mergesort<Tuple2<T> >(srcFilename, "tmp0", totalMemory, blockSize, tuple2Lcmp<T>);
	BufferedReader<Tuple2<T> > leftSortedReader("tmp0", "rb", blockSize);
	mergesort<Tuple2<T> >(srcIndsetFilename, "tmp1", totalMemory, blockSize, tuple2Rcmp<T>);
	BufferedReader<Tuple2<T> > indsetReader("tmp1", "rb", blockSize);

	BufferedWriter<Tuple2<T> > join0x1Writer("tmp2", "wb", blockSize);
	
	do {
		Tuple2<T> t = leftSortedReader.get();
		bool drop = false;
		do {
			if (t.left == indsetReader.get().right) {
				drop = true;
				break;
			}
		} while (t.left >= indsetReader.get().right && indsetReader.seekNext());
		if (!drop) {
			join0x1Writer.put(t);
		}
	} while (leftSortedReader.seekNext());
	join0x1Writer.close();

	// filter (u, n(u)) gotten against n(u) </- independent  set
	mergesort<Tuple2<T> >("tmp2", "tmp3", totalMemory, blockSize, tuple2Rcmp<T>);
	BufferedReader<Tuple2<T> > join0x1Reader("tmp3", "rb", blockSize);
	indsetReader.reset();

	BufferedWriter<Tuple2<T> > edgeWriter(tgtFilename, "wb", blockSize);

	do {
		Tuple2<T> t = join0x1Reader.get();
		bool drop = false;
		do {
			if (t.right == indsetReader.get().right) {
				drop = true;
				break;
			}
		} while (t.right >= indsetReader.get().right && indsetReader.seekNext());
		if (!drop) {
			edgeWriter.put(t);
		}
	} while (join0x1Reader.seekNext());
	join0x1Reader.close();

	// collapse edges + calc weights supplement
	leftSortedReader.reset();
	mergesort<Tuple2<T> >(srcFilename, "tmp4", totalMemory, blockSize, tuple2Rcmp<T>);
	BufferedReader<Tuple2<T> > rightSortedReader("tmp4", "rb", blockSize);
	indsetReader.reset();
	BufferedReader<Tuple2<T> > weightReader(srcWeightFilename, "rb", blockSize);

	BufferedWriter<Tuple2<T> > supplWriter("tmp5", "wb", blockSize);

	do {
		Tuple2<T> t_ = rightSortedReader.get();
		Tuple2<T> _t = leftSortedReader.get();
		do {
			if (t_.right == indsetReader.get().right && _t.left == indsetReader.get().right) {
				Tuple2<T> t = { t_.left, _t.right };
				edgeWriter.put(t);
				do {
					if (indsetReader.get().right == weightReader.get().left) {
						Tuple2<T> t2 = { t.left, weightReader.get().right };
						supplWriter.put(t2);
						break;
					}
				} while (indsetReader.get().right >= weightReader.get().left && weightReader.seekNext());
				break;
			}
		} while (t_.right >= indsetReader.get().right && indsetReader.seekNext());
	} while (leftSortedReader.seekNext() && rightSortedReader.seekNext());
	leftSortedReader.close();
	rightSortedReader.close();
	indsetReader.close();
	supplWriter.close();
	edgeWriter.close();

	// adjust weights
	weightReader.reset();
	mergesort<Tuple2<T> >("tmp5", "tmp6", totalMemory, blockSize, tuple2Lcmp<T>);
	BufferedReader<Tuple2<T> > supplReader("tmp6", "rb", blockSize);

	BufferedWriter<Tuple2<T> > weightWriter(tgtWeightFilename, "wb", blockSize);

	do {
		Tuple2<T> t = weightReader.get();
		bool match = false;
		do {
			if (t.left == supplReader.get().left) {
				Tuple2<T> t2 = { t.left, t.right + supplReader.get().right };
				weightWriter.put(t2);
				match = true;
				break;
			}
		} while (t.left >= supplReader.get().left && supplReader.seekNext());
		if (!match) {
			weightWriter.put(t);
		}
	} while (weightReader.seekNext());
	weightReader.close();
	weightWriter.close();
	supplReader.close();

	return 0;
}

template <class T>
int doRank(const char* srcFilename, const char* srcWeightFilename, const char* tgtFilename, int totalMemory/*B*/, int blockSize/*B*/)
{
	// calc number of items to be processed
	FILE* file = fopen(srcFilename, "rb");
	fseek(file, 0L, SEEK_END);
	int fileSize = ftell(file);
	fclose(file);
	
	int numItems = fileSize / sizeof(Tuple2<T>);

	// join (u, n(u)) pairs with weight(u)
	mergesort<Tuple2<T> >(srcFilename, "tmp0", totalMemory, blockSize, tuple2Lcmp<T>);
	BufferedReader<Tuple2<T> > leftSortedReader("tmp0", "rb", blockSize);

	BufferedReader<Tuple2<T> > weightReader(srcWeightFilename, "rb", blockSize);

	Tuple3<T>* triples = new Tuple3<T>[numItems];
	int i = 0;
	do {
		Tuple2<T> t = leftSortedReader.get();
		do {
			if (t.left == weightReader.get().left) {
				triples[i++] = { t.left, t.right, weightReader.get().right };
				break;
			}
		} while (t.left >= weightReader.get().left && weightReader.seekNext());
	} while (leftSortedReader.seekNext());
	leftSortedReader.close();
	weightReader.close();

	// do ranking
	Tuple2<T>* result = new Tuple2<T>[numItems];
	result[0] = { triples[0].left, triples[0].right };// node + weight

	i = 0;
	T curr = triples[0].middle;
	while (curr != result[0].left) {
		for (int j = 0; j < numItems; ++j) {
			if (curr == triples[j].left) {
				result[++i] = { curr, triples[j].right };
				curr = triples[j].middle;
				break;
			}
		}
	}
	delete[] triples;

	// writeout (u, rank)
	BufferedWriter<Tuple2<T> > tgtWriter(tgtFilename, "wb", fileSize);

	int weight = 0;
	for (int i = 0; i < numItems; ++i) {
		Tuple2<T> t = { result[i].left, weight };
		weight += result[i].right;
		tgtWriter.put(t);
	}
	tgtWriter.close();
	delete[] result;

	return 0;
}

template <class T>
int recoverNodes(const char* srcFilename, const char* srcIndsetFilename, const char* srcWeightFilename, const char* tgtFilename, int totalMemory/*B*/, int blockSize/*B*/)
{
	// join (u, n(u)) pairs with weight(u)
	mergesort<Tuple2<T> >(srcFilename, "tmp0", totalMemory, blockSize, tuple2Lcmp<T>);
	BufferedReader<Tuple2<T> > leftSortedReader("tmp0", "rb", blockSize);
	BufferedReader<Tuple2<T> > weightReader(srcWeightFilename, "rb", blockSize);

	BufferedWriter<Tuple3<T> > join0WeightWriter("tmp1", "wb", blockSize);

	do {
		Tuple2<T> t = leftSortedReader.get();
		do {
			if (t.left == weightReader.get().left) {
				Tuple3<T> t2 = { t.left, t.right, weightReader.get().right };
				join0WeightWriter.put(t2);
				break;
			}
		} while (t.left >= weightReader.get().left && weightReader.seekNext());
	} while (leftSortedReader.seekNext());
	leftSortedReader.close();
	join0WeightWriter.close();

	// join dropped (u, n(u)) pairs with weight(n(u))
	mergesort<Tuple2<T> >(srcIndsetFilename, "tmp2", totalMemory, blockSize, tuple2Rcmp<T>);
	BufferedReader<Tuple2<T> > indsetReader("tmp2", "rb", blockSize);
	weightReader.reset();

	BufferedWriter<Tuple3<T> > join2WeightWriter("tmp3", "wb", blockSize);

	do {
		Tuple2<T> t = indsetReader.get();
		do {
			if (t.right == weightReader.get().left) {
				Tuple3<T> t2 = { t.left, t.right, weightReader.get().right };
				join2WeightWriter.put(t2);
				break;
			}
		} while (t.right >= weightReader.get().left && weightReader.seekNext());
	} while (indsetReader.seekNext());
	indsetReader.close();
	weightReader.close();
	join2WeightWriter.close();

	// recover nodes
	mergesort<Tuple3<T> >("tmp3", "tmp4", totalMemory, blockSize, tuple3Lcmp<T>);
	BufferedReader<Tuple3<T> > join2WeightReader("tmp4", "rb", blockSize);
	BufferedReader<Tuple3<T> > join0WeightReader("tmp1", "rb", blockSize);
	BufferedWriter<Tuple2<T> > tgtWriter(tgtFilename, "wb", blockSize);

	do {
		Tuple3<T> t = join0WeightReader.get();
		Tuple2<T> t2 = { t.left, t.middle };
		tgtWriter.put(t2);
		do {
			if (t.left == join2WeightReader.get().left) {
				// rank(u) = rank(p(u)) + weight(p(u)) - weight(u)
				Tuple2<T> t3 = { join2WeightReader.get().middle, t.middle + t.right - join2WeightReader.get().right };
				tgtWriter.put(t3);
				break;
			}
		} while (t.left >= join2WeightReader.get().left && join2WeightReader.seekNext());
	} while (join0WeightReader.seekNext());
	join0WeightReader.close();
	join2WeightReader.close();
	tgtWriter.close();

	return 0;
}

int main(int argc, char* argv[])
{
	const int DEFAULT_BLOCK_SIZE = 16 * 1024;//64 * 1024;// B
	const int DEFAULT_TOTAL_MEMORY = 64 * 1024;//256 * 1024;// B

	//genInput("input.bin", 1250000);
	
	FILE* is = fopen("input.bin", "rb");
	int N;
	fread(&N, 1, sizeof(uint32_t), is);
	BufferedReader<Tuple2<uint32_t> > inputReader(is, DEFAULT_BLOCK_SIZE);
	BufferedWriter<Tuple2<uint32_t> > inputWriter("input0", "wb", DEFAULT_BLOCK_SIZE);
	for (int i = 0; i < N; ++i) {
		inputWriter.put(inputReader.get());
		inputReader.seekNext();
	}
	inputReader.close();
	inputWriter.close();

	//
	/*outputTuple2("input0", stdout);
	FILE* outputTxt = fopen("origin.txt", "w");
	outputTuple2("input0", outputTxt);
	fclose(outputTxt);*/
	//

	int fileSize = calcFileSize("input0");
	initWeights<uint32_t>("input0", "weight0", DEFAULT_TOTAL_MEMORY, DEFAULT_BLOCK_SIZE);

	char* indsetFilename = new char[16];

	char* srcWeightFilename = new char[16];
	char* tgtWeightFilename = new char[16];
	strcpy(tgtWeightFilename, "weight0");

	//clock_t start = clock();

	int cnt = 1;
	while (fileSize > 128 * 1024) {
		sprintf(indsetFilename, "%s%d", "indset", cnt);
		strcpy(srcWeightFilename, tgtWeightFilename);
		sprintf(tgtWeightFilename, "%s%d", "weight", cnt);

		int indsetCnt;
		while (true) {
			indsetCnt = calcIndependentSet<uint32_t>(cnt % 2 == 0 ? "input1" : "input0", indsetFilename, DEFAULT_TOTAL_MEMORY, DEFAULT_BLOCK_SIZE);
			if (indsetCnt > 0) {
				break;
			}
		};

		/*printf("\INDSET\n");
		outputTuple2(indsetFilename, stdout);*/

		dropNodes<uint32_t>(cnt % 2 == 0 ? "input1" : "input0", indsetFilename, srcWeightFilename, 
			cnt % 2 == 0 ? "input0" : "input1", tgtWeightFilename, DEFAULT_TOTAL_MEMORY, DEFAULT_BLOCK_SIZE);

		/*printf("\INPUT\n");
		outputTuple2(cnt % 2 == 0 ? "input0" : "input1", stdout);

		printf("\WEIGHT\n");
		outputTuple2(tgtWeightFilename, stdout);*/

		fileSize -= indsetCnt * sizeof(uint32_t) * 2;

		//printf("\nFILESIZE\n%d\n", fileSize);

		++cnt;
	}

	//fprintf(stdout, "Top down: %f\n", float(clock() - start) / CLOCKS_PER_SEC);

	//
	//start = clock();

	doRank<uint32_t>(cnt % 2 == 0 ? "input1" : "input0", tgtWeightFilename, 
		cnt % 2 == 0 ? "output1" : "output0", DEFAULT_TOTAL_MEMORY, DEFAULT_BLOCK_SIZE);

	//fprintf(stdout, "Ranking: %f\n", float(clock() - start) / CLOCKS_PER_SEC);

	/*printf("\nRANKING\n");
	outputTuple2(cnt % 2 == 0 ? "output1" : "output0", stdout);*/

	//
	//start = clock();

	while (--cnt > 0) {
		sprintf(indsetFilename, "%s%d", "indset", cnt);
		sprintf(srcWeightFilename, "%s%d", "weight", cnt);
		recoverNodes<uint32_t>(cnt % 2 == 0 ? "output0" : "output1", indsetFilename, srcWeightFilename,
			cnt % 2 == 0 ? "output1" : "output0", DEFAULT_TOTAL_MEMORY, DEFAULT_BLOCK_SIZE);

		/*printf("\RECOVERED\n");
		outputTuple2(cnt % 2 == 0 ? "output1" : "output0", stdout);*/
	}

	//fprintf(stdout, "Bottom up: %f\n", float(clock() - start) / CLOCKS_PER_SEC);

	// writeout result
	mergesort<Tuple2<uint32_t> >(cnt % 2 == 0 ? "output0" : "output1", "tmp0", DEFAULT_TOTAL_MEMORY, DEFAULT_BLOCK_SIZE, tuple2Rcmp<uint32_t>);
	BufferedReader<Tuple2<uint32_t> > outputReader("tmp0", "rb", DEFAULT_BLOCK_SIZE);

	BufferedWriter<uint32_t> outputWriter("output.bin", "wb", DEFAULT_BLOCK_SIZE);

	cnt = 0;
	int i = 0;
	int min = -1;
	do {
		if (min == -1) {
			min = outputReader.get().left;
			i = cnt;
		}
		if (min > outputReader.get().left) {
			min = outputReader.get().left;
			i = cnt;
		}
		++cnt;
	} while (outputReader.seekNext());

	//printf("\nINDEX\n%d -> %d\n", i, min);

	fseek(outputReader.file, i * sizeof(uint32_t) * 2, SEEK_SET);
	outputReader.reload();
	do {
		outputWriter.put(outputReader.get().left);
	} while (outputReader.seekNext());

	outputReader.reset();
	while (i-- > 0) {
		outputWriter.put(outputReader.get().left);
		outputReader.seekNext();
	}
	outputReader.close();
	outputWriter.close();

	//
	/*outputTxt = fopen("ranked.txt", "w");
	outputSeq("output.bin", outputTxt);
	fclose(outputTxt);*/
	//

	return 0;
}