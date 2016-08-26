
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLMemoryAssistant.h>
#include <vector>

using namespace icl::utils;

static const char kernel01[] = "__kernel void plus(__global int *v0, __global int *v1, __global int *vR) {"
							   "	int id = get_global_id(0);"
							   "	vR[id] = v0[id] + v1[id];"
							   "}";

static const char kernel02[] = "__kernel void mult(__global int *v0, __global int *v1, __global int *vR) {"
							   "	int id = get_global_id(0);"
							   "	vR[id] = v0[id] * v1[id];"
							   "}";

// /////////////////////////////////////////////////////////////////////////////////////////////////

class Program01 {

public:

	Program01(CLDeviceContext const &context) {
		prog = CLProgram(context,std::string(kernel01));
		kernel = prog.createKernel("plus");

	}

	Program01() {
		prog = CLProgram("gpu",std::string(kernel01));
		kernel = prog.createKernel("plus");
	}

	void apply(CLBuffer &v0, CLBuffer &v1, CLBuffer &res, int size) {
		kernel.setArgs(v0,v1,res);
		kernel.apply(size);
	}

	~Program01() {}

	CLProgram prog;
	CLKernel kernel;

};

// /////////////////////////////////////////////////////////////////////////////////////////////////

class Program02 {

public:

	Program02(CLDeviceContext const &context) {
		prog = CLProgram(context,std::string(kernel02));
		kernel = prog.createKernel("mult");

	}

	Program02() {
		prog = CLProgram("gpu",std::string(kernel02));
		kernel = prog.createKernel("mult");
	}

	void apply(CLBuffer &v0, CLBuffer &v1, CLBuffer &res, int size) {
		kernel.setArgs(v0,v1,res);
		kernel.apply(size);
	}

	CLProgram prog;
	CLKernel kernel;

};

// /////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {

	CLMemoryAssistant cl_ass("gpu");

	Program01 p1(cl_ass.getDeviceContext());
	Program02 p2(cl_ass.getDeviceContext());

	std::vector<int> v1_v;// = {0,1,2,3,4,5,6,7,8,9};
	std::vector<int> v2_v;// = {0,1,2,3,4,5,6,7,8,9};
	std::vector<int> v3_v;// = {2,2,2,2,2,2,2,2,2,2};
	for (int i = 0; i < 10; ++i) {
		v1_v.push_back(i);
		v2_v.push_back(i);
		v3_v.push_back(2);
	}

	size_t num_bytes = sizeof(int);
	cl_ass.createNamedBuffer("v1","rw",v1_v.size(),num_bytes,&v1_v[0]);
	cl_ass.createNamedBuffer("v2","rw",v2_v.size(),num_bytes,&v2_v[0]);
	cl_ass.createNamedBuffer("v3","rw",v3_v.size(),num_bytes,&v3_v[0]);
	cl_ass.createNamedBuffer("res1","rw",v1_v.size(),num_bytes);
	cl_ass.createNamedBuffer("res2","rw",v3_v.size(),num_bytes);

	p1.apply(cl_ass.asBuf("v1"),cl_ass.asBuf("v2"),cl_ass.asBuf("res1"),v1_v.size());
	p2.apply(cl_ass.asBuf("res1"),cl_ass.asBuf("v3"),cl_ass.asBuf("res2"),v1_v.size());

	std::vector<int> res_v(v3_v.size(),0);

	cl_ass.asBuf("res2").read(&res_v[0],sizeof(int)*v3_v.size());

	for (uint i = 0; i < res_v.size(); ++i)
		std::cout << res_v[i] << "; ";
	std::cout << std::endl;

	return 0;
}
