/*
 * BackendX86.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: michi
 */

#include "BackendX86.h"
#include "Serializer.h"
#include "CommandList.h"
#include "SerialNode.h"
#include "../../os/msg.h"

namespace kaba {


// FIXME why are there ::ARM checks?!?



//bool is_typed_function_pointer(const Class *c);

BackendX86::BackendX86(Serializer *s) : Backend(s) {

	// eax, ecx, edx
	map_reg_root = {Asm::RegRoot::A, Asm::RegRoot::C, Asm::RegRoot::D};

	p_eax = param_preg(TypeReg32, Asm::RegID::EAX);
	p_eax_int = param_preg(TypeInt32, Asm::RegID::EAX);
	p_rax = param_preg(TypeReg64, Asm::RegID::RAX);

	p_deref_eax = param_deref_preg(TypePointer, Asm::RegID::EAX);

	p_ax = param_preg(TypeReg16, Asm::RegID::AX);
	p_al = param_preg(TypeReg8, Asm::RegID::AL);
	p_al_bool = param_preg(TypeBool, Asm::RegID::AL);
	p_al_char = param_preg(TypeInt8, Asm::RegID::AL);
	p_xmm0 = param_preg(TypeReg128, Asm::RegID::XMM0);
	p_xmm1 = param_preg(TypeReg128, Asm::RegID::XMM1);
}

BackendX86::~BackendX86() {
}

void BackendX86::process(Function *f, int index) {
	cur_func = f;
	cur_func_index = index;
	//call_used = false;
	stack_offset = f->_var_size;
	stack_max_size = f->_var_size;

	correct();

	do_mapping();
}

namespace x86helper {
static Asm::InstID trafo_inst_float(Asm::InstID inst, const Class *t) {
	if (t == TypeFloat64) {
		if (inst == Asm::InstID::FADD)
			return Asm::InstID::ADDSD;
		if (inst == Asm::InstID::FSUB)
			return Asm::InstID::SUBSD;
		if (inst == Asm::InstID::FMUL)
			return Asm::InstID::MULSD;
		if (inst == Asm::InstID::FDIV)
			return Asm::InstID::DIVSD;
	} else {
		if (inst == Asm::InstID::FADD)
			return Asm::InstID::ADDSS;
		if (inst == Asm::InstID::FSUB)
			return Asm::InstID::SUBSS;
		if (inst == Asm::InstID::FMUL)
			return Asm::InstID::MULSS;
		if (inst == Asm::InstID::FDIV)
			return Asm::InstID::DIVSS;
	}
	return Asm::InstID::INVALID;
}

static bool inst_is_arithmetic(Asm::InstID i) {
	if ((i == Asm::InstID::IMUL) /*or (i == Asm::InstID::IDIV)*/ or (i == Asm::InstID::ADD) or (i == Asm::InstID::SUB))
		return true;
	if ((i == Asm::InstID::AND) or (i == Asm::InstID::OR) or (i == Asm::InstID::XOR))
		return true;
	return false;
}
}

void BackendX86::correct() {
	// instead of in-place editing, let's create a backup and a new list from that
	pre_cmd.ser = cmd.ser;
	cmd.cmd.exchange(pre_cmd.cmd);
	// no vregs yet, but temp vars
	pre_cmd.temp_var = cmd.temp_var;
	for (auto &t: cmd.temp_var)
		t.first = t.last = -1;

	cmd.next_cmd_target(0);
	add_function_intro_params(cur_func);

	correct_parameters_variables_to_memory(cmd);
	correct_parameters_variables_to_memory(pre_cmd);

	// a bit pointless now (wrong list)
	serializer->cmd_list_out("x:a", "paramtrafo");

	correct_implement_commands();

	serializer->cmd_list_out("x:b", "post paramtrafo");
}

void BackendX86::correct_parameters_variables_to_memory(CommandList &cmd) {
	for (auto &c: cmd.cmd) {
		for (auto &p: c.p) {
			if (p.kind == NodeKind::None) {
			} else if (p.kind == NodeKind::VarLocal) {
				auto v = (Variable*)p.p;
				p.p = v->_offset;
				p.kind = NodeKind::LocalMemory;
			} else if (p.kind == NodeKind::VarGlobal) {
				auto v = (Variable*)p.p;
				p.p = (int_p)v->memory;
				if (!p.p)
					module->do_error_link("variable is not linkable: " + v->name);
				p.kind = NodeKind::Memory;
			} else if (p.kind == NodeKind::Constant) {
				auto cc = (Constant*)p.p;
				if (module->tree->flag_function_pointer_as_code and (p.type == TypeFunctionRef)) {
					auto *fp = (Function*)(int_p)cc->as_int64();
					p.kind = NodeKind::Label;
					p.p = fp->_label;
				} else {
					p.p = (int_p)cc->address_runtime; // FIXME ....need a cleaner approach for compiling os...
					if (config.fully_linear_output or (p.type == TypeFunctionCodeRef)) // from raw_function_pointer
						p.kind = NodeKind::Memory;
					else
						p.kind = NodeKind::ConstantByAddress;
				}
			} else if (p.kind == NodeKind::Label) {
			} else {
				//msg_write(p.str(serializer));
			}
		}
	}
}

void BackendX86::implement_return(const SerialNodeParam &p) {
	if (p.kind != NodeKind::None) {
		if (cur_func->effective_return_type->_return_in_float_registers()) {
			// if ((config.instruction_set == Asm::INSTRUCTION_SET_AMD64) or (config.compile_os)) ???
			//		cmd.add_cmd(Asm::InstID::FLD, t);
			if (cur_func->effective_return_type == TypeFloat32) {
				insert_cmd(Asm::InstID::MOVSS, p_xmm0, p);
			} else if (cur_func->effective_return_type == TypeFloat64) {
				insert_cmd(Asm::InstID::MOVSD, p_xmm0, p);
			} else if (cur_func->effective_return_type->size == 8) {
				// float[2]
				insert_cmd(Asm::InstID::MOVLPS, p_xmm0, p);
			} else if (cur_func->effective_return_type->size == 12) {
				// float[3]
				insert_cmd(Asm::InstID::MOVLPS, p_xmm0, param_shift(p, 0, TypeReg64));
				insert_cmd(Asm::InstID::MOVSS, p_xmm1, param_shift(p, 8, TypeFloat32));
			} else if (cur_func->effective_return_type->size == 16) {
				// float[4]
				insert_cmd(Asm::InstID::MOVLPS, p_xmm0, param_shift(p, 0, TypeReg64));
				insert_cmd(Asm::InstID::MOVLPS, p_xmm1, param_shift(p, 8, TypeReg64));
			} else {
				do_error("...ret xmm " + cur_func->effective_return_type->long_name());
			}
		} else {
			// store return directly in eax / fpu stack (4 byte)
			if (cur_func->effective_return_type->size == 1) {
				int v = cmd.add_virtual_reg(Asm::RegID::AL);
				insert_cmd(Asm::InstID::MOV, param_vreg(cur_func->effective_return_type, v), p);
			} else if (cur_func->effective_return_type->size == 8) {
				int v = cmd.add_virtual_reg(Asm::RegID::RAX);
				insert_cmd(Asm::InstID::MOV, param_vreg(cur_func->effective_return_type, v), p);
			} else {
				int v = cmd.add_virtual_reg(Asm::RegID::EAX);
				insert_cmd(Asm::InstID::MOV, param_vreg(cur_func->effective_return_type, v), p);
			}
		}
	}
	insert_cmd(Asm::InstID::LEAVE);
	//if (cur_func->effective_return_type->uses_return_by_memory())
	//	insert_cmd(Asm::InstID::RET, param_imm(TypeReg16, 4));
	//else
	insert_cmd(Asm::InstID::RET);
}

void BackendX86::implement_mov_chunk(const SerialNodeParam &p1, const SerialNodeParam &p2, int size) {
	//auto p1 = c.p[0];
	//auto p2 = c.p[1];
	//cmd.remove_cmd(i);
	//cmd.next_cmd_target(i);
	//msg_error("CORRECT MOV " + p1.type->name);

	for (int j=0; j<size/4; j++)
		insert_cmd(Asm::InstID::MOV, param_shift(p1, j * 4, TypeInt32), param_shift(p2, j * 4, TypeInt32));
	for (int j=4*(size/4); j<size; j++)
		insert_cmd(Asm::InstID::MOV, param_shift(p1, j, TypeInt8), param_shift(p2, j, TypeInt8));
}

void BackendX86::correct_implement_commands() {

	Array<SerialNodeParam> func_params;

	for (int _i=0; _i<pre_cmd.cmd.num; _i++) {
		auto &c = pre_cmd.cmd[_i];
		cmd.next_cmd_index = cmd.cmd.num;
		if (c.inst == Asm::InstID::MOV) {
			int size = c.p[0].type->size;
			auto p1 = c.p[0];
			auto p2 = c.p[1];

			// mov can only copy these sizes (ignore 2...)
			//if (size != 1 and size != 4 and size != config.pointer_size) {
				implement_mov_chunk(p1, p2, size);
			//}
		} else if (c.inst == Asm::InstID::MOVSX or c.inst == Asm::InstID::MOVZX) {
			// only  (i8 <-> i32)  or  (i32 <-> i64)
			auto inst = c.inst;
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			if (p1.type == TypeInt64 and p2.type == TypeInt32) {
				// i64 <- i32
				int reg = find_unused_reg(cmd.cmd.num, cmd.cmd.num, p2.type->size);
				insert_cmd(Asm::InstID::MOV, param_vreg(p2.type, reg), p2);
				auto preg_x = reg_resize(cmd.virtual_reg[reg].reg, p1.type->size);
				insert_cmd(Asm::InstID::MOVSXD, param_vreg(p1.type, reg, preg_x), param_vreg(p2.type, reg));
				insert_cmd(Asm::InstID::MOV, p1, param_vreg(p1.type, reg, preg_x));
			} else if (p1.type == TypeInt32 and (p2.type == TypeInt8 or p2.type == TypeUInt8)) {
				// i32 <- i8/u8
				int reg = find_unused_reg(cmd.cmd.num, cmd.cmd.num, max(p1.type->size, p2.type->size));
				auto preg = reg_resize(cmd.virtual_reg[reg].reg, p1.type->size);
				insert_cmd(inst, param_vreg(p1.type, reg, preg), p2);
				insert_cmd(Asm::InstID::MOV, p1, param_vreg(p1.type, reg, preg));
				if (p2.type == TypeUInt8)
					insert_cmd(Asm::InstID::AND, p1, param_imm(TypeInt32, 0x000000ff));
			} else {
				// i8 <- i32
				// i32 <- i64
				int reg = find_unused_reg(cmd.cmd.num, cmd.cmd.num, p2.type->size);
				insert_cmd(Asm::InstID::MOV, param_vreg(p2.type, reg), p2);
				auto preg_x = reg_resize(cmd.virtual_reg[reg].reg, p1.type->size);
				insert_cmd(Asm::InstID::MOV, p1, param_vreg(p1.type, reg, preg_x));
			}

		} else if (c.inst == Asm::InstID::LEA) {
			auto p0 = c.p[0];
			auto p1 = c.p[1];
			if ((p1.kind == NodeKind::LocalMemory) or (p1.kind == NodeKind::VarTemp)) {
				insert_lea(p0, p1);
			} else {
				do_error("illegal reference to " + p1.str(serializer));
			}
		} else if (c.inst == Asm::InstID::MODULO) {
			auto r = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			if (p1.type == TypeInt32) {
				int veax = cmd.add_virtual_reg(Asm::RegID::EAX);
				int vedx = cmd.add_virtual_reg(Asm::RegID::EDX);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt32, veax), p1);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt32, vedx), param_vreg(TypeInt32, veax));
				insert_cmd(Asm::InstID::SAR, param_vreg(TypeInt32, vedx), param_imm(TypeInt8, 0x1f));
				insert_cmd(Asm::InstID::IDIV, param_vreg(TypeInt32, veax), p2);
				insert_cmd(Asm::InstID::MOV, r, param_vreg(TypeInt32, vedx));
			} else { // int64
				int vrax = cmd.add_virtual_reg(Asm::RegID::RAX);
				int vrdx = cmd.add_virtual_reg(Asm::RegID::RDX);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt64, vrax), p1);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt64, vrdx), param_vreg(TypeInt64, vrax));
				insert_cmd(Asm::InstID::SAR, param_vreg(TypeInt64, vrdx), param_imm(TypeInt8, 0x3f));
				insert_cmd(Asm::InstID::IDIV, param_vreg(TypeInt64, vrax), p2);
				insert_cmd(Asm::InstID::MOV, r, param_vreg(TypeInt64, vrdx));
			}
		} else if (c.inst == Asm::InstID::IDIV) {
			auto r = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			if (p2 == p_none) {
				p2 = p1;
				p1 = r;
			}
			if (p1.type == TypeInt32) {
				int veax = cmd.add_virtual_reg(Asm::RegID::EAX);
				int vedx = cmd.add_virtual_reg(Asm::RegID::EDX);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt32, veax), p1);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt32, vedx), param_vreg(TypeInt32, veax));
				insert_cmd(Asm::InstID::SAR, param_vreg(TypeInt32, vedx), param_imm(TypeInt8, 0x1f));
				insert_cmd(Asm::InstID::IDIV, param_vreg(TypeInt32, veax), p2);
				insert_cmd(Asm::InstID::MOV, r, param_vreg(TypeInt32, veax));
			} else { // int64
				int vrax = cmd.add_virtual_reg(Asm::RegID::RAX);
				int vrdx = cmd.add_virtual_reg(Asm::RegID::RDX);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt64, vrax), p1);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypeInt64, vrdx), param_vreg(TypeInt64, vrax));
				insert_cmd(Asm::InstID::SAR, param_vreg(TypeInt64, vrdx), param_imm(TypeInt8, 0x3f));
				insert_cmd(Asm::InstID::IDIV, param_vreg(TypeInt64, vrax), p2);
				insert_cmd(Asm::InstID::MOV, r, param_vreg(TypeInt64, vrax));
			}
		} else if ((c.inst == Asm::InstID::SHL) or (c.inst == Asm::InstID::SHR)) {
			auto inst = c.inst;
			auto r = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			auto type = p1.type;
			int vecx;
			if (type == TypeInt64) {
				vecx = cmd.add_virtual_reg(Asm::RegID::RCX);
			} else {
				vecx = cmd.add_virtual_reg(Asm::RegID::ECX);
			}
			insert_cmd(Asm::InstID::MOV, param_vreg(type, vecx), p2);
			insert_cmd(Asm::InstID::MOV, r, p1);
			insert_cmd(inst, r, param_vreg(TypeInt8, vecx, Asm::RegID::CL));
		} else if (x86helper::inst_is_arithmetic(c.inst)) {
			if (c.p[2].kind == NodeKind::None) {
				insert_cmd(c.inst, c.p[0], c.p[1], c.p[2]); // cmd.cmd.add(c);
				continue;
			}
			auto inst = c.inst;
			auto r = c.p[0];
			auto p1 = c.p[1];
			auto p2 = c.p[2];
			auto type = p1.type;
			int reg = find_unused_reg(cmd.cmd.num, cmd.cmd.num, type->size);

			auto t = param_vreg(type, reg);
			insert_cmd(Asm::InstID::MOV, t, p1);
			insert_cmd(inst, t, p2);
			insert_cmd(Asm::InstID::MOV, r, t);
		} else if (c.inst == Asm::InstID::CMP) {
			if (c.p[0].type->size > config.target.pointer_size and false) {
				do_error("chunk cmp... currently done by the Serializer!");
				// chunk cmp
				auto p1 = c.p[1];
				auto p2 = c.p[2];
				int i0 = cmd.cmd.num;

				[[maybe_unused]] int label_after_cmp = list->create_label("_CMP_AFTER_" + i2s(serializer->num_labels ++));

				int reg = find_unused_reg(i0, i0, 1);
				auto t = param_vreg(TypeBool, reg);

				insert_cmd(Asm::InstID::CMP, param_shift(p1, 0, TypeInt32), param_shift(p2, 0, TypeInt32));
				insert_cmd(Asm::InstID::SETZ, t);
				for (int k=1; k<p1.type->size/4; k++) {
					insert_cmd(Asm::InstID::CMP, param_shift(p1, k*4, TypeInt32), param_shift(p2, k*4, TypeInt32));
					//insert_cmd(Asm::InstID::SETZ, param_vreg(TypeBool, val));
					//insert_cmd(Asm::InstID::AND, param_vreg(TypeBool, val));
				}
			} else {
				insert_cmd(c.inst, c.p[0], c.p[1], c.p[2]); // cmd.cmd.add(c);
			}
/*			// TODO also check p[0]
			if (((c.p[1].kind == NodeKind::CONSTANT_BY_ADDRESS) or (c.p[1].kind == NodeKind::IMMEDIATE)) and (c.p[1].type->size == 8)) {
				int64 ii = c.p[1].p;
				if (c.p[1].kind == NodeKind::CONSTANT_BY_ADDRESS)
					ii = *(int64*)c.p[1].p;
				if ((ii & 0xffffffff00000000) == 0) {
					c.p[1].type = TypeInt32;
				} else {
					auto p1 = c.p[1];

					int reg = find_unused_reg(i, i, p1.type->size);
					auto t = param_vreg(p1.type, reg);
					//cmd.remove_cmd(i);
					cmd.next_cmd_target(i);
					insert_cmd(Asm::InstID::MOV, t, p1);
					cmd.set_cmd_param(i + 1, 1, t);
					cmd.set_virtual_reg(reg, i, i + 1);
					i ++;

					//do_error("cmp immediate > 32bit");
				}
			}*/
		} else if ((c.inst == Asm::InstID::FMUL) or (c.inst == Asm::InstID::FDIV) or (c.inst == Asm::InstID::FADD) or (c.inst == Asm::InstID::FSUB)) {
			auto inst = c.inst;
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			auto p3 = c.p[2];

			inst = x86helper::trafo_inst_float(inst, p1.type);
			auto inst_mov = (p1.type == TypeFloat64) ? Asm::InstID::MOVSD : Asm::InstID::MOVSS;

			if (p3.kind == NodeKind::None) {
				// a += b
				insert_cmd(inst_mov, p_xmm0, p1);
				insert_cmd(inst, p_xmm0, p2);
				insert_cmd(inst_mov, p1, p_xmm0);
			} else {
				// a = b + c
				insert_cmd(inst_mov, p_xmm0, p2);
				insert_cmd(inst, p_xmm0, p3);
				insert_cmd(inst_mov, p1, p_xmm0);
			}
		} else if (c.inst == Asm::InstID::FCMP) {
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			if (p1.type == TypeFloat64) {
				insert_cmd(Asm::InstID::MOVSD, p_xmm0, p1);
				insert_cmd(Asm::InstID::UCOMISD, p_xmm0, p2);
			} else {
				insert_cmd(Asm::InstID::MOVSS, p_xmm0, p1);
				insert_cmd(Asm::InstID::UCOMISS, p_xmm0, p2);
			}
		} else if (c.inst == Asm::InstID::CVTSI2SS) {
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			insert_cmd(Asm::InstID::CVTSI2SS, p_xmm0, p2);
			insert_cmd(Asm::InstID::MOVSS, p1, p_xmm0);
		} else if (c.inst == Asm::InstID::CVTTSS2SI) {
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			int veax = cmd.add_virtual_reg(Asm::RegID::EAX);
			insert_cmd(Asm::InstID::MOVSS, p_xmm0, p2);
			insert_cmd(Asm::InstID::CVTTSS2SI, param_vreg(TypeInt32, veax), p_xmm0);
			insert_cmd(Asm::InstID::MOV, p1, param_vreg(TypeInt32, veax));

		} else if (c.inst == Asm::InstID::CVTSS2SD) {
			// f32 -> f64
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			[[maybe_unused]] int veax = cmd.add_virtual_reg(Asm::RegID::XMM0);
			insert_cmd(Asm::InstID::CVTSS2SD, p_xmm0, p2);
			insert_cmd(Asm::InstID::MOVSD, p1, p_xmm0);
		} else if (c.inst == Asm::InstID::CVTSD2SS) {
			// f64 -> f32
			auto p1 = c.p[0];
			auto p2 = c.p[1];
			[[maybe_unused]] int veax = cmd.add_virtual_reg(Asm::RegID::XMM0);
			insert_cmd(Asm::InstID::CVTSD2SS, p_xmm0, p2);
			insert_cmd(Asm::InstID::MOVSS, p1, p_xmm0);
		} else if (c.inst == Asm::InstID::PUSH) {
			func_params.add(c.p[0]);
		} else if ((c.inst == Asm::InstID::CALL) or (c.inst == Asm::InstID::CALL_MEMBER)) {

			if (c.p[1].type == TypeFunctionCodeRef) {
				//do_error("indirect call...");
				auto fp = c.p[1];
				auto ret = c.p[0];
				add_pointer_call(fp, func_params, ret, (c.inst == Asm::InstID::CALL));
//			} else if (is_typed_function_pointer(c.p[1].type)) {
//				do_error("BACKEND: POINTER CALL");
			} else {
				//func_params.add(c.p[0]);
				auto *f = ((Function*)c.p[1].p);
				auto ret = c.p[0];
				add_function_call(f, func_params, ret);
			}
			func_params.clear();
		} else if (c.inst == Asm::InstID::RET) {
			implement_return(c.p[0]);
		} else {
			insert_cmd(c.inst, c.p[0], c.p[1], c.p[2]); // cmd.cmd.add(c);
		}
	}
}


