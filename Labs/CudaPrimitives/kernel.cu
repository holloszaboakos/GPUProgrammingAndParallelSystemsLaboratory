
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <chrono>
#include <iostream>

cudaError_t addWithCuda(int* c, const int* a, const int* b, unsigned int size);

__device__ void square(int& output, int input)
{
	output = input * input;
}

template <typename IN, typename OUT, void (FUN)(OUT&, IN)>
__global__ void map(IN* inputArray, OUT* outputArray)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	FUN(outputArray[index], inputArray[index]);
}

__device__ void sum(int& output, int left, int right)
{
	output = left + right;
}
template <typename T, void (FUN)(T&, T, T)>
__global__ void reduceFolding(T* inputArray, T* outputArray, unsigned int size)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	outputArray[index] = inputArray[index];
	__syncthreads();
	for (unsigned int shift = size / 2; shift != 0; shift /= 2) {
		if (index < shift)
			FUN(outputArray[index], outputArray[index], outputArray[index + shift]);
		__syncthreads();
	}
}
template <typename T, void (FUN)(T&, T, T)>
__global__ void reduceBinary(T* inputArray, T* outputArray, unsigned int size)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	outputArray[index] = inputArray[index];
	__syncthreads();
	for (unsigned int shift = 2; shift <= size; shift *= 2) {
		if (index % shift == 0)
			FUN(outputArray[index], outputArray[index], outputArray[index + (shift / 2)]);
		__syncthreads();
	}
}
template <typename T, void (FUN)(T&, T, T)>
__global__ void scanNaiv(T* inputArray, T* outputArray, unsigned int size)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	outputArray[index] = inputArray[index];
	__syncthreads();
	for (int shift = 1; shift <= size; shift *= 2) {
		if (index - shift >= 0)
			FUN(outputArray[index], outputArray[index], outputArray[index - shift]);
		__syncthreads();
	}
}

template <typename T, void (FUN)(T&, T, T)>
__global__ void scanBinary(T* inputArray, T* outputArray, unsigned int size)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	outputArray[index] = inputArray[index];
	__syncthreads();
	for (int shift = 2; shift <= size; shift *= 2) {
		int copySource = (shift / 2 - 1);
		if (index % shift > copySource)
			FUN(outputArray[index], outputArray[index], outputArray[index - (index % shift) + copySource]);
		__syncthreads();
	}
}

template <typename T>
__global__ void gather(T* inputArray, int* indexArray, T* outputArray)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	outputArray[index] = inputArray[indexArray[index]];
}

template <typename T>
__global__ void scatter(T* inputArray, int* indexArray, T* outputArray)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	outputArray[indexArray[index]] = inputArray[index];
}

template <typename T>
__global__ void compact(T* inputArray, int* flagArray, T* outputArray)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (flagArray[index] == 1) {
		int counter = 0;
		for (int i = 0; i < index; i++) {
			if (flagArray[i] == 1)
				counter++;
		}
		outputArray[counter] = inputArray[index];
	}
}

template <typename T>
__global__ void compactRewritingInputArray(T* dataArray, int size)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int value = dataArray[index];
	dataArray[index] = 0;
	if (dataArray[size + index] == 1) {
		int counter = 0;
		for (int i = size; i < size + index; i++) {
			if (dataArray[i] == 1)
				counter++;
		}
		dataArray[counter] = value;
	}
}

template <typename T>
__global__ void mergeSort(T* dataArray, T* swapArray, int size)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int value = dataArray[index];
	int firstArrayIndex;
	int secondArrayIndex;
	bool swap = true;
	for (int sortSize = 1; sortSize < size; sortSize *= 2) {
		if (index % (sortSize * 2) == 0) {
			firstArrayIndex = index;
			secondArrayIndex = index + sortSize;
			if (swap) {
				for (int resultIndex = index; resultIndex < index + sortSize * 2; resultIndex++)
					if (firstArrayIndex == index + sortSize) {
						swapArray[resultIndex] = dataArray[secondArrayIndex];
						secondArrayIndex++;
					}
					else if (secondArrayIndex == index + sortSize + sortSize) {
						swapArray[resultIndex] = dataArray[firstArrayIndex];
						firstArrayIndex++;
					}
					else if (dataArray[secondArrayIndex] < dataArray[firstArrayIndex]) {
						swapArray[resultIndex] = dataArray[secondArrayIndex];
						secondArrayIndex++;
					}
					else  {
						swapArray[resultIndex] = dataArray[firstArrayIndex];
						firstArrayIndex++;
					}
			}
			else {
				for (int resultIndex = index; resultIndex < index + sortSize * 2; resultIndex++)
					if (firstArrayIndex == index + sortSize) {
						dataArray[resultIndex] = swapArray[secondArrayIndex];
						secondArrayIndex++;
					}
					else if (secondArrayIndex == index + sortSize + sortSize) {
						dataArray[resultIndex] = swapArray[firstArrayIndex];
						firstArrayIndex++;
					}
					else if (swapArray[secondArrayIndex] < swapArray[firstArrayIndex]) {
						dataArray[resultIndex] = swapArray[secondArrayIndex];
						secondArrayIndex++;
					}
					else {
						dataArray[resultIndex] = swapArray[firstArrayIndex];
						firstArrayIndex++;
					}
			}
		}

		swap = !swap;
		__syncthreads();
	}
	if (!swap) {
		dataArray[index] = swapArray[index];
	}
}


