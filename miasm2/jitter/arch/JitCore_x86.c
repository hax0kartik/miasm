#include <Python.h>
#include "structmember.h"
#include <stdint.h>
#include <inttypes.h>
#include "../queue.h"
#include "../vm_mngr.h"
#include "../vm_mngr_py.h"
#include "../bn.h"
#include "../JitCore.h"
#include "../op_semantics.h"
#include "JitCore_x86.h"

vm_cpu_t ref_arch_regs;

reg_dict gpreg_dict[] = {
			 {.name = "RAX", .offset = offsetof(vm_cpu_t, RAX), .size = 64},
			 {.name = "RBX", .offset = offsetof(vm_cpu_t, RBX), .size = 64},
			 {.name = "RCX", .offset = offsetof(vm_cpu_t, RCX), .size = 64},
			 {.name = "RDX", .offset = offsetof(vm_cpu_t, RDX), .size = 64},
			 {.name = "RSI", .offset = offsetof(vm_cpu_t, RSI), .size = 64},
			 {.name = "RDI", .offset = offsetof(vm_cpu_t, RDI), .size = 64},
			 {.name = "RSP", .offset = offsetof(vm_cpu_t, RSP), .size = 64},
			 {.name = "RBP", .offset = offsetof(vm_cpu_t, RBP), .size = 64},

			 {.name = "R8", .offset = offsetof(vm_cpu_t, R8), .size = 64},
			 {.name = "R9", .offset = offsetof(vm_cpu_t, R9), .size = 64},
			 {.name = "R10", .offset = offsetof(vm_cpu_t, R10), .size = 64},
			 {.name = "R11", .offset = offsetof(vm_cpu_t, R11), .size = 64},
			 {.name = "R12", .offset = offsetof(vm_cpu_t, R12), .size = 64},
			 {.name = "R13", .offset = offsetof(vm_cpu_t, R13), .size = 64},
			 {.name = "R14", .offset = offsetof(vm_cpu_t, R14), .size = 64},
			 {.name = "R15", .offset = offsetof(vm_cpu_t, R15), .size = 64},

			 {.name = "RIP", .offset = offsetof(vm_cpu_t, RIP), .size = 64},

			 {.name = "zf", .offset = offsetof(vm_cpu_t, zf), .size = 8},
			 {.name = "nf", .offset = offsetof(vm_cpu_t, nf), .size = 8},
			 {.name = "pf", .offset = offsetof(vm_cpu_t, pf), .size = 8},
			 {.name = "of", .offset = offsetof(vm_cpu_t, of), .size = 8},
			 {.name = "cf", .offset = offsetof(vm_cpu_t, cf), .size = 8},
			 {.name = "af", .offset = offsetof(vm_cpu_t, af), .size = 8},
			 {.name = "df", .offset = offsetof(vm_cpu_t, df), .size = 8},

			 {.name = "ES", .offset = offsetof(vm_cpu_t, ES), .size = 16},
			 {.name = "CS", .offset = offsetof(vm_cpu_t, CS), .size = 16},
			 {.name = "SS", .offset = offsetof(vm_cpu_t, SS), .size = 16},
			 {.name = "DS", .offset = offsetof(vm_cpu_t, DS), .size = 16},
			 {.name = "FS", .offset = offsetof(vm_cpu_t, FS), .size = 16},
			 {.name = "GS", .offset = offsetof(vm_cpu_t, GS), .size = 16},

			 {.name = "MM0", .offset = offsetof(vm_cpu_t, MM0), .size = 64},
			 {.name = "MM1", .offset = offsetof(vm_cpu_t, MM1), .size = 64},
			 {.name = "MM2", .offset = offsetof(vm_cpu_t, MM2), .size = 64},
			 {.name = "MM3", .offset = offsetof(vm_cpu_t, MM3), .size = 64},
			 {.name = "MM4", .offset = offsetof(vm_cpu_t, MM4), .size = 64},
			 {.name = "MM5", .offset = offsetof(vm_cpu_t, MM5), .size = 64},
			 {.name = "MM6", .offset = offsetof(vm_cpu_t, MM6), .size = 64},
			 {.name = "MM7", .offset = offsetof(vm_cpu_t, MM7), .size = 64},

			 {.name = "XMM0", .offset = offsetof(vm_cpu_t, XMM0), .size = 128},
			 {.name = "XMM1", .offset = offsetof(vm_cpu_t, XMM1), .size = 128},
			 {.name = "XMM2", .offset = offsetof(vm_cpu_t, XMM2), .size = 128},
			 {.name = "XMM3", .offset = offsetof(vm_cpu_t, XMM3), .size = 128},
			 {.name = "XMM4", .offset = offsetof(vm_cpu_t, XMM4), .size = 128},
			 {.name = "XMM5", .offset = offsetof(vm_cpu_t, XMM5), .size = 128},
			 {.name = "XMM6", .offset = offsetof(vm_cpu_t, XMM6), .size = 128},
			 {.name = "XMM7", .offset = offsetof(vm_cpu_t, XMM7), .size = 128},
			 {.name = "XMM8", .offset = offsetof(vm_cpu_t, XMM8), .size = 128},
			 {.name = "XMM9", .offset = offsetof(vm_cpu_t, XMM9), .size = 128},
			 {.name = "XMM10", .offset = offsetof(vm_cpu_t, XMM10), .size = 128},
			 {.name = "XMM11", .offset = offsetof(vm_cpu_t, XMM11), .size = 128},
			 {.name = "XMM12", .offset = offsetof(vm_cpu_t, XMM12), .size = 128},
			 {.name = "XMM13", .offset = offsetof(vm_cpu_t, XMM13), .size = 128},
			 {.name = "XMM14", .offset = offsetof(vm_cpu_t, XMM14), .size = 128},
			 {.name = "XMM15", .offset = offsetof(vm_cpu_t, XMM15), .size = 128},

			 {.name = "tsc1", .offset = offsetof(vm_cpu_t, tsc1), .size = 32},
			 {.name = "tsc2", .offset = offsetof(vm_cpu_t, tsc2), .size = 32},

			 {.name = "exception_flags", .offset = offsetof(vm_cpu_t, exception_flags), .size = 32},
			 {.name = "interrupt_num", .offset = offsetof(vm_cpu_t, interrupt_num), .size = 32},
};