void BackendX86::function_call_post(int push_size, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	const Class *type = ret.get_type_save();

	if (push_size > 127)
		insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::ESP), param_imm(TypeInt32, push_size));
	else if (push_size > 0)
		insert_cmd(Asm::InstID::ADD, param_preg(TypePointer, Asm::RegID::ESP), param_imm(TypeInt8, push_size));

	// return > 4b already got copied to [ret] by the function!
	if ((type != TypeVoid) and !type->uses_return_by_memory()) {
		if (type == TypeFloat32) {
			if (true) //config.compile_os)
				insert_cmd(Asm::InstID::MOVSS, ret, p_xmm0);
			else
				insert_cmd(Asm::InstID::FSTP, ret);
		} else if (type->size == 1) {
			int v = cmd.add_virtual_reg(Asm::RegID::AL);
			insert_cmd(Asm::InstID::MOV, ret, param_vreg(type, v));
			cmd.set_virtual_reg(v, cmd.cmd.num - 2, cmd.cmd.num - 1);
		} else {
			int v = cmd.add_virtual_reg(Asm::RegID::EAX);
			insert_cmd(Asm::InstID::MOV, ret, param_vreg(type, v));
			cmd.set_virtual_reg(v, cmd.cmd.num - 2, cmd.cmd.num - 1);
		}
	}
}

