#include <iostream>
#include "matrix.hpp"
#include <sys/time.h>

#define CL_TARGET_OPENCL_VERSION 120
#define CL_MINIMUM_OPENCL_VERSION 120
#include <CL/cl.hpp>

using namespace std;

const size_t SIZE = 1024;
const size_t N = SIZE*SIZE;
typedef Matrix& MatrixRef;

void MultiplyMatrices_cpu(const MatrixRef a, const MatrixRef b, MatrixRef c) {
  for (size_t i = 0; i < SIZE; i++) {
    for (size_t j = 0; j < SIZE; j++) {
      for (size_t k = 0; k < SIZE; k++) {
        c[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}


//SIMT top locality priority is the j index (thread id lowest dimension)
//J (TID0)
//  A: Reuse of a single data item across adjacent threads (temporal locality)
//  B: Adjacent threads access adjacent memory locations (spatial locality)
//K (Loop inside each thread)
//  A: Consecutive iterations access consecutive elements of A (spatial locality)
//  B: Consecutive iterations access consecutive rows of B (poor locality)
//I (TID1)
//  A: Accesses all rows of A (poor locality, doesn't matter)
//  B: Reusing the B matrix (temporal locality, doesn't matter)


//Microthreads - each thread executes independently, but will swap out 
//  on cache miss or other blocking operation
//
//Even assuming threads execute multiple instruction until a stall, 
// each thread will probably hit a stall on every iteration for a miss in the 
// B matrix.
//J (TID0)
//K 
//I

const std::string MMProgramString = "\
__kernel void mmul(global float* A, global float* B, global float* C, size_t SIZE) {\
  int i = get_global_id(1)*2;\
  int j_start = get_global_id(0)*4;\
  float4 sum0 = {0.0f, 0.0f, 0.0f, 0.0f};\
  float4 sum1 = {0.0f, 0.0f, 0.0f, 0.0f};\
  for (int k = 0; k < SIZE; k++) {\
    float4 Bvec = (*(global float4*) &B[k*SIZE + j_start]);\
    sum0 += A[i*SIZE+k] * Bvec;\
    sum1 += A[(i+1)*SIZE+k] * Bvec;\
  }\
  (*(global float4*) & C[i*SIZE+j_start]) += sum0;\
  (*(global float4*) & C[(i+1)*SIZE+j_start]) += sum1;\
  return;\
}\
";

void MultiplyMatrices_ocl(const MatrixRef a, const MatrixRef b, MatrixRef c) {
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  if (platforms.empty()) {
    std::cerr << "\nNo OpenCL Platforms found";
    exit(1);
  }

  cl::Platform platform = platforms[0]; //Pick platform 0 as default

  std::vector<cl::Device> devices;
  platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
  if (devices.empty()) {
    std::cerr << "\nNo OpenCL Devices found";
    exit(1);
  }

  cl::Device device = devices[0]; //Pick device 0 as default
  cl::Context context({device});

  cl::Program::Sources sources;
  sources.push_back({MMProgramString.c_str(), MMProgramString.length()+1});
  cl::Program program(context, sources);
  cl_int buildstatus = program.build({device});

  if (buildstatus != CL_SUCCESS) {
    std::cerr << "\nError building appliction: " 
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
  }
  cl::CommandQueue queue(context, device);

  //Option 3

  cl::Buffer deviceA(context, CL_MEM_ALLOC_HOST_PTR, sizeof(float)*N);
  cl::Buffer deviceB(context, CL_MEM_ALLOC_HOST_PTR, sizeof(float)*N);
  cl::Buffer deviceC(context, CL_MEM_ALLOC_HOST_PTR, sizeof(float)*N);

  float* hostA = (float*) queue.enqueueMapBuffer(deviceA, CL_TRUE, {CL_MAP_WRITE}, 0, sizeof(float)*N);
  float* hostB = (float*) queue.enqueueMapBuffer(deviceB, CL_TRUE, {CL_MAP_WRITE}, 0, sizeof(float)*N);
  float* hostC = (float*) queue.enqueueMapBuffer(deviceC, CL_TRUE, {CL_MAP_WRITE}, 0, sizeof(float)*N);
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      hostA[i*SIZE + j] = a[i][j];
      hostB[i*SIZE + j] = b[i][j];
      hostC[i*SIZE + j] = c[i][j];
    }
  }

  //Last possible place
  cl::Kernel mmul_kernel(program, "mmul");
  mmul_kernel.setArg(0, deviceA);
  mmul_kernel.setArg(1, deviceB);
  mmul_kernel.setArg(2, deviceC);
  mmul_kernel.setArg(3, SIZE);

  cl::Event kernel_finished;
  queue.enqueueNDRangeKernel(mmul_kernel, cl::NullRange, cl::NDRange(SIZE/4, SIZE/2), cl::NullRange, nullptr, &kernel_finished);
  queue.finish();

  struct timeval start, end;

  gettimeofday(&start, NULL);
  for (int i = 0; i< 10; i++) {
    queue.enqueueNDRangeKernel(mmul_kernel, cl::NullRange, cl::NDRange(SIZE/4, SIZE/2), cl::NullRange, nullptr, &kernel_finished);
  }
  queue.finish();
  gettimeofday(&end, NULL);

  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      a[i][j] = hostA[i*SIZE + j];
      b[i][j] = hostB[i*SIZE + j];
      c[i][j] = hostC[i*SIZE + j];
    }
  }

  double elapsedtime_sec = double(end.tv_sec - start.tv_sec) + 
                             double(end.tv_usec - start.tv_usec)/1000000.0;
  cout << std::endl << "\nGPU Multiplication time (N=" << SIZE << "): " << elapsedtime_sec / 10 << std::endl;
  cl_int status = 0;
  kernel_finished.getInfo(CL_EVENT_COMMAND_EXECUTION_STATUS, &status);
  if (status != CL_COMPLETE) {
    std::cerr << std::endl << "Error executing kernel: Code " << (int) status << std::endl;
    exit(1);
  }

  return;
}

int main() {
  Matrix a(SIZE);
  Matrix b(SIZE);
  Matrix c(SIZE);

  struct timeval start, end;

  gettimeofday(&start, NULL);

  MultiplyMatrices_ocl(a, b, c);

  gettimeofday(&end, NULL);

  double elapsedtime_sec = double(end.tv_sec - start.tv_sec) + 
                             double(end.tv_usec - start.tv_usec)/1000000.0;
  cout << std::endl << "Multiplication time (N=" << SIZE << "): " << elapsedtime_sec << std::endl;

  float checksum = 0.0f;
  for (size_t i = 0; i < SIZE; i++) {
    for (size_t j = 0; j < SIZE; j++) {
      checksum += c[i][j];
    }
  }

  cout << "MMchecksum = " << checksum << endl;

  return 0;
}