__device__ int* data;

int main() {

	const int dataSize = 1024;
	int* dataCPU = new int[sizeof(int) * dataSize];
	int* indexCPU = new int[sizeof(int) * dataSize];
	int* resultCPU = new int[sizeof(int) * dataSize * 2];
	int* dataGPU;
	int* indexGPU;
	int* resultGPU;
	cudaMalloc(&dataGPU, sizeof(int) * dataSize * 2);
	cudaMalloc(&indexGPU, sizeof(int) * dataSize);
	cudaMalloc(&resultGPU, sizeof(int) * dataSize * 2);
	int threadsPerBlock = 256;
	int blocksPerGrid = 4;
	int wrongCount = 0;
	
	
	//map test
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
	}
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	map<int, int, square> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, resultGPU);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * dataSize, cudaMemcpyDeviceToHost);
	for (int i = 0; i < dataSize; ++i) {
		if (resultCPU[i] != i * i) wrongCount++;
	}
	printf("Number of wrong squares : % d\n", wrongCount);
	//binary reduce test
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
	}
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	reduceBinary<int, sum> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, resultGPU, dataSize);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * dataSize, cudaMemcpyDeviceToHost);
	if (resultCPU[0] != (dataSize - 1 + 0) * dataSize / 2)
		printf("Wrong result : % d\n", resultCPU[0]);
	else
		printf("Good result\n");
	//folding reduce test
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
	}
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	reduceFolding<int, sum> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, resultGPU, dataSize);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * dataSize, cudaMemcpyDeviceToHost);
	if (resultCPU[0] != (dataSize - 1 + 0) * dataSize / 2)
		printf("Wrong result : % d\n", resultCPU[0]);
	else
		printf("Good result\n");

	//naiv scan test
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
	}
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	scanNaiv<int, sum> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, resultGPU, dataSize);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * dataSize, cudaMemcpyDeviceToHost);
	unsigned int sumOf = 0;
	wrongCount = 0;
	for (int i = 0; i < dataSize; ++i) {
		sumOf += i;
		if (resultCPU[i] != sumOf) {
			printf("result : % d\n", resultCPU[i]);
			printf("wanted : % d\n", sumOf);
			wrongCount++;
		}
	}
	printf("Number of wrong sums : % d\n", wrongCount);

	//binary scan test
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
	}
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	scanBinary<int, sum><<< blocksPerGrid, threadsPerBlock >>>(dataGPU, resultGPU, dataSize);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * dataSize, cudaMemcpyDeviceToHost);
	sumOf = 0;
	wrongCount = 0;
	for (int i = 0; i < dataSize; ++i) {
		sumOf += i;
		if (resultCPU[i] != sumOf) {
			printf("result : % d\n", resultCPU[i]);
			printf("wanted : % d\n", sumOf);
			wrongCount++;
		}
	}
	printf("Number of wrong sums : % d\n", wrongCount);

	//gather
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = -i;
	}
	for (int i = 0; i < dataSize / 2; ++i) {
		indexCPU[i] = i * 2;
	}
	cudaMemcpy(dataGPU, dataCPU, sizeof(int)* dataSize, cudaMemcpyHostToDevice);
	cudaMemcpy(indexGPU, indexCPU, sizeof(int) * (dataSize/2), cudaMemcpyHostToDevice);
	gather<int> <<< blocksPerGrid / 2, threadsPerBlock >>> (dataGPU, indexGPU, resultGPU);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * (dataSize/2), cudaMemcpyDeviceToHost);
	wrongCount = 0;
	for (int i = 0; i < dataSize / 2; ++i) {
		if (resultCPU[i] != -i * 2) {
			wrongCount++;
		}
	}
	printf("Number of wrong sums : % d\n", wrongCount);
	//scatter
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
	}
	for (int i = 0; i < dataSize; ++i) {
		indexCPU[i] = i * 2;
	}
	for (int i = 0; i < dataSize * 2; ++i) {
		resultCPU[i] = 0;
	}
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	cudaMemcpy(indexGPU, indexCPU, sizeof(int)* dataSize, cudaMemcpyHostToDevice);
	cudaMemcpy(resultGPU, resultCPU, sizeof(int) * (dataSize * 2), cudaMemcpyHostToDevice);
	scatter<int> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, indexGPU, resultGPU);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * (dataSize * 2), cudaMemcpyDeviceToHost);
	wrongCount = 0;
	for (int i = 0; i < dataSize * 2; ++i) {
		if ((i%2 == 0 && resultCPU[i]!=i/2) || (i % 2 == 1 && resultCPU[i] != 0)) {
			wrongCount++;
		}
	}
	printf("Number of wrong sums : % d\n", wrongCount);
	
	//compact
	
	std::chrono::steady_clock::time_point initialize = std::chrono::steady_clock::now();
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
		indexCPU[i] = i % 2;
	}
	for (int i = 0; i < dataSize / 2; ++i) {
		resultCPU[i] = 0;
	}
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	cudaMemcpy(indexGPU, indexCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	compact<int> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, indexGPU, resultGPU);
	cudaMemcpy(resultCPU, resultGPU, sizeof(int) * (dataSize / 2), cudaMemcpyDeviceToHost);
	wrongCount = 0;
	for (int i = 0; i < dataSize / 2; ++i) {
		if (resultCPU[i] != (i * 2) + 1) {
			wrongCount++;
		}
	}
	printf("Number of wrong sums : % d\n", wrongCount);
	//gpu terület felszabadítása

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	std::cout << "Time until beging = " << std::chrono::duration_cast<std::chrono::nanoseconds> (begin - initialize).count() << "[nanoseconds]" << std::endl;
	std::cout << "Time until end = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[nanoseconds]" << std::endl;


	initialize = std::chrono::steady_clock::now();
	//const int dataSize = 1024;
	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = i;
	}
	for (int i = dataSize; i < dataSize * 2; ++i) {
		dataCPU[i] = i%2;
	}
	begin = std::chrono::steady_clock::now();
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize * 2, cudaMemcpyHostToDevice);
	compactRewritingInputArray<int> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, dataSize);
	cudaMemcpy(dataCPU, dataGPU, sizeof(int) * dataSize, cudaMemcpyDeviceToHost);
	wrongCount = 0;
	for (int i = 0; i < dataSize / 2; ++i) {
		if (dataCPU[i] != (i * 2) + 1) {
			wrongCount++;
		}
	}
	for (int i = dataSize / 2; i < dataSize; ++i) {
		if (dataCPU[i] != 0) {
			wrongCount++;
		}
	}
	printf("Number of wrong sums : % d\n", wrongCount);
	//gpu terület felszabadítása
	//***C++11 Style:***

	end = std::chrono::steady_clock::now();

	std::cout << "Time until beging = " << std::chrono::duration_cast<std::chrono::nanoseconds> (begin - initialize).count() << "[nanoseconds]" << std::endl;
	std::cout << "Time until end = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[nanoseconds]" << std::endl;

	initialize = std::chrono::steady_clock::now();

	for (int i = 0; i < dataSize; ++i) {
		dataCPU[i] = dataSize - i;
	}
	begin = std::chrono::steady_clock::now();
	cudaMemcpy(dataGPU, dataCPU, sizeof(int) * dataSize, cudaMemcpyHostToDevice);
	mergeSort<int> <<< blocksPerGrid, threadsPerBlock >>> (dataGPU, dataGPU + dataSize, dataSize);
	cudaMemcpy(dataCPU, dataGPU, sizeof(int) * dataSize, cudaMemcpyDeviceToHost);
	wrongCount = 0;
	for (int i = 0; i < dataSize - 1; ++i) {
		if (dataCPU[i] > dataCPU[i+1]) {
			wrongCount++;
			printf("index: % d value1: % d value2: % d\n", i, dataCPU[i], dataCPU[i+1]);
		}
	}
	printf("Number of wrong sums : % d\n", wrongCount);
	//gpu terület felszabadítása

	end = std::chrono::steady_clock::now();

	std::cout << "Time until beging = " << std::chrono::duration_cast<std::chrono::nanoseconds> (begin - initialize).count() << "[nanoseconds]" << std::endl;
	std::cout << "Time until end = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[nanoseconds]" << std::endl;

	cudaFree(dataGPU);
	cudaFree(indexGPU);
	cudaFree(resultGPU);

	return 0;
}