void BackendX86::add_function_call(Function *f, const Array<SerialNodeParam> &params, const SerialNodeParam &ret) {
	serializer->call_used = true;
	int push_size = function_call_pre(params, ret, f->is_static());

	if (f->address != 0) {
		insert_cmd(Asm::InstID::CALL, param_imm(TypeReg32, f->address)); // the actual call
		// function pointer will be shifted later...(asm translates to RIP-relative)
	} else if (f->_label >= 0) {
		insert_cmd(Asm::InstID::CALL, param_label(TypeInt32, f->_label));
	} else {
		serializer->do_error_link("could not link function " + f->signature());
	}
	extend_reg_usage_to_call(cmd.next_cmd_index - 1);
	mark_regs_busy_at_call(cmd.next_cmd_index - 1);

	function_call_post(push_size, params, ret);
}

void BackendX86::add_pointer_call(const SerialNodeParam &fp, const Array<SerialNodeParam> &params, const SerialNodeParam &ret, bool is_static) {
	serializer->call_used = true;
	int push_size = function_call_pre(params, ret, is_static);

	insert_cmd(Asm::InstID::MOV, p_eax, fp);
	insert_cmd(Asm::InstID::CALL, p_eax);
	extend_reg_usage_to_call(cmd.next_cmd_index - 1);
	mark_regs_busy_at_call(cmd.next_cmd_index - 1);

	function_call_post(push_size, params, ret);
}

