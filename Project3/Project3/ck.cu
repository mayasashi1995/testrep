#include "ck.cuh"

surface<void, cudaSurfaceType2D> surfRef;

__global__  void makeImage(uchar4 *A, size_t pitch, const int w, const int h)
{
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	int j = blockIdx.y * blockDim.y + threadIdx.y;

	uchar4 *p = (uchar4 *)((char *)A + j * pitch) + i;
	(*p).x = i * 255.0f / w;
	(*p).y = j * 255.0f / h;
	(*p).z = (*p).z + 1;
	(*p).w = 255;
}

__global__ void cudaKernelReadWriteSurface_Kernel(cudaArray_t s, int w, int h)
{

}

void cudaImageFunc(dim3 tpb, dim3 tb, uchar4 *devD, size_t pitch, int w, int h)
{
	printf("cudaImageFunc called\n");
	cudaError_t err;
	makeImage<<<tb, tpb>>>(devD, pitch, w, h);
	err = cudaGetLastError();
	if (err != cudaSuccess)
	{
		printf("WARNING : %s\n", cudaGetErrorString(err));
	}
}

void cudaKernelReadWriteSurface(dim3 tpb, dim3 tb, cudaArray_t s, int w, int h)
{
	/*Create surface reference*/
	const surfaceReference *surfRefPtr;
	cudaGetSurfaceReference(&surfRefPtr, &surfRef);
	cudaChannelFormatDesc channelDesc;
	cudaGetChannelDesc(&channelDesc, s);
	cudaBindSurfaceToArray(surfRefPtr, s, &channelDesc); 

}