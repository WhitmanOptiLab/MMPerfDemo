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

const std::string MMProgramString = "\
__kernel void mmul(global float* A, global float* B, global float* C, size_t SIZE) {\
  int i = get_global_id(1);\
  int j = get_global_id(0);\
  float sum = 0.0f;\
  for (int k = 0; k < SIZE; k++) {\
    sum += a[i*SIZE+k] * b[k*SIZE + j];\
  }\
  c[i*SIZE+j] = sum;\
  return;\
}\
";

void MultiplyMatrices_ocl(const MatrixRef a, const MatrixRef b, MatrixRef c) {
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  if (platforms.empty()) {
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
  cl::Program program(context, sources);
  program.build({device});
  cl::Buffer deviceA(context, CL_MEM_READ_ONLY, sizeof(float)*N);
  cl::Buffer deviceB(context, CL_MEM_READ_ONLY, sizeof(float)*N);
  cl::Buffer deviceC(context, CL_MEM_READ_WRITE, sizeof(float)*N);

  cl::CommandQueue queue(context, device);

  queue.enqueueWriteBuffer(deviceA, CL_TRUE, 0, sizeof(float)*N, a[0]);
  queue.enqueueWriteBuffer(deviceB, CL_TRUE, 0, sizeof(float)*N, b[0]);
  queue.enqueueWriteBuffer(deviceC, CL_TRUE, 0, sizeof(float)*N, c[0]);

  cl::Kernel mmul_kernel(program, "mmul");
  mmul_kernel.setArg(0, deviceA);
  mmul_kernel.setArg(1, deviceB);
  mmul_kernel.setArg(2, deviceC);
  mmul_kernel.setArg(3, SIZE);

  cl::Event kernel_finished;
  queue.enqueueNDRangeKernel(mmul_kernel, cl::NullRange, cl::NDRange(SIZE, SIZE), cl::NullRange, nullptr, &kernel_finished);

  queue.enqueueReadBuffer(deviceC, CL_TRUE, 0, sizeof(float)*N, c[0]);
  queue.finish();
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