int BackendX86::function_call_pre(const Array<SerialNodeParam> &_params, const SerialNodeParam &ret, bool is_static) {
	const Class *type = ret.get_type_save();

	auto params = _params;

	// return data too big... push address
	SerialNodeParam ret_ref;
	if (type->uses_return_by_memory()) {
		ret_ref = insert_reference(ret);
	}

	// grow stack (down) for local variables of the calling function
//	cmd.add_cmd(- cur_func->_VarSize - LocalOffset - 8);
	int64 push_size = 0;

	// skip the class instance for now...
	int p0 = 0;
	if (!is_static)
		p0 = 1;

	// push parameters onto stack
	for (int p=params.num-1; p>=p0; p--) {
		if (params[p].type) {
			int s = mem_align(params[p].type->size, 4);
			for (int j=0; j<s/4; j++)
				insert_cmd(Asm::InstID::PUSH, param_shift(params[p], s - 4 - j * 4, TypeInt32));
			push_size += s;
		}
	}

	if (config.target.abi == Abi::X86_WINDOWS) {
		// more than 4 byte have to be returned -> give return address as very last parameter!
		if (type->uses_return_by_memory())
			insert_cmd(Asm::InstID::PUSH, ret_ref); // nachtraegliche eSP-Korrektur macht die Funktion
	}

	// _cdecl: push class instance as first parameter
	if (!is_static) {
		insert_cmd(Asm::InstID::PUSH, params[0]);
		push_size += config.target.pointer_size;
	}

	if (config.target.abi == Abi::X86_GNU) {
		// more than 4 byte have to be returned -> give return address as very first parameter!
		if (type->uses_return_by_memory())
			insert_cmd(Asm::InstID::PUSH, ret_ref); // nachtraegliche eSP-Korrektur macht die Funktion
	}
	return push_size;
}

