#include <stdio.h>
#include <cuda.h>

#include "device_launch_parameters.h"
#include "cuda_runtime.h"
#include <cuda_surface_types.h>


void cudaImageFunc(dim3 tpb, dim3 tb, uchar4 *devD, size_t pitch, int w, int h);
void cudaKernelReadWriteSurface(dim3 tpb, dim3 tb, cudaArray_t s, int w, int h);