/************************** JitCpu object **************************/





PyObject* cpu_get_gpreg(JitCpu* self)
{
    PyObject *dict = PyDict_New();
    PyObject *o;

    get_reg(RAX);
    get_reg(RBX);
    get_reg(RCX);
    get_reg(RDX);
    get_reg(RSI);
    get_reg(RDI);
    get_reg(RSP);
    get_reg(RBP);

    get_reg(R8);
    get_reg(R9);
    get_reg(R10);
    get_reg(R11);
    get_reg(R12);
    get_reg(R13);
    get_reg(R14);
    get_reg(R15);

    get_reg(RIP);

    get_reg(zf);
    get_reg(nf);
    get_reg(pf);
    get_reg(of);
    get_reg(cf);
    get_reg(af);
    get_reg(df);


    get_reg(ES);
    get_reg(CS);
    get_reg(SS);
    get_reg(DS);
    get_reg(FS);
    get_reg(GS);

    get_reg(MM0);
    get_reg(MM1);
    get_reg(MM2);
    get_reg(MM3);
    get_reg(MM4);
    get_reg(MM5);
    get_reg(MM6);
    get_reg(MM7);

    get_reg_bn(XMM0, 128);
    get_reg_bn(XMM1, 128);
    get_reg_bn(XMM2, 128);
    get_reg_bn(XMM3, 128);
    get_reg_bn(XMM4, 128);
    get_reg_bn(XMM5, 128);
    get_reg_bn(XMM6, 128);
    get_reg_bn(XMM7, 128);
    get_reg_bn(XMM8, 128);
    get_reg_bn(XMM9, 128);
    get_reg_bn(XMM10, 128);
    get_reg_bn(XMM11, 128);
    get_reg_bn(XMM12, 128);
    get_reg_bn(XMM13, 128);
    get_reg_bn(XMM14, 128);
    get_reg_bn(XMM15, 128);

    get_reg(tsc1);
    get_reg(tsc2);

    return dict;
}