void BackendX86::mark_regs_busy_at_call(int index) {
	// call violates all used registers...
	for (int i=0; i<map_reg_root.num; i++) {
		int v = cmd.add_virtual_reg(get_reg(map_reg_root[i], 4));
		cmd.use_virtual_reg(v, index, index);
	}
}

void BackendX86::extend_reg_usage_to_call(int index) {
	for (int v: func_param_virts)
		cmd.use_virtual_reg(v, index, index);
}

SerialNodeParam BackendX86::insert_reference(const SerialNodeParam &param, const Class *type) {
	SerialNodeParam ret;
	if (!type)
		type = module->tree->type_ref(param.type, -1);
	ret.type = type;
	ret.shift = 0;
	if (param.kind == NodeKind::ConstantByAddress) {
		return param_imm(type, (int_p)((char*)param.p + param.shift));

	} else if ((param.kind == NodeKind::Immediate) or (param.kind == NodeKind::Memory)) {
		if (param.shift > 0)
			msg_error("Serializer: immediade/mem + shift?!?!?");
		return param_imm(type, param.p);

	} else if (param.kind == NodeKind::DereferenceVarTemp) {
		ret = param;
		ret.kind = NodeKind::VarTemp; // FIXME why was it param.kind ?!?!?


	} else if ((param.kind == NodeKind::LocalMemory) or (param.kind == NodeKind::VarTemp)) {
		ret = cmd._add_temp(type);
		insert_lea(ret, param);
	} else {
		do_error("illegal reference to: " + param.str(serializer));
	}
	return ret;
}

void BackendX86::insert_lea(const SerialNodeParam &p1, const SerialNodeParam &p2) {

	if (config.target.instruction_set == Asm::InstructionSet::AMD64) {
		int r = cmd.add_virtual_reg(Asm::RegID::RAX);
		insert_cmd(Asm::InstID::LEA, param_vreg(TypeReg64, r), p2);
		insert_cmd(Asm::InstID::MOV, p1, param_vreg(TypeReg64, r));
	} else {
		int r = cmd.add_virtual_reg(Asm::RegID::EAX);
		insert_cmd(Asm::InstID::LEA, param_vreg(TypeReg32, r), p2);
		insert_cmd(Asm::InstID::MOV, p1, param_vreg(TypeReg32, r));
	}
}


void BackendX86::add_function_outro(Function *f) {
	insert_cmd(Asm::InstID::LEAVE, p_none);
	if (f->effective_return_type->uses_return_by_memory())
		insert_cmd(Asm::InstID::RET, param_imm(TypeReg16, 4));
	else
		insert_cmd(Asm::InstID::RET, p_none);
}

void BackendX86::add_function_intro_params(Function *f) {
}


bool BackendX86::dist_fits_32bit(int64 a, void *b) {
	int_p d = (int_p)a - (int_p)b;
	if (d < 0)
		d = -d;
	return (d < 0x70000000);
}

void BackendX86::correct_far_mem_access() {

	for (int i=0; i<cmd.cmd.num; i++) {
		auto &c = cmd.cmd[i];

		if (c.p[1].kind == NodeKind::ConstantByAddress) {
			if (!dist_fits_32bit(c.p[1].p + c.p[1].shift, module->opcode)) {
				auto p1 = c.p[1];


				// will be converted to immediate anyways...?
				bool imm_allowed = Asm::get_instruction_allow_const(c.inst);
				if (imm_allowed and (c.p[1].type->size <= 4))
					continue;

				int reg = find_unused_reg(i, i, config.target.pointer_size);

				cmd.next_cmd_target(i);
				insert_cmd(Asm::InstID::MOV, param_vreg(TypePointer, reg), param_imm(TypePointer, p1.p + p1.shift)); // prepare input into register
				cmd.set_cmd_param(i+1, 1, param_deref_vreg(p1.type, reg)); // change input in original instruction
				cmd.set_virtual_reg(reg, i, i + 1);
			}
		}
	}
}


void BackendX86::do_mapping() {

	map_referenced_temp_vars_to_stack();



	try_map_temp_vars_to_registers();

	serializer->cmd_list_out("map:a", "post temp -> reg");

	map_remaining_temp_vars_to_stack();

	serializer->cmd_list_out("map:b", "post temp -> stack");

	resolve_deref_temp_and_local();

	serializer->cmd_list_out("map:c", "post deref t&l");

	correct_params_indirect_in();

	serializer->cmd_list_out("map:d", "unallowed");

	/*

	if (config.allow_simplification){
	SimplifyMovs();

	SimplifyFPUStack();
	}
	*/


	for (int i=0; i<cmd.cmd.num; i++)
		correct_unallowed_param_combis2(cmd.cmd[i]);

	if (config.target.instruction_set == Asm::InstructionSet::AMD64)
		correct_far_mem_access();

	serializer->cmd_list_out("map:z", "end");
}

void BackendX86::correct_unallowed_param_combis2(SerialNode &c) {
	if (c.inst == Asm::InstID::CMP)
		if ((c.p[1].kind == NodeKind::Immediate) and (c.p[1].type->size == 8)) {
			if ((c.p[1].p & 0xffffffff00000000) != 0)
				do_error("cmp immediate > 32bit");
			c.p[1].type = TypeInt32;
		}
}


namespace x86helper {
inline bool param_is_simple(SerialNodeParam &p) {
	return ((p.kind == NodeKind::Register) or (p.kind == NodeKind::VarTemp) or (p.kind == NodeKind::None));
}

inline bool param_combi_allowed(Asm::InstID inst, SerialNodeParam &p1, SerialNodeParam &p2) {
//	if (inst >= Asm::inst_label)
//		return true;
	if ((!param_is_simple(p1)) and (!param_is_simple(p2)))
		return false;
	bool r1, w1, r2, w2;
	Asm::get_instruction_param_flags(inst, r1, w1, r2, w2);
	if (w1 and (p1.kind == NodeKind::Immediate))
		return false;
	if (w2 and (p2.kind == NodeKind::Immediate))
		return false;
	if ((p1.kind == NodeKind::Immediate) or (p2.kind == NodeKind::Immediate))
		if (!Asm::get_instruction_allow_const(inst))
			return false;
	return true;
}

inline void try_map_param_to_stack(SerialNodeParam &p, int v, SerialNodeParam &stackvar) {
	if ((p.kind == NodeKind::VarTemp) and (p.p == v)) {
		p.kind = NodeKind::LocalMemory;//stackvar.kind;
		p.p = stackvar.p;
	} else if ((p.kind == NodeKind::DereferenceVarTemp) and (p.p == v)) {
		p.kind = NodeKind::DerefereceLocalMemory;
		p.p = stackvar.p;
	}
}
}

void BackendX86::map_referenced_temp_vars_to_stack() {
	for (SerialNode &c: cmd.cmd)
		if (c.inst == Asm::InstID::LEA)
			if (c.p[1].kind == NodeKind::VarTemp) {
				int v = c.p[1].p;
//				msg_error("ref b " + i2s(v));
				cmd.temp_var[v].referenced = true;
				cmd.temp_var[v].force_stack = true;
			}

	for (int i=cmd.temp_var.num-1;i>=0;i--) {
		if (!cmd.temp_var[i].force_stack)
			continue;
		SerialNodeParam stackvar;
		add_stack_var(cmd.temp_var[i], stackvar);
		for (int j=0;j<cmd.cmd.num;j++) {
			for (int k=0; k<SERIAL_NODE_NUM_PARAMS; k++)
				x86helper::try_map_param_to_stack(cmd.cmd[j].p[k], i, stackvar);
		}
		cmd.remove_temp_var(i);
	}
}

void BackendX86::try_map_temp_vars_to_registers() {
	for (int i=cmd.temp_var.num-1;i>=0;i--) {
		if (cmd.temp_var[i].force_stack)
			continue;
	}
}

void BackendX86::map_remaining_temp_vars_to_stack() {
	for (int i=cmd.temp_var.num-1;i>=0;i--) {
		SerialNodeParam stackvar;
		add_stack_var(cmd.temp_var[i], stackvar);
		for (int j=0;j<cmd.cmd.num;j++) {
			for (int k=0; k<SERIAL_NODE_NUM_PARAMS; k++)
				x86helper::try_map_param_to_stack(cmd.cmd[j].p[k], i, stackvar);
		}
		//cmd.remove_temp_var(i); // nah, no need, it's faster to just clear them all!
	}
	cmd.temp_var.clear();
}



void BackendX86::add_stack_var(TempVar &v, SerialNodeParam &p) {
	// find free stack space for the life span of the variable....
	// don't mind... use old algorithm: ALWAYS grow stack... remove ALL on each command in a block

	int s = mem_align(v.type->size, 4);
//	msg_write(format("add stack var  %s %d   %d-%d       vs=%d", v.type->name.c_str(), v.type->size, v.first, v.last, cur_func->_var_size));
//	for (auto&& [i, t]: temp_var)
//		if (&t == &v)
//			msg_write("#" + i2s(i));

	if (true) {
		// TODO super important!!!!!!
		if (config.target.instruction_set == Asm::InstructionSet::ARM32) {
			v.stack_offset = stack_offset;
			stack_offset += s;

		} else {
			stack_offset += s;
			v.stack_offset = - stack_offset;
		}
	} else {
		StackOccupationX so;
		so.create(serializer, (config.target.instruction_set != Asm::InstructionSet::ARM32), cur_func->_var_size, v.first, v.last);
		v.stack_offset = so.find_free(v.type->size);
		if (config.target.instruction_set == Asm::InstructionSet::ARM32) {
			stack_offset = v.stack_offset + s;
		} else {
			stack_offset = - v.stack_offset;
		}
	}
//	msg_write("=>");
//	msg_write(v.stack_offset);

	if (stack_offset > stack_max_size)
		stack_max_size = stack_offset;

	v.mapped = true;

	p.kind = NodeKind::LocalMemory;
	p.p = v.stack_offset;
	p.type = v.type;
	p.shift = 0;
}