PyObject* cpu_set_gpreg(JitCpu* self, PyObject *args)
{
    PyObject* dict;
    PyObject *d_key, *d_value = NULL;
    Py_ssize_t pos = 0;
    uint64_t val;
    unsigned int i, found;

    if (!PyArg_ParseTuple(args, "O", &dict))
	    RAISE(PyExc_TypeError,"Cannot parse arguments");
    if(!PyDict_Check(dict))
	    RAISE(PyExc_TypeError, "arg must be dict");
    while(PyDict_Next(dict, &pos, &d_key, &d_value)){
	    if(!PyString_Check(d_key))
		    RAISE(PyExc_TypeError, "key must be str");

	    found = 0;
	    for (i=0; i < sizeof(gpreg_dict)/sizeof(reg_dict); i++){
		    if (strcmp(PyString_AsString(d_key), gpreg_dict[i].name))
			    continue;
		    found = 1;
		    switch (gpreg_dict[i].size) {
			    case 8:
				    PyGetInt(d_value, val);
				    *((uint8_t*)(((char*)(self->cpu)) + gpreg_dict[i].offset)) = val;
				    break;
			    case 16:
				    PyGetInt(d_value, val);
				    *((uint16_t*)(((char*)(self->cpu)) + gpreg_dict[i].offset)) = val;
				    break;
			    case 32:
				    PyGetInt(d_value, val);
				    *((uint32_t*)(((char*)(self->cpu)) + gpreg_dict[i].offset)) = val;
				    break;
			    case 64:
				    PyGetInt(d_value, val);
				    *((uint64_t*)(((char*)(self->cpu)) + gpreg_dict[i].offset)) = val;
				    break;
			    case 128:
				    {
					    bn_t bn;
					    int j;
					    PyObject* py_long = d_value;
					    PyObject* py_long_new;
					    PyObject* py_tmp;
					    PyObject* cst_32;
					    PyObject* cst_ffffffff;
					    uint64_t tmp;

					    /* Ensure py_long is a PyLong */
					    if (PyInt_Check(py_long)){
						    tmp = (uint64_t)PyInt_AsLong(py_long);
						    py_long = PyLong_FromLong((long)tmp);
					    } else if (PyLong_Check(py_long)){
						    /* Already PyLong */
						    /* Increment ref as we will decement it next */
						    Py_INCREF(py_long);
					    }
					    else{
						    RAISE(PyExc_TypeError,"arg must be int");
					    }


					    cst_ffffffff = PyLong_FromLong(0xffffffff);
					    cst_32 = PyLong_FromLong(32);
					    bn = bignum_from_int(0);

					    for (j = 0; j < BN_BYTE_SIZE; j += 4) {
						    py_tmp = PyObject_CallMethod(py_long, "__and__", "O", cst_ffffffff);
						    tmp = PyLong_AsUnsignedLongMask(py_tmp);
						    Py_DECREF(py_tmp);
						    bn = bignum_lshift(bn, 32);
						    bn = bignum_or(bn, bignum_from_uint64(tmp));

						    py_long_new = PyObject_CallMethod(py_long, "__rshift__", "O", cst_32);
						    Py_DECREF(py_long);
						    py_long = py_long_new;
					    }
					    Py_DECREF(py_long);
					    Py_DECREF(cst_32);
					    Py_DECREF(cst_ffffffff);
					    *(bn_t*)(((char*)(self->cpu)) + gpreg_dict[i].offset) = bignum_mask(bn, 128);
				    }
				    break;
		    }
		    break;
	    }

	    if (found)
		    continue;
	    fprintf(stderr, "unknown key: %s\n", PyString_AsString(d_key));
	    RAISE(PyExc_ValueError, "unknown reg");
    }
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject * cpu_init_regs(JitCpu* self)
{
	memset(self->cpu, 0, sizeof(vm_cpu_t));
	((vm_cpu_t*)self->cpu)->tsc1 = 0x22222222;
	((vm_cpu_t*)self->cpu)->tsc2 = 0x11111111;
	((vm_cpu_t*)self->cpu)->i_f = 1;
	Py_INCREF(Py_None);
	return Py_None;

}

void dump_gpregs_16(vm_cpu_t* vmcpu)
{
	printf("EAX %.8"PRIX32" EBX %.8"PRIX32" ECX %.8"PRIX32" EDX %.8"PRIX32" ",
	       (uint32_t)(vmcpu->RAX & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RBX & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RCX & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RDX & 0xFFFFFFFF));
	printf("ESI %.8"PRIX32" EDI %.8"PRIX32" ESP %.8"PRIX32" EBP %.8"PRIX32" ",
	       (uint32_t)(vmcpu->RSI & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RDI & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RSP & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RBP & 0xFFFFFFFF));
	printf("EIP %.8"PRIX32" ",
	       (uint32_t)(vmcpu->RIP & 0xFFFFFFFF));
	printf("zf %.1d nf %.1d of %.1d cf %.1d\n",
	       (uint32_t)(vmcpu->zf & 0x1),
	       (uint32_t)(vmcpu->nf & 0x1),
	       (uint32_t)(vmcpu->of & 0x1),
	       (uint32_t)(vmcpu->cf & 0x1));
}

void dump_gpregs_32(vm_cpu_t* vmcpu)
{

	printf("EAX %.8"PRIX32" EBX %.8"PRIX32" ECX %.8"PRIX32" EDX %.8"PRIX32" ",
	       (uint32_t)(vmcpu->RAX & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RBX & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RCX & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RDX & 0xFFFFFFFF));
	printf("ESI %.8"PRIX32" EDI %.8"PRIX32" ESP %.8"PRIX32" EBP %.8"PRIX32" ",
	       (uint32_t)(vmcpu->RSI & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RDI & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RSP & 0xFFFFFFFF),
	       (uint32_t)(vmcpu->RBP & 0xFFFFFFFF));
	printf("EIP %.8"PRIX32" ",
	       (uint32_t)(vmcpu->RIP & 0xFFFFFFFF));
	printf("zf %.1d nf %.1d of %.1d cf %.1d\n",
	       (uint32_t)(vmcpu->zf & 0x1),
	       (uint32_t)(vmcpu->nf & 0x1),
	       (uint32_t)(vmcpu->of & 0x1),
	       (uint32_t)(vmcpu->cf & 0x1));

}

void dump_gpregs_64(vm_cpu_t* vmcpu)
{

	printf("RAX %.16"PRIX64" RBX %.16"PRIX64" RCX %.16"PRIX64" RDX %.16"PRIX64" ",
	       vmcpu->RAX, vmcpu->RBX, vmcpu->RCX, vmcpu->RDX);
	printf("RSI %.16"PRIX64" RDI %.16"PRIX64" RSP %.16"PRIX64" RBP %.16"PRIX64" ",
	       vmcpu->RSI, vmcpu->RDI, vmcpu->RSP, vmcpu->RBP);
	printf("RIP %.16"PRIX64"\n",
	       vmcpu->RIP);
	printf("R8  %.16"PRIX64" R9  %.16"PRIX64" R10 %.16"PRIX64" R11 %.16"PRIX64" ",
	       vmcpu->R8, vmcpu->R9, vmcpu->R10, vmcpu->R11);
	printf("R12 %.16"PRIX64" R13 %.16"PRIX64" R14 %.16"PRIX64" R15 %.16"PRIX64" ",
	       vmcpu->R12, vmcpu->R13, vmcpu->R14, vmcpu->R15);


	printf("zf %.1d nf %.1d of %.1d cf %.1d\n",
	       vmcpu->zf, vmcpu->nf, vmcpu->of, vmcpu->cf);

}

PyObject * cpu_dump_gpregs(JitCpu* self, PyObject* args)
{
	vm_cpu_t* vmcpu;

	vmcpu = self->cpu;
	dump_gpregs_64(vmcpu);
	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * cpu_dump_gpregs_with_attrib(JitCpu* self, PyObject* args)
{
	vm_cpu_t* vmcpu;
	PyObject *item1;
	uint64_t attrib;

	if (!PyArg_ParseTuple(args, "O", &item1))
		RAISE(PyExc_TypeError,"Cannot parse arguments");

	PyGetInt(item1, attrib);

	vmcpu = self->cpu;
	if (attrib == 16 || attrib == 32)
		dump_gpregs_32(vmcpu);
	else if (attrib == 64)
		dump_gpregs_64(vmcpu);
	else {
		RAISE(PyExc_TypeError,"Bad attrib");
	}

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject* cpu_set_exception(JitCpu* self, PyObject* args)
{
	PyObject *item1;
	uint64_t i;

	if (!PyArg_ParseTuple(args, "O", &item1))
		RAISE(PyExc_TypeError,"Cannot parse arguments");

	PyGetInt(item1, i);

	((vm_cpu_t*)self->cpu)->exception_flags = i;
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* cpu_get_exception(JitCpu* self, PyObject* args)
{
	return PyLong_FromUnsignedLongLong((uint64_t)(((vm_cpu_t*)self->cpu)->exception_flags));
}

PyObject* cpu_set_interrupt_num(JitCpu* self, PyObject* args)
{
	PyObject *item1;
	uint64_t i;

	if (!PyArg_ParseTuple(args, "O", &item1))
		RAISE(PyExc_TypeError,"Cannot parse arguments");

	PyGetInt(item1, i);

	((vm_cpu_t*)self->cpu)->interrupt_num = i;
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* cpu_get_interrupt_num(JitCpu* self, PyObject* args)
{
	return PyLong_FromUnsignedLongLong((uint64_t)(((vm_cpu_t*)self->cpu)->interrupt_num));
}

PyObject* cpu_set_segm_base(JitCpu* self, PyObject* args)
{
	PyObject *item1, *item2;
	uint64_t segm_num, segm_base;

	if (!PyArg_ParseTuple(args, "OO", &item1, &item2))
		RAISE(PyExc_TypeError,"Cannot parse arguments");

	PyGetInt(item1, segm_num);
	PyGetInt(item2, segm_base);
	((vm_cpu_t*)self->cpu)->segm_base[segm_num] = segm_base;

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* cpu_get_segm_base(JitCpu* self, PyObject* args)
{
	PyObject *item1;
	uint64_t segm_num;
	PyObject* v;

	if (!PyArg_ParseTuple(args, "O", &item1))
		RAISE(PyExc_TypeError,"Cannot parse arguments");
	PyGetInt(item1, segm_num);
	v = PyInt_FromLong((long)(((vm_cpu_t*)self->cpu)->segm_base[segm_num]));
	return v;
}

uint64_t segm2addr(JitCpu* jitcpu, uint64_t segm, uint64_t addr)
{
	return addr + ((vm_cpu_t*)jitcpu->cpu)->segm_base[segm];
}

void MEM_WRITE_08(JitCpu* jitcpu, uint64_t addr, uint8_t src)
{
	vm_MEM_WRITE_08(&((VmMngr*)jitcpu->pyvm)->vm_mngr, addr, src);
}

void MEM_WRITE_16(JitCpu* jitcpu, uint64_t addr, uint16_t src)
{
	vm_MEM_WRITE_16(&((VmMngr*)jitcpu->pyvm)->vm_mngr, addr, src);
}

void MEM_WRITE_32(JitCpu* jitcpu, uint64_t addr, uint32_t src)
{
	vm_MEM_WRITE_32(&((VmMngr*)jitcpu->pyvm)->vm_mngr, addr, src);
}

void MEM_WRITE_64(JitCpu* jitcpu, uint64_t addr, uint64_t src)
{
	vm_MEM_WRITE_64(&((VmMngr*)jitcpu->pyvm)->vm_mngr, addr, src);
}



PyObject* vm_set_mem(JitCpu *self, PyObject* args)
{
       PyObject *py_addr;
       PyObject *py_buffer;
       Py_ssize_t py_length;

       char * buffer;
       uint64_t size;
       uint64_t addr;
       int ret;

       if (!PyArg_ParseTuple(args, "OO", &py_addr, &py_buffer))
	       RAISE(PyExc_TypeError,"Cannot parse arguments");

       PyGetInt(py_addr, addr);

       if(!PyString_Check(py_buffer))
	       RAISE(PyExc_TypeError,"arg must be str");

       size = PyString_Size(py_buffer);
       PyString_AsStringAndSize(py_buffer, &buffer, &py_length);

       ret = vm_write_mem(&(((VmMngr*)self->pyvm)->vm_mngr), addr, buffer, size);
       if (ret < 0)
	       RAISE(PyExc_TypeError,"arg must be str");

       Py_INCREF(Py_None);
       return Py_None;
}

static PyMemberDef JitCpu_members[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef JitCpu_methods[] = {
	{"init_regs", (PyCFunction)cpu_init_regs, METH_NOARGS,
	 "X"},
	{"dump_gpregs", (PyCFunction)cpu_dump_gpregs, METH_NOARGS,
	 "X"},
	{"dump_gpregs_with_attrib", (PyCFunction)cpu_dump_gpregs_with_attrib, METH_VARARGS,
	 "X"},
	{"get_gpreg", (PyCFunction)cpu_get_gpreg, METH_NOARGS,
	 "X"},
	{"set_gpreg", (PyCFunction)cpu_set_gpreg, METH_VARARGS,
	 "X"},
	{"get_segm_base", (PyCFunction)cpu_get_segm_base, METH_VARARGS,
	 "X"},
	{"set_segm_base", (PyCFunction)cpu_set_segm_base, METH_VARARGS,
	 "X"},
	{"get_exception", (PyCFunction)cpu_get_exception, METH_VARARGS,
	 "X"},
	{"set_exception", (PyCFunction)cpu_set_exception, METH_VARARGS,
	 "X"},
	{"set_mem", (PyCFunction)vm_set_mem, METH_VARARGS,
	 "X"},
	{"get_mem", (PyCFunction)vm_get_mem, METH_VARARGS,
	 "X"},
	{"get_interrupt_num", (PyCFunction)cpu_get_interrupt_num, METH_VARARGS,
	 "X"},
	{"set_interrupt_num", (PyCFunction)cpu_set_interrupt_num, METH_VARARGS,
	 "X"},
	{NULL}  /* Sentinel */
};

static int
JitCpu_init(JitCpu *self, PyObject *args, PyObject *kwds)
{
	self->cpu = malloc(sizeof(vm_cpu_t));
	if (self->cpu == NULL) {
		fprintf(stderr, "cannot alloc vm_cpu_t\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

#define getset_reg_E_u32(regname)						\
	static PyObject *JitCpu_get_E ## regname  (JitCpu *self, void *closure) \
	{								\
		return PyLong_FromUnsignedLongLong((uint32_t)(((vm_cpu_t*)(self->cpu))->R ## regname & 0xFFFFFFFF  )); \
	}								\
	static int JitCpu_set_E ## regname  (JitCpu *self, PyObject *value, void *closure) \
	{								\
		uint64_t val;						\
		PyGetInt_retneg(value, val);				\
		val &= 0xFFFFFFFF;					\
		val |= ((vm_cpu_t*)(self->cpu))->R ##regname & 0xFFFFFFFF00000000ULL; \
		((vm_cpu_t*)(self->cpu))->R ## regname   = val;			\
		return 0;						\
	}



#define getset_reg_R_u16(regname)						\
	static PyObject *JitCpu_get_ ## regname  (JitCpu *self, void *closure) \
	{								\
		return PyLong_FromUnsignedLongLong((uint16_t)(((vm_cpu_t*)(self->cpu))->R ## regname & 0xFFFF  )); \
	}								\
	static int JitCpu_set_ ## regname  (JitCpu *self, PyObject *value, void *closure) \
	{								\
		uint64_t val;						\
		PyGetInt_retneg(value, val);				\
		val &= 0xFFFF;						\
		val |= ((vm_cpu_t*)(self->cpu))->R ##regname & 0xFFFFFFFFFFFF0000ULL; \
		((vm_cpu_t*)(self->cpu))->R ## regname   = val;			\
		return 0;						\
	}


getset_reg_u64(RAX);
getset_reg_u64(RBX);
getset_reg_u64(RCX);
getset_reg_u64(RDX);
getset_reg_u64(RSI);
getset_reg_u64(RDI);
getset_reg_u64(RSP);
getset_reg_u64(RBP);

getset_reg_u64(R8);
getset_reg_u64(R9);
getset_reg_u64(R10);
getset_reg_u64(R11);
getset_reg_u64(R12);
getset_reg_u64(R13);
getset_reg_u64(R14);
getset_reg_u64(R15);

getset_reg_u64(RIP);

getset_reg_u64(zf);
getset_reg_u64(nf);
getset_reg_u64(pf);
getset_reg_u64(of);
getset_reg_u64(cf);
getset_reg_u64(af);
getset_reg_u64(df);


getset_reg_u16(ES);
getset_reg_u16(CS);
getset_reg_u16(SS);
getset_reg_u16(DS);
getset_reg_u16(FS);
getset_reg_u16(GS);

getset_reg_E_u32(AX);
getset_reg_E_u32(BX);
getset_reg_E_u32(CX);
getset_reg_E_u32(DX);
getset_reg_E_u32(SI);
getset_reg_E_u32(DI);
getset_reg_E_u32(SP);
getset_reg_E_u32(BP);
getset_reg_E_u32(IP);

getset_reg_R_u16(AX);
getset_reg_R_u16(BX);
getset_reg_R_u16(CX);
getset_reg_R_u16(DX);
getset_reg_R_u16(SI);
getset_reg_R_u16(DI);
getset_reg_R_u16(SP);
getset_reg_R_u16(BP);

getset_reg_R_u16(IP);

getset_reg_u64(MM0);
getset_reg_u64(MM1);
getset_reg_u64(MM2);
getset_reg_u64(MM3);
getset_reg_u64(MM4);
getset_reg_u64(MM5);
getset_reg_u64(MM6);
getset_reg_u64(MM7);

getset_reg_bn(XMM0, 128);
getset_reg_bn(XMM1, 128);
getset_reg_bn(XMM2, 128);
getset_reg_bn(XMM3, 128);
getset_reg_bn(XMM4, 128);
getset_reg_bn(XMM5, 128);
getset_reg_bn(XMM6, 128);
getset_reg_bn(XMM7, 128);
getset_reg_bn(XMM8, 128);
getset_reg_bn(XMM9, 128);
getset_reg_bn(XMM10, 128);
getset_reg_bn(XMM11, 128);
getset_reg_bn(XMM12, 128);
getset_reg_bn(XMM13, 128);
getset_reg_bn(XMM14, 128);
getset_reg_bn(XMM15, 128);

getset_reg_u32(tsc1);
getset_reg_u32(tsc2);

getset_reg_u32(exception_flags);
getset_reg_u32(interrupt_num);


PyObject* get_gpreg_offset_all(void)
{
    PyObject *dict = PyDict_New();
    PyObject *o;
    get_reg_off(exception_flags);

    get_reg_off(RAX);
    get_reg_off(RBX);
    get_reg_off(RCX);
    get_reg_off(RDX);
    get_reg_off(RSI);
    get_reg_off(RDI);
    get_reg_off(RSP);
    get_reg_off(RBP);
    get_reg_off(R8);
    get_reg_off(R9);
    get_reg_off(R10);
    get_reg_off(R11);
    get_reg_off(R12);
    get_reg_off(R13);
    get_reg_off(R14);
    get_reg_off(R15);
    get_reg_off(RIP);
    get_reg_off(zf);
    get_reg_off(nf);
    get_reg_off(pf);
    get_reg_off(of);
    get_reg_off(cf);
    get_reg_off(af);
    get_reg_off(df);
    get_reg_off(tf);
    get_reg_off(i_f);
    get_reg_off(iopl_f);
    get_reg_off(nt);
    get_reg_off(rf);
    get_reg_off(vm);
    get_reg_off(ac);
    get_reg_off(vif);
    get_reg_off(vip);
    get_reg_off(i_d);
    get_reg_off(my_tick);
    get_reg_off(cond);

    get_reg_off(float_st0);
    get_reg_off(float_st1);
    get_reg_off(float_st2);
    get_reg_off(float_st3);
    get_reg_off(float_st4);
    get_reg_off(float_st5);
    get_reg_off(float_st6);
    get_reg_off(float_st7);

    get_reg_off(ES);
    get_reg_off(CS);
    get_reg_off(SS);
    get_reg_off(DS);
    get_reg_off(FS);
    get_reg_off(GS);

    get_reg_off(MM0);
    get_reg_off(MM1);
    get_reg_off(MM2);
    get_reg_off(MM3);
    get_reg_off(MM4);
    get_reg_off(MM5);
    get_reg_off(MM6);
    get_reg_off(MM7);

    get_reg_off(XMM0);
    get_reg_off(XMM1);
    get_reg_off(XMM2);
    get_reg_off(XMM3);
    get_reg_off(XMM4);
    get_reg_off(XMM5);
    get_reg_off(XMM6);
    get_reg_off(XMM7);
    get_reg_off(XMM8);
    get_reg_off(XMM9);
    get_reg_off(XMM10);
    get_reg_off(XMM11);
    get_reg_off(XMM12);
    get_reg_off(XMM13);
    get_reg_off(XMM14);
    get_reg_off(XMM15);

    get_reg_off(tsc1);
    get_reg_off(tsc2);

    get_reg_off(interrupt_num);
    get_reg_off(exception_flags);

    get_reg_off(float_stack_ptr);
    get_reg_off(reg_float_cs);
    get_reg_off(reg_float_eip);
    get_reg_off(reg_float_control);

    return dict;
}


static PyGetSetDef JitCpu_getseters[] = {
    {"vmmngr",
     (getter)JitCpu_get_vmmngr, (setter)JitCpu_set_vmmngr,
     "vmmngr",
     NULL},

    {"jitter",
     (getter)JitCpu_get_jitter, (setter)JitCpu_set_jitter,
     "jitter",
     NULL},


    {"RAX", (getter)JitCpu_get_RAX, (setter)JitCpu_set_RAX, "RAX", NULL},
    {"RBX", (getter)JitCpu_get_RBX, (setter)JitCpu_set_RBX, "RBX", NULL},
    {"RCX", (getter)JitCpu_get_RCX, (setter)JitCpu_set_RCX, "RCX", NULL},
    {"RDX", (getter)JitCpu_get_RDX, (setter)JitCpu_set_RDX, "RDX", NULL},
    {"RSI", (getter)JitCpu_get_RSI, (setter)JitCpu_set_RSI, "RSI", NULL},
    {"RDI", (getter)JitCpu_get_RDI, (setter)JitCpu_set_RDI, "RDI", NULL},
    {"RSP", (getter)JitCpu_get_RSP, (setter)JitCpu_set_RSP, "RSP", NULL},
    {"RBP", (getter)JitCpu_get_RBP, (setter)JitCpu_set_RBP, "RBP", NULL},
    {"R8",  (getter)JitCpu_get_R8,  (setter)JitCpu_set_R8,  "R8",  NULL},
    {"R9",  (getter)JitCpu_get_R9,  (setter)JitCpu_set_R9,  "R9",  NULL},
    {"R10", (getter)JitCpu_get_R10, (setter)JitCpu_set_R10, "R10", NULL},
    {"R11", (getter)JitCpu_get_R11, (setter)JitCpu_set_R11, "R11", NULL},
    {"R12", (getter)JitCpu_get_R12, (setter)JitCpu_set_R12, "R12", NULL},
    {"R13", (getter)JitCpu_get_R13, (setter)JitCpu_set_R13, "R13", NULL},
    {"R14", (getter)JitCpu_get_R14, (setter)JitCpu_set_R14, "R14", NULL},
    {"R15", (getter)JitCpu_get_R15, (setter)JitCpu_set_R15, "R15", NULL},
    {"RIP", (getter)JitCpu_get_RIP, (setter)JitCpu_set_RIP, "RIP", NULL},
    {"zf", (getter)JitCpu_get_zf, (setter)JitCpu_set_zf, "zf", NULL},
    {"nf", (getter)JitCpu_get_nf, (setter)JitCpu_set_nf, "nf", NULL},
    {"pf", (getter)JitCpu_get_pf, (setter)JitCpu_set_pf, "pf", NULL},
    {"of", (getter)JitCpu_get_of, (setter)JitCpu_set_of, "of", NULL},
    {"cf", (getter)JitCpu_get_cf, (setter)JitCpu_set_cf, "cf", NULL},
    {"af", (getter)JitCpu_get_af, (setter)JitCpu_set_af, "af", NULL},
    {"df", (getter)JitCpu_get_df, (setter)JitCpu_set_df, "df", NULL},
    {"ES", (getter)JitCpu_get_ES, (setter)JitCpu_set_ES, "ES", NULL},
    {"CS", (getter)JitCpu_get_CS, (setter)JitCpu_set_CS, "CS", NULL},
    {"SS", (getter)JitCpu_get_SS, (setter)JitCpu_set_SS, "SS", NULL},
    {"DS", (getter)JitCpu_get_DS, (setter)JitCpu_set_DS, "DS", NULL},
    {"FS", (getter)JitCpu_get_FS, (setter)JitCpu_set_FS, "FS", NULL},
    {"GS", (getter)JitCpu_get_GS, (setter)JitCpu_set_GS, "GS", NULL},

    {"EAX", (getter)JitCpu_get_EAX, (setter)JitCpu_set_EAX, "EAX", NULL},
    {"EBX", (getter)JitCpu_get_EBX, (setter)JitCpu_set_EBX, "EBX", NULL},
    {"ECX", (getter)JitCpu_get_ECX, (setter)JitCpu_set_ECX, "ECX", NULL},
    {"EDX", (getter)JitCpu_get_EDX, (setter)JitCpu_set_EDX, "EDX", NULL},
    {"ESI", (getter)JitCpu_get_ESI, (setter)JitCpu_set_ESI, "ESI", NULL},
    {"EDI", (getter)JitCpu_get_EDI, (setter)JitCpu_set_EDI, "EDI", NULL},
    {"ESP", (getter)JitCpu_get_ESP, (setter)JitCpu_set_ESP, "ESP", NULL},
    {"EBP", (getter)JitCpu_get_EBP, (setter)JitCpu_set_EBP, "EBP", NULL},
    {"EIP", (getter)JitCpu_get_EIP, (setter)JitCpu_set_EIP, "EIP", NULL},

    {"AX", (getter)JitCpu_get_AX, (setter)JitCpu_set_AX, "AX", NULL},
    {"BX", (getter)JitCpu_get_BX, (setter)JitCpu_set_BX, "BX", NULL},
    {"CX", (getter)JitCpu_get_CX, (setter)JitCpu_set_CX, "CX", NULL},
    {"DX", (getter)JitCpu_get_DX, (setter)JitCpu_set_DX, "DX", NULL},
    {"SI", (getter)JitCpu_get_SI, (setter)JitCpu_set_SI, "SI", NULL},
    {"DI", (getter)JitCpu_get_DI, (setter)JitCpu_set_DI, "DI", NULL},
    {"SP", (getter)JitCpu_get_SP, (setter)JitCpu_set_SP, "SP", NULL},
    {"BP", (getter)JitCpu_get_BP, (setter)JitCpu_set_BP, "BP", NULL},

    {"IP", (getter)JitCpu_get_IP, (setter)JitCpu_set_IP, "IP", NULL},

    {"MM0", (getter)JitCpu_get_MM0, (setter)JitCpu_set_MM0, "MM0", NULL},
    {"MM1", (getter)JitCpu_get_MM1, (setter)JitCpu_set_MM1, "MM1", NULL},
    {"MM2", (getter)JitCpu_get_MM2, (setter)JitCpu_set_MM2, "MM2", NULL},
    {"MM3", (getter)JitCpu_get_MM3, (setter)JitCpu_set_MM3, "MM3", NULL},
    {"MM4", (getter)JitCpu_get_MM4, (setter)JitCpu_set_MM4, "MM4", NULL},
    {"MM5", (getter)JitCpu_get_MM5, (setter)JitCpu_set_MM5, "MM5", NULL},
    {"MM6", (getter)JitCpu_get_MM6, (setter)JitCpu_set_MM6, "MM6", NULL},
    {"MM7", (getter)JitCpu_get_MM7, (setter)JitCpu_set_MM7, "MM7", NULL},

    {"XMM0", (getter)JitCpu_get_XMM0, (setter)JitCpu_set_XMM0, "XMM0", NULL},
    {"XMM1", (getter)JitCpu_get_XMM1, (setter)JitCpu_set_XMM1, "XMM1", NULL},
    {"XMM2", (getter)JitCpu_get_XMM2, (setter)JitCpu_set_XMM2, "XMM2", NULL},
    {"XMM3", (getter)JitCpu_get_XMM3, (setter)JitCpu_set_XMM3, "XMM3", NULL},
    {"XMM4", (getter)JitCpu_get_XMM4, (setter)JitCpu_set_XMM4, "XMM4", NULL},
    {"XMM5", (getter)JitCpu_get_XMM5, (setter)JitCpu_set_XMM5, "XMM5", NULL},
    {"XMM6", (getter)JitCpu_get_XMM6, (setter)JitCpu_set_XMM6, "XMM6", NULL},
    {"XMM7", (getter)JitCpu_get_XMM7, (setter)JitCpu_set_XMM7, "XMM7", NULL},
    {"XMM8", (getter)JitCpu_get_XMM8, (setter)JitCpu_set_XMM8, "XMM8", NULL},
    {"XMM9", (getter)JitCpu_get_XMM9, (setter)JitCpu_set_XMM9, "XMM9", NULL},
    {"XMM10", (getter)JitCpu_get_XMM10, (setter)JitCpu_set_XMM10, "XMM10", NULL},
    {"XMM11", (getter)JitCpu_get_XMM11, (setter)JitCpu_set_XMM11, "XMM11", NULL},
    {"XMM12", (getter)JitCpu_get_XMM12, (setter)JitCpu_set_XMM12, "XMM12", NULL},
    {"XMM13", (getter)JitCpu_get_XMM13, (setter)JitCpu_set_XMM13, "XMM13", NULL},
    {"XMM14", (getter)JitCpu_get_XMM14, (setter)JitCpu_set_XMM14, "XMM14", NULL},
    {"XMM15", (getter)JitCpu_get_XMM15, (setter)JitCpu_set_XMM15, "XMM15", NULL},

    {"tsc1", (getter)JitCpu_get_tsc1, (setter)JitCpu_set_tsc1, "tsc1", NULL},
    {"tsc2", (getter)JitCpu_get_tsc2, (setter)JitCpu_set_tsc2, "tsc2", NULL},

    {"exception_flags", (getter)JitCpu_get_exception_flags, (setter)JitCpu_set_exception_flags, "exception_flags", NULL},
    {"interrupt_num", (getter)JitCpu_get_interrupt_num, (setter)JitCpu_set_interrupt_num, "interrupt_num", NULL},


    {NULL}  /* Sentinel */
};


static PyTypeObject JitCpuType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "JitCore_x86.JitCpu",   /*tp_name*/
    sizeof(JitCpu),            /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)JitCpu_dealloc,/*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "JitCpu objects",          /* tp_doc */
    0,			       /* tp_traverse */
    0,			       /* tp_clear */
    0,			       /* tp_richcompare */
    0,			       /* tp_weaklistoffset */
    0,			       /* tp_iter */
    0,			       /* tp_iternext */
    JitCpu_methods,            /* tp_methods */
    JitCpu_members,            /* tp_members */
    JitCpu_getseters,          /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)JitCpu_init,     /* tp_init */
    0,                         /* tp_alloc */
    JitCpu_new,                /* tp_new */
};



static PyMethodDef JitCore_x86_Methods[] = {

	/*

	*/
	{"get_gpreg_offset_all", (PyCFunction)get_gpreg_offset_all, METH_NOARGS},
	{NULL, NULL, 0, NULL}        /* Sentinel */

};

static PyObject *JitCore_x86_Error;

PyMODINIT_FUNC
initJitCore_x86(void)
{
    PyObject *m;

    if (PyType_Ready(&JitCpuType) < 0)
	return;

    m = Py_InitModule("JitCore_x86", JitCore_x86_Methods);
    if (m == NULL)
	    return;

    JitCore_x86_Error = PyErr_NewException("JitCore_x86.error", NULL, NULL);
    Py_INCREF(JitCore_x86_Error);
    PyModule_AddObject(m, "error", JitCore_x86_Error);

    Py_INCREF(&JitCpuType);
    PyModule_AddObject(m, "JitCpu", (PyObject *)&JitCpuType);

}






