// mov [0x..] [0x...]  ->  mov eax, [0x..]   mov [0x..] eax    (etc)
void BackendX86::correct_params_indirect_in() {
	for (int i=cmd.cmd.num-1;i>=0;i--){
		auto &c = cmd.cmd[i];
		if (c.inst >= Asm::InstID::LABEL)
			continue;

		// bad?
		if (x86helper::param_combi_allowed(c.inst, c.p[0], c.p[1]))
			continue;

#if !defined(NDEBUG)
		if ((c.p[0].kind == NodeKind::ConstantByAddress) or (c.p[0].kind == NodeKind::Immediate))
			if (c.inst != Asm::InstID::CMP)
				do_error("output into const: " + c.str(serializer));
#endif

		// correct
		bool mov_first_param = (c.p[1].kind == NodeKind::None) or (c.p[0].kind == NodeKind::ConstantByAddress) or (c.p[0].kind == NodeKind::Immediate);
		int p_index = mov_first_param ? 0 : 1;
		SerialNodeParam p = c.p[p_index];

#if !defined(NDEBUG)
		if (p.type->name == "color")
			do_error("color in assembler..." + serializer->cur_func->long_name());
#endif

		int reg = find_unused_reg(i, i, p.type->size);
		auto p_reg = param_vreg(p.type, reg);

		cmd.next_cmd_target(i);
		insert_cmd(Asm::InstID::MOV, p_reg, p); // prepare input into register
		cmd.set_cmd_param(i+1, p_index, p_reg); // change input in original instruction
		cmd.set_virtual_reg(reg, i, i + 1);
	}
	scan_temp_var_usage();
}

// inst ... [local] ...
// ->      mov reg, local     inst ...[reg]...
void BackendX86::solve_deref_temp_local(int c, int np, bool is_local) {
	SerialNodeParam p = cmd.cmd[c].p[np];
	int shift = p.shift;

	const Class *type_pointer = is_local ? TypePointer : cmd.temp_var[p.p].type;
	const Class *type_data = p.type;

	p.kind = is_local ? NodeKind::LocalMemory : NodeKind::VarTemp;
	p.shift = 0;
	p.type = type_pointer;

	int reg = find_unused_reg(c, c, config.target.pointer_size);
	if (reg < 0)
		module->do_error_internal("solve_deref_temp_local... no registers available");
	SerialNodeParam p_reg = param_vreg(type_pointer, reg);
	SerialNodeParam p_deref_reg = param_deref_vreg(type_data, reg);

	cmd.set_cmd_param(c, np, p_deref_reg);

	cmd.next_cmd_target(c);
	insert_cmd(Asm::InstID::MOV, p_reg, p);
	if (shift > 0) {
		// solve_deref_temp_local
		insert_cmd(Asm::InstID::ADD, p_reg, param_imm(TypeInt32, shift));
		cmd.set_virtual_reg(reg, c, c+2);
	} else {
		cmd.set_virtual_reg(reg, c, c+1);
	}
}


void BackendX86::resolve_deref_temp_and_local() {
	for (int i=cmd.cmd.num-1;i>=0;i--) {
		if (cmd.cmd[i].inst >= Asm::InstID::LABEL)
			continue;
		bool dl1 = ((cmd.cmd[i].p[0].kind == NodeKind::DerefereceLocalMemory) or (cmd.cmd[i].p[0].kind == NodeKind::DereferenceVarTemp));
		bool dl2 = ((cmd.cmd[i].p[1].kind == NodeKind::DerefereceLocalMemory) or (cmd.cmd[i].p[1].kind == NodeKind::DereferenceVarTemp));
		if (!(dl1 or dl2))
			continue;

		bool is_local1 = (cmd.cmd[i].p[0].kind == NodeKind::DerefereceLocalMemory);
		bool is_local2 = (cmd.cmd[i].p[1].kind == NodeKind::DerefereceLocalMemory);

		//msg_write(format("deref temp/local... cmd=%d", i));
		if (!dl2) {
			solve_deref_temp_local(i, 0, is_local1);
			i ++;
		} else if (!dl1) {
			solve_deref_temp_local(i, 1, is_local2);
			i ++;
		} else {
			// hopefully... p2 is read-only

			const Class *type_pointer = TypePointer;
			const Class *type_data = cmd.cmd[i].p[0].type;

			int reg = find_unused_reg(i, i, type_data->size);
			if (reg < 0)
				do_error("deref local... both sides... .no registers available");

			SerialNodeParam p_reg = param_vreg(type_data, reg);

			int reg2 = find_unused_reg(i, i, config.target.pointer_size, cmd.virtual_reg[reg].reg_root);
			if (reg2 < 0)
				do_error("deref temp/local... both sides... .no registers available");
			SerialNodeParam p_reg2 = param_vreg(type_pointer, reg2);
			SerialNodeParam p_deref_reg2 = param_deref_vreg(type_data, reg2);

			// inst [l1] [l2]
			// ->
			// mov reg2, l2
			//   (add reg2, shift2)
			// mov reg, [reg2]
			// mov reg2, l1
			//   (add reg2, shift1)
			// inst [reg2], reg
			SerialNodeParam p1 = cmd.cmd[i].p[0];
			SerialNodeParam p2 = cmd.cmd[i].p[1];
			int shift1 = p1.shift;
			int shift2 = p2.shift;
			p1.shift = p2.shift = 0;

			p1.kind = is_local1 ? NodeKind::LocalMemory : NodeKind::VarTemp;
			p2.kind = is_local2 ? NodeKind::LocalMemory : NodeKind::VarTemp;
			p1.type = type_pointer;
			p2.type = type_pointer;
			cmd.set_cmd_param(i, 0, p_deref_reg2);
			cmd.set_cmd_param(i, 1, p_reg);
			int cmd_pos = i;

			int r2_first = cmd_pos;
			cmd.next_cmd_target(cmd_pos ++);
			insert_cmd(Asm::InstID::MOV, p_reg2, p2);

			if (shift2 > 0) {
				// resolve deref temp&loc 2
				cmd.next_cmd_target(cmd_pos ++);
				insert_cmd(Asm::InstID::ADD, p_reg2, param_imm(TypeInt32, shift2));
			}

			int r1_first = cmd_pos;
			cmd.next_cmd_target(cmd_pos ++);
			insert_cmd(Asm::InstID::MOV, p_reg, p_deref_reg2);

			cmd.next_cmd_target(cmd_pos ++);
			insert_cmd(Asm::InstID::MOV, p_reg2, p1);

			if (shift1 > 0) {
				// resolve deref temp&loc 1
				cmd.next_cmd_target(cmd_pos ++);
				insert_cmd(Asm::InstID::ADD, p_reg2, param_imm(TypeInt32, shift1));
			}

			cmd.set_virtual_reg(reg, r1_first, cmd_pos);
			cmd.set_virtual_reg(reg2, r2_first, cmd_pos);

			i = cmd_pos;
		}
	}
}

void BackendX86::scan_temp_var_usage() {
	/*msg_write("ScanTempVarUsage");
	for (auto&& [i, v]: temp_var) {
		v.first = -1;
		v.last = -1;
		v.usage_count = 0;
		for (int c=0;c<cmd.num;c++) {
			if (temp_in_cmd(c, i) > 0) {
				v.usage_count ++;
				if (v.first < 0)
					v.first = c;
				v.last = c;
			}
		}
	}
	temp_var_ranges_defined = true;*/
}





Asm::InstructionParam BackendX86::prepare_param(Asm::InstID inst, SerialNodeParam &p) {
	if (p.kind == NodeKind::None) {
		return Asm::param_none;
	} else if (p.kind == NodeKind::Label) {
		return Asm::param_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::DereferenceLabel) {
		return Asm::param_deref_label(p.p, p.type->size);
	} else if (p.kind == NodeKind::Register) {
		if (p.shift > 0)
			do_error("prepare_param: reg + shift");
		return Asm::param_reg(p.as_reg());
		//param_size = p.type->size;
	} else if (p.kind == NodeKind::DereferenceRegister) {
		if (p.shift != 0)
			return Asm::param_deref_reg_shift(p.as_reg(), p.shift, p.type->size);
		else
			return Asm::param_deref_reg(p.as_reg(), p.type->size);
	} else if (p.kind == NodeKind::Memory) {
		int size = p.type->size;
		// compiler self-test
		if ((size != 1) and (size != 2) and (size != 4) and (size != 8))
			do_error("prepare_param: evil global of type " + p.type->name);
		return Asm::param_deref_imm(p.p + p.shift, size);
	} else if (p.kind == NodeKind::LocalMemory) {
		return Asm::param_deref_reg_shift(Asm::RegID::EBP, p.p + p.shift, p.type->size);
		//if ((param_size != 1) and (param_size != 2) and (param_size != 4) and (param_size != 8))
		//	param_size = -1; // lea doesn't need size...
			//s->DoErrorInternal("prepare_param: evil local of type " + p.type->name);
	} else if (p.kind == NodeKind::ConstantByAddress) {
		bool imm_allowed = Asm::get_instruction_allow_const(inst);
		if (imm_allowed and p.type->is_pointer_raw()) {
			return Asm::param_imm(*(int_p*)(p.p + p.shift), p.type->size);
		} else if (imm_allowed and (p.type->size <= 4)) {
			return Asm::param_imm(*(int*)(p.p + p.shift), p.type->size);
		} else {
			return Asm::param_deref_imm(p.p + p.shift, p.type->size);
		}
	} else if (p.kind == NodeKind::Immediate) {
		if (p.shift > 0)
			do_error("prepare_param: immediate + shift");
		return Asm::param_imm(p.p, p.type->size);
	} else {
		do_error("prepare_param: unexpected param..." + p.str(serializer));
	}
	return Asm::param_none;
}


void BackendX86::assemble_cmd(SerialNode &c) {
	// translate parameters
	auto p1 = prepare_param(c.inst, c.p[0]);
	auto p2 = prepare_param(c.inst, c.p[1]);

	// assemble instruction
	//list->current_line = c.
	list->add2(c.inst, p1, p2);
}


void BackendX86::add_function_intro_frame(int stack_alloc_size) {
	auto reg_bp = Asm::RegID::EBP;
	auto reg_sp = Asm::RegID::ESP;
	//int s = config.pointer_size;
	list->add2(Asm::InstID::PUSH, Asm::param_reg(reg_bp));
	list->add2(Asm::InstID::MOV, Asm::param_reg(reg_bp), Asm::param_reg(reg_sp));
	if (stack_alloc_size > 127){
		list->add2(Asm::InstID::SUB, Asm::param_reg(reg_sp), Asm::param_imm(stack_alloc_size, Asm::SIZE_32));
	}else if (stack_alloc_size > 0){
		list->add2(Asm::InstID::SUB, Asm::param_reg(reg_sp), Asm::param_imm(stack_alloc_size, Asm::SIZE_8));
	}
}
// convert    SerialNode[] cmd   into    Asm::Instruction..List list
void BackendX86::assemble() {
	// intro + allocate stack memory
	stack_max_size += max_push_size;
	stack_max_size = mem_align(stack_max_size, config.target.stack_frame_align);

	list->insert_location_label(cur_func->_label);
	if (!flags_has(cur_func->flags, Flags::Noframe))
		add_function_intro_frame(stack_max_size); // param intro later...

	for (auto &c: cmd.cmd) {

		if (c.inst == Asm::InstID::LABEL) {
			list->insert_location_label(c.p[0].p);
		} else if (c.inst == Asm::InstID::ASM) {
			add_asm_block(c.p[0].p);
		} else {
			assemble_cmd(c);
		}
	}
	list->add2(Asm::InstID::ALIGN_OPCODE);
}

}
