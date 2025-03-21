#include "../../base/base.h"
#include "../../os/msg.h"
#include "asm.h"
#include "internal.h"
#include <stdio.h>

namespace Asm {


int OCParam;

ParserState state;


InstructionSetData instruction_set;

void ParserState::init() {
	set_bits(instruction_set.pointer_size);

	if (CurrentMetaInfo)
		set_bits(CurrentMetaInfo->bits_size);

	list = nullptr;
}

void ParserState::set_bits(int bits_size) {
	default_addr_size = min(bits_size, (int)SIZE_32);
	default_param_size = min(bits_size, (int)SIZE_32);
	full_register_size = bits_size;
	param_size = default_param_size;
	addr_size = default_addr_size;
}

void ParserState::reset(InstructionWithParamsList *_list) {
	explicit_size = SIZE_UNKNOWN;
	param_size = default_param_size;
	addr_size = default_addr_size;
	extend_mod_rm_base = false;
	extend_mod_rm_reg = false;
	extend_mod_rm_index = false;
	list = _list;
}

string ParserState::get_label(int i) {
	if (list)
		if ((i >= 0) and (i < list->label.num))
			return list->label[i].name;
	return "_label_" + i2s(i);
}

const char *code_buffer;
MetaInfo *CurrentMetaInfo = nullptr;
MetaInfo DummyMetaInfo(SIZE_32);

int arm_encode_8l4(unsigned int value);

Exception::Exception(const string &_message, const string &_expression, int _line, int _column) {
	expression = _expression;
	text = _message;
	line = _line;
	column = _column;
}

Exception::~Exception() {}

string Exception::message() const {
	string m;
	if (expression != "")
		m += "\"" + expression + "\": ";
	m += text;
	if (line >= 0)
		m += "\nline " + i2s(line + 1);
	return m;
}

void Exception::print() const {
	msg_error(message());
}

void raise_error(const string &str) {
	msg_error(str);
	//msg_error(str + format("\nline %d", LineNo + 1));
	throw Exception(str, "", state.line_no, state.column_no);
}


bool DebugAsm = false;

static void so(const char *str) {
	if (DebugAsm)
		printf("%s\n",str);
}

static void so(const string &str) {
	if (DebugAsm)
		printf("%s\n",str.c_str());
}

[[maybe_unused]] static void so(int i) {
	if (DebugAsm)
		printf("%d\n",i);
}



MetaInfo::MetaInfo(int _bits_size) {
	bits_size = _bits_size;
	code_origin = 0;
	line_offset = 0;
}




Array<Register> registers;
Array<Register*> register_by_id;
RegRoot reg_root[(int)RegID::COUNT];
RegID reg_from_root[(int)RegRoot::COUNT][MAX_REG_SIZE + 1];

void add_reg(const string &name, RegID id, RegGroup group, int size, RegRoot root) {
	Register r;
	r.extend_mod_rm = false;
	r.name = name;
	r.id = id;
	r.group = group;
	if (group == RegGroup::GENERAL2) {
		r.group = RegGroup::GENERAL;
		r.extend_mod_rm = true;
	}
	r.size = size;
	registers.add(r);
	reg_root[(int)id] = root;
	reg_from_root[(int)root][size] = id;
}

string get_reg_name(RegID reg) {
	if (((int)reg < 0) or ((int)reg >= (int)RegID::COUNT))
		return "INVALID REG: " + i2s((int)reg);
	return RegisterByID(reg)->name;
}

// rw1/2: 
const InstructionName instruction_names[(int)InstID::NUM_INSTRUCTION_NAMES + 1] = {
	{InstID::DB,		"db"},
	{InstID::DW,		"dw"},
	{InstID::DD,		"dd"},
	{InstID::DQ,		"dq"},
	{InstID::DS,		"ds"},
	{InstID::DZ,		"dz"},
	{InstID::ALIGN_OPCODE,	":align:"},

	{InstID::ADD,		"add",		3, 1},
	{InstID::ADC,		"adc",		3, 1},
	{InstID::SUB,		"sub",		3, 1},
	{InstID::SBB,		"sbb",		3, 1},
	{InstID::INC,		"inc",		3},
	{InstID::DEC,		"dec",		3},
	{InstID::MUL,		"mul",		3, 1},
	{InstID::IMUL,		"imul",		3, 1},
	{InstID::DIV,		"div",		64+3, 64+1},
	{InstID::IDIV,		"idiv",		64+3, 64+1},
	{InstID::MOV,		"mov",		2, 1},
	{InstID::MOVZX,	"movzx",	2, 1},
	{InstID::MOVSX,	"movsx",	2, 1},
	{InstID::MOVSXD,	"movsxd",	2, 1},
	{InstID::AND,		"and",		3, 1},
	{InstID::OR,		"or",		3, 1},
	{InstID::XOR,		"xor",		3, 1},
	{InstID::NOT,		"not",		3},
	{InstID::NEG,		"neg",		3},
	{InstID::POP,		"pop",		2},
	{InstID::POPA,		"popa",		2},
	{InstID::PUSH,		"push",		1},
	{InstID::PUSHA,	"pusha",	1},
	
	{InstID::JO,		"jo",		1},
	{InstID::JNO,		"jno",		1},
	{InstID::JB,		"jb",		1},
	{InstID::JNB,		"jnb",		1},
	{InstID::JZ,		"jz",		1},
	{InstID::JNZ,		"jnz",		1},
	{InstID::JBE,		"jbe",		1},
	{InstID::JNBE,		"jnbe",		1},
	{InstID::JS,		"js",		1},
	{InstID::JNS,		"jns",		1},
	{InstID::JP,		"jp",		1},
	{InstID::JNP,		"jnp",		1},
	{InstID::JL,		"jl",		1},
	{InstID::JNL,		"jnl",		1},
	{InstID::JLE,		"jle",		1},
	{InstID::JNLE,		"jnle",		1},
	
	{InstID::CMP,		"cmp",		1, 1},
	
	{InstID::SETO,		"seto",		2},
	{InstID::SETNO,	"setno",	2},
	{InstID::SETB,		"setb",		2},
	{InstID::SETNB,	"setnb",	2},
	{InstID::SETZ,		"setz",		2},
	{InstID::SETNZ,	"setnz",	2},
	{InstID::SETBE,	"setbe",	2},
	{InstID::SETNBE,	"setnbe",	2},
	{InstID::SETS,		"sets",		2},
	{InstID::SETNS,	"setns",	2},
	{InstID::SETP,		"setp",		2},
	{InstID::SETNP,	"setnp",	2},
	{InstID::SETL,		"setl",		2},
	{InstID::SETNL,	"setnl",	2},
	{InstID::SETLE,	"setle",	2},
	{InstID::SETNLE,	"setnle",	2},
	
	{InstID::SLDT,		"sldt"},
	{InstID::STR,		"str"},
	{InstID::LLDT,		"lldt"},
	{InstID::LTR,		"ltr"},
	{InstID::VERR,		"verr"},
	{InstID::VERW,		"verw"},
	{InstID::SGDT,		"sgdt"},
	{InstID::SIDT,		"sidt"},
	{InstID::LGDT,		"lgdt"},
	{InstID::LIDT,		"lidt"},
	{InstID::SMSW,		"smsw"},
	{InstID::LMSW,		"lmsw"},
	
	{InstID::TEST,		"test",		1, 1},
	{InstID::XCHG,		"xchg",		3, 3},
	{InstID::LEA,		"lea", 		32+2, 32+1},
	{InstID::NOP,		"nop"},
	{InstID::CBW_CWDE,	"cbw/cwde"},
	{InstID::CGQ_CWD,	"cgq/cwd"},
	{InstID::MOVS_DS_ESI_ES_EDI,	"movs_ds:esi,es:edi"},
	{InstID::MOVS_B_DS_ESI_ES_EDI,	"movs.b_ds:esi,es:edi"},
	{InstID::CMPS_DS_ESI_ES_EDI,	"cmps_ds:esi,es:edi"},
	{InstID::CMPS_B_DS_ESI_ES_EDI,	"cmps.b_ds:esi,es:edi"},
	{InstID::ROL,		"rol",		3, 1},
	{InstID::ROR,		"ror",		3, 1},
	{InstID::RCL,		"rcl",		3, 1},
	{InstID::RCR,		"rcr",		3, 1},
	{InstID::SHL,		"shl",		3, 1},
	{InstID::SHR,		"shr",		3, 1},
	{InstID::SAR,		"sar",		3, 1},
	{InstID::RET,		"ret",		1},
	{InstID::LEAVE,	"leave",	1},
	{InstID::RET_FAR,	"ret_far",	1},
	{InstID::INT,		"int",		1},
	{InstID::IRET,		"iret",		1},
	
	// x87
	{InstID::FADD,		"fadd",		64+32+1},
	{InstID::FMUL,		"fmul",		64+32+1},
	{InstID::FSUB,		"fsub",		64+32+1},
	{InstID::FDIV,		"fdiv",		64+32+1},
	{InstID::FLD,		"fld",		64+32+1},
	{InstID::FLD1,		"fld1",		64+32+0},
	{InstID::FLDZ,		"fldz",		64+32+0},
	{InstID::FLDPI,	"fldpi",	64+32+0},
	{InstID::FXCH,		"fxch",		64+32+3, 64+32+3},
	{InstID::FST,		"fst",		64+32+2},
	{InstID::FSTP,		"fstp",		64+32+2},
	{InstID::FILD,		"fild",		64+32+1},
	{InstID::FADDP,	"faddp",	64+32+1},
	{InstID::FMULP,	"fmulp",	64+32+1},
	{InstID::FSUBP,	"fsubp",	64+32+1},
	{InstID::FDIVP,	"fdivp",	64+32+1},
	{InstID::FLDCW,	"fldcw",	64+32+1},
	{InstID::FNSTCW,	"fnstcw",	64+32+2},
	{InstID::FNSTSW,	"fnstsw",	64+32+2},
	{InstID::FISTP,	"fistp",	64+32+2},
	{InstID::FSQRT,	"fsqrt",	64+32+3},
	{InstID::FSIN,		"fsin",		64+32+3},
	{InstID::FCOS,		"fcos",		64+32+3},
	{InstID::FPTAN,	"fptan",	64+32+3},
	{InstID::FPATAN,	"fpatan",	64+32+3},
	{InstID::FYL2X,	"fyl2x",	64+32+3},
	{InstID::FCHS,		"fchs",		64+32+3},
	{InstID::FABS,		"fabs",		64+32+3},
	{InstID::FUCOMPP,	"fucompp",	64+32+1, 64+32+1},
	
	{InstID::LOOP,		"loop"},
	{InstID::LOOPE,	"loope"},
	{InstID::LOOPNE,	"loopne"},
	{InstID::IN,		"in",		2, 1},
	{InstID::OUT,		"out",		1, 1},
	
	{InstID::CALL,		"call",		1},
	{InstID::CALL_FAR,	"call_far", 1},
	{InstID::JMP,		"jmp",		1},
	{InstID::JMP_FAR,	"jmp_far",		1},
	{InstID::LOCK,		"lock"},
	{InstID::REP,		"rep"},
	{InstID::REPNE,	"repne"},
	{InstID::HLT,		"hlt"},
	{InstID::CMC,		"cmc"},
	{InstID::CLC,		"clc"},
	{InstID::STC,		"stc"},
	{InstID::CLI,		"cli"},
	{InstID::STI,		"sti"},
	{InstID::CLD,		"cld"},
	{InstID::STD,		"std"},

	// sse
	{InstID::MOVSS,  "movss",  64+3, 64+1},
	{InstID::MOVSD,  "movsd",  64+3, 64+1},
	{InstID::MOVUPS, "movups", 64+3, 64+1},
	{InstID::MOVAPS, "movaps", 64+3, 64+1},
	{InstID::MOVLPS, "movlps", 64+3, 64+1},
	{InstID::MOVHPS, "movhps", 64+3, 64+1},
	{InstID::ADDSS,  "addss",  64+3, 64+1},
	{InstID::ADDSD,  "addsd",  64+3, 64+1},
	{InstID::ADDPS,  "addps",  64+3, 64+1},
	{InstID::SUBSS,  "subss",  64+3, 64+1},
	{InstID::SUBSD,  "subsd",  64+3, 64+1},
	{InstID::MULSS,  "mulss",  64+3, 64+1},
	{InstID::MULSD,  "mulsd",  64+3, 64+1},
	{InstID::DIVSS,  "divss",  64+3, 64+1},
	{InstID::DIVSD,  "divsd",  64+3, 64+1},
	{InstID::SQRTSS, "sqrtss", 64+3, 64+1},
	{InstID::SQRTSD, "sqrtsd", 64+3, 64+1},
	{InstID::MINSS,  "minss",  64+3, 64+1},
	{InstID::MINSD,  "minsd",  64+3, 64+1},
	{InstID::MAXSS,  "maxss",  64+3, 64+1},
	{InstID::MAXSD,  "maxsd",  64+3, 64+1},
	{InstID::CVTSS2SD,  "cvtss2sd",  64+3, 64+1},
	{InstID::CVTSD2SS,  "cvtsd2ss",  64+3, 64+1},
	{InstID::CVTTSS2SI, "cvttss2si", 64+3, 64+1},
	{InstID::CVTTSD2SI, "cvttsd2si", 64+3, 64+1},
	{InstID::CVTSI2SS,  "cvtsi2ss",  64+3, 64+1},
	{InstID::CVTSI2SD,  "cvtsi2sd",  64+3, 64+1},
	{InstID::COMISS,    "comiss",    64+3, 64+1},
	{InstID::COMISD,    "comisd",    64+3, 64+1},
	{InstID::UCOMISS,   "ucomiss",   64+3, 64+1},
	{InstID::UCOMISD,   "ucomisd",   64+3, 64+1},

	{InstID::WRMSR,    "wrmsr"},
	{InstID::RDTSC,    "rdtsc"},
	{InstID::RDMSR,    "rdmsr"},
	{InstID::RDPMC,    "rdpmc"},
	{InstID::CPUID,    "cpuid"},
	{InstID::LFENCE,   "lfence"},
	{InstID::MFENCE,   "mfence"},
	{InstID::SFENCE,   "sfence"},
	{InstID::CLFLUSH,  "clflush"},

	// amd64
	{InstID::SYSCALL,  "syscall"},
	{InstID::SYSRET,   "sysret"},
	{InstID::SYSENTER, "sysenter"},
	{InstID::SYSEXIT,  "sysexit"},

	{InstID::B,		"b"},
	{InstID::BL,		"bl"},
	{InstID::BLR,		"blr"},
	{InstID::BLX,		"blx"},
	{InstID::TBZ,		"tbz"},
	{InstID::TBNZ,		"tbnz"},
	{InstID::ADR,		"adr"},
	{InstID::ADRP,		"adrp"},
	{InstID::CSET,		"cset"},

	{InstID::MULS, "muls"},
	{InstID::ADDS, "adds"},
	{InstID::SUBS, "subs"},
	{InstID::RSBS, "rsbs"},
	{InstID::ADCS, "adcs"},
	{InstID::SBCS, "sbcs"},
	{InstID::RSCS, "rscs"},
	{InstID::ANDS, "ands"},
	{InstID::BICS, "bics"},
	{InstID::XORS, "xors"},
	{InstID::ORS, "ors"},
	{InstID::MOVS, "movs"},
	{InstID::MVNS, "movns"},
	{InstID::SXTW, "sxtw"},
	{InstID::SXTB, "sxtb"},

	{InstID::LDR,		"ldr"},
	{InstID::LDUR,		"ldur"},
	{InstID::LDRB,		"ldrb"},
	{InstID::LDRSW,		"ldrsw"},
	{InstID::LDP,		"ldp"},
	{InstID::LDP_PREINDEX,		"ldp!"},
	{InstID::LDP_POSTINDEX,		"ldp_post"},
//	{inst_str,		"str"},
	{InstID::STUR,		"stur"},
	{InstID::STRB,		"strb"},
	{InstID::STP,		"stp"},
	{InstID::STP_PREINDEX,		"stp!"},
	{InstID::STP_POSTINDEX,		"stp_post"},

	{InstID::LDMIA,		"ldmia"},
	{InstID::LDMIB,		"ldmib"},
	{InstID::LDMDA,		"ldmda"},
	{InstID::LDMDB,		"ldmdb"},
	{InstID::STMIA,		"stmia"},
	{InstID::STMIB,		"stmib"},
	{InstID::STMDA,		"stmda"},
	{InstID::STMDB,		"stmdb"},

	{InstID::RSB,	"rsb"},
	{InstID::SBC,	"sbc"},
	{InstID::RSC,	"rsc"},
	{InstID::TST,	"tst"},
	{InstID::TEQ,	"teq"},
	{InstID::CMN,	"cmn"},
	{InstID::BIC,	"bic"},
	{InstID::MVN,	"mvn"},


	// ARM float
	{InstID::FMACS,	"fmacs"},
	{InstID::FNMACS,	"fnmacs"},
	{InstID::FMSCS,	"fmscs"},
	{InstID::FNMSCS,	"fnmscs"},
	{InstID::FMULS,	"fmuls"},
	{InstID::FNMULS,	"fnmuls"},
	{InstID::FADDS,	"fadds"},
	{InstID::FSUBS,	"fsubs"},
	{InstID::FDIVS,	"fdivs"},
	{InstID::FCPYS,	"fcpys"},
	{InstID::FABSS,	"fabss"},
	{InstID::FNEGS,	"fnegs"},
	{InstID::FSQRTS,	"fsqrts"},
	{InstID::FCMPS,	"fcmps"},
	{InstID::FCMPES,	"fcmpes"},
	{InstID::FCMPZS,	"fcmpzs"},
	{InstID::FCMPEZS,	"fcmpezs"},
	{InstID::CVTDS,	"cvtds"},
	{InstID::FTOUIS,	"ftouis"},
	{InstID::FTOUIZS,	"ftouizs"},
	{InstID::FTOSIS,	"ftosis"},
	{InstID::FTOSIZS,	"ftosizs"},
	{InstID::FUITOS,	"fuitos"},
	{InstID::FSITOS,	"fsitos"},
	{InstID::FMRS,	"fmrs"},
	{InstID::FMSR,	"fmsr"},
	{InstID::FLDS,	"flds"},
	{InstID::FSTS,	"fsts"},
	{InstID::FMOV,	"fmov"},
	{InstID::FCMP,	"fcmp"},
	{InstID::SCVTF,	"scvtf"},
	{InstID::FCVTZS,	"fcvtzs"},
	{InstID::FCVT,	"fcvt"},

	{InstID::MODULO, "modulo"},
	{InstID::LABEL, "-label-"},
	{InstID::ASM, "-asm-"},
	{InstID::CALL_MEMBER, "call-member"},

	{InstID::INVALID,			"???"}
};





InstructionParam param_none;


InstructionWithParamsList::InstructionWithParamsList(int line_no) {
	current_inst = 0;
	current_line = line_no;
	current_col = 0;
}

InstructionWithParamsList::~InstructionWithParamsList() {}

Register *get_reg(Asm::RegID reg) {
	if (((int)reg < 0) or ((int)reg >= (int)register_by_id.num))
		raise_error("invalid register index: " + i2s((int)reg));
	return RegisterByID(reg);
}

InstructionParam param_reg(Asm::RegID reg) {
	InstructionParam p;
	p.type = ParamType::REGISTER;
	p.reg = get_reg(reg);
	p.size = p.reg->size;
	return p;
}

InstructionParam param_deref_reg(Asm::RegID reg, int size) {
	InstructionParam p;
	p.type = ParamType::REGISTER;
	p.reg = get_reg(reg);
	p.size = size;
	p.deref = true;
	return p;
}

InstructionParam param_reg_set(int set) {
	InstructionParam p;
	p.type = ParamType::REGISTER_SET;
	p.size = SIZE_32;
	p.value = set;
	return p;
}

InstructionParam param_deref_reg_shift(Asm::RegID reg, int shift, int size) {
	InstructionParam p;
	p.type = ParamType::REGISTER;
	p.reg = get_reg(reg);
	p.size = size;
	p.deref = true;
	p.value = shift;
	p.disp = ((shift < 120) and (shift > -120)) ? DispMode::_8 : DispMode::_32;
	return p;
}

InstructionParam param_deref_reg_shift_reg(Asm::RegID reg, Asm::RegID reg2, int size) {
	InstructionParam p;
	p.type = ParamType::REGISTER;
	p.reg = get_reg(reg);
	p.size = size;
	p.reg2 = get_reg(reg2);
	p.deref = true;
	p.value = 1;
	p.disp = DispMode::REG2;
	return p;
}

InstructionParam param_imm(int64 value, int size) {
	InstructionParam p;
	p.type = ParamType::IMMEDIATE;
	p.size = size;
	p.value = value;
	return p;
}

InstructionParam param_deref_imm(int64 value, int size) {
	InstructionParam p;
	p.type = ParamType::IMMEDIATE;
	p.size = size;
	p.value = value;
	p.deref = true;
	return p;
}

InstructionParam param_label(int64 value, int size) {
	InstructionParam p;
	p.type = ParamType::IMMEDIATE;
	p.size = size;
	p.value = value;
	p.is_label = true;
	return p;
}

InstructionParam param_deref_label(int64 value, int size) {
	InstructionParam p;
	p.type = ParamType::IMMEDIATE;
	p.size = size;
	p.value = value;
	p.is_label = true;
	p.deref = true;
	return p;
}

void InstructionWithParamsList::add_arm(ArmCond cond, InstID inst, const InstructionParam &p1 = param_none, const InstructionParam &p2, const InstructionParam &p3) {
	InstructionWithParams i;
	i.inst = inst;
	i.condition = cond;
	i.p[0] = p1;
	i.p[1] = p2;
	i.p[2] = p3;
	i.line = current_line;
	i.col = current_col;
	add(i);
}

void InstructionWithParamsList::add2(InstID inst, const InstructionParam &p1, const InstructionParam &p2) {
	InstructionWithParams i;
	i.inst = inst;
	i.condition = ArmCond::Always;
	i.p[0] = p1;
	i.p[1] = p2;
	i.p[2] = param_none;
	i.line = current_line;
	i.col = current_col;
	add(i);
}

void InstructionWithParamsList::show() {
	msg_write("--------------");
	state.reset(this);
	foreachi(Asm::InstructionWithParams &i, *this, n) {
		for (Label &l: label)
			if (l.inst_no == n)
				msg_write("    " + l.name + ":");
		msg_write(i.str());
	}
}


int InstructionWithParamsList::create_label(const string &name) {
	/*if (name == "$")
		return -1;*/
	Label l;
	l.name = name;
	l.inst_no = -1;
	l.value = -1;
	label.add(l);
	return label.num - 1;
}

int InstructionWithParamsList::_find_label(const string &name) {
	foreachi (Label &l, label, i)
		if (l.name == name)
			return i;
	return -1;

}

// really declare an existing one now
void InstructionWithParamsList::insert_location_label(int index) {
	if (index < 0)
		return;
	Label &l = label[index];
	if (l.inst_no >= 0 and l.name != "$")
		raise_error("label already declared: " + l.name);
	l.inst_no = num;
	so("----redecl");

}
int64 InstructionWithParamsList::_label_value(int index) {
	if (index < 0)
		return 0;
	Label &l = label[index];
	return l.value;

}

// declare
int InstructionWithParamsList::find_or_create_label(const string &name) {
	so("add_label: " + name);
	// label already in use? (used before declared)
	int l = _find_label(name);
	if (l < 0)
		l = create_label(name);
	return l;
}

void *InstructionWithParamsList::get_label_value(const string &name) {
	return (void*)_label_value(_find_label(name));
}


void InstructionWithParamsList::add_wanted_label(int pos, int label_no, int inst_no, bool rel, bool abs, int size) {
	if ((label_no < 0) or (label_no >= label.num))
		raise_error("illegal wanted label request");
	WantedLabel w;
	w.pos = pos;
	w.size = size;
	w.label_no = label_no;
	w.name = label[label_no].name;
	w.relative = rel;
	w.abs = abs;
	w.inst_no = inst_no;
	wanted_label.add(w);
	so("add wanted label");
}

string size_out(int size) {
	if (size == SIZE_8)
		return "8";
	if (size == SIZE_16)
		return "16";
	if (size == SIZE_32)
		return "32";
	if (size == SIZE_48)
		return "48";
	if (size == SIZE_64)
		return "64";
	if (size == SIZE_128)
		return "128";
	return "???";
}


string get_size_name(int size) {
	if (size == SIZE_8)
		return "byte";
	if (size == SIZE_16)
		return "word";
	if (size == SIZE_32)
		return "dword";
	if (size == SIZE_48)
		return "s48";
	if (size == SIZE_64)
		return "qword";
	if (size == SIZE_128)
		return "dqword";
	return "";
}

void InstructionParamFuzzy::print() const {
	string t;
	if (used) {
		if (allow_register)
			t += "	Reg";
		if (allow_immediate)
			t += "	Im";
		if (allow_memory_address)
			t += "	[Mem]";
		if (allow_memory_indirect)
			t += "	[Mem + ind]";
		if (reg)
			t += "  " + reg->name;
		if (size != SIZE_UNKNOWN)
			t += "  " + size_out(size);
		if (mrm_mode == ModRM::REG)
			t += "   /r";
		else if (mrm_mode == ModRM::MOD_RM)
			t += "   /m";
	} else {
		t += "	None";
	}
	msg_write(t);
}

const string get_instruction_name(InstID inst) {
	if (((int)inst >= 0) and ((int)inst < (int)InstID::NUM_INSTRUCTION_NAMES))
		return Asm::instruction_names[(int)inst].name;
	return "???";
}

void get_instruction_param_flags(InstID inst, bool &p1_read, bool &p1_write, bool &p2_read, bool &p2_write) {
	for (int i=0;i<(int)InstID::NUM_INSTRUCTION_NAMES;i++)
		if (instruction_names[i].inst == inst) {
			p1_read = ((instruction_names[i].rw1 & 1) > 0);
			p1_write = ((instruction_names[i].rw1 & 2) > 0);
			p2_read = ((instruction_names[i].rw2 & 1) > 0);
			p2_write = ((instruction_names[i].rw2 & 2) > 0);
		}
}

bool get_instruction_allow_const(InstID inst) {
	for (int i=0;i<(int)InstID::NUM_INSTRUCTION_NAMES;i++)
		if (instruction_names[i].inst == inst)
			return ((instruction_names[i].rw1 & 64) == 0);
	return false;
}

bool get_instruction_allow_gen_reg(InstID inst) {
	for (int i=0;i<(int)InstID::NUM_INSTRUCTION_NAMES;i++)
		if (instruction_names[i].inst == inst)
			return ((instruction_names[i].rw1 & 32) == 0);
	return false;
}



InstructionSet guess_native_instruction_set() {
#ifdef CPU_AMD64
	return InstructionSet::AMD64;
#endif
#ifdef CPU_X86
	return InstructionSet::X86;
#endif
#ifdef CPU_ARM64
	return InstructionSet::ARM64;
#endif
#ifdef CPU_ARM32
	return InstructionSet::ARM;
#endif
	return InstructionSet::UNKNOWN;
}



void init(InstructionSet set) {
	if (set == InstructionSet::NATIVE)
		set = guess_native_instruction_set();

	instruction_set.set = set;
	instruction_set.pointer_size = 4;
	if ((set == InstructionSet::AMD64) || (set == InstructionSet::ARM64))
		instruction_set.pointer_size = 8;

	for (int i=0; i<(int)RegRoot::COUNT; i++)
		for (int j=0; j<=MAX_REG_SIZE; j++)
			reg_from_root[i][j] = RegID::INVALID;


	// self check
	for (int i=0; i<(int)InstID::NUM_INSTRUCTION_NAMES; i++)
		if (instruction_names[i].inst != (InstID)i)
			msg_error(format("%s  %d  !=  %d", instruction_names[i].name, (int)instruction_names[i].inst, i));

	if (set == InstructionSet::ARM32)
		arm32_init();
	else if (set == InstructionSet::ARM64)
		arm64_init();
	else if ((set == InstructionSet::X86) || (set == InstructionSet::AMD64))
		x86_init();

	if (set == InstructionSet::UNKNOWN)
		msg_error("kaba/asm: unsupported CPU instruction set");
}

InstructionParam::InstructionParam() {
	type = ParamType::NONE;
	disp = DispMode::NONE;
	reg = nullptr;
	reg2 = nullptr;
	deref = false;
	size = SIZE_UNKNOWN;
	value = 0;
	is_label = false;
	write_back = false;
}

bool InstructionParam::has_explicit_size() const {
	if (type == ParamType::REGISTER) {
		return !deref;
	} else if (type == ParamType::IMMEDIATE) {
		return !is_label and !deref;
	}
	return false;
}

// convert an asm parameter into a human readable expression
string InstructionParam::str(bool hide_size) {
	//msg_write("----");
	//msg_write(p.type);
	if (type == ParamType::INVALID) {
		return "-\?\?\?-";
	} else if (type == ParamType::NONE) {
		return "";
	} else if (type == ParamType::REGISTER) {
		string post;
		if (write_back)
			post = "!";
			//msg_write((int_p)reg);
			//msg_write((int_p)disp);
		if (deref) {
			//msg_write("deref");
			string ss;
			if (!hide_size or size != state.default_param_size)
				ss = get_size_name(size) + " ";
			string s = reg->name;
			if (disp == DispMode::_8) {
				if (value > 0)
					s += format("+0x%02x", (value & 0xff));
				else
					s += format("-0x%02x", ((-value) & 0xff));
			} else if (disp == DispMode::_16) {
				s += format("+0x%04x", (value & 0xffff));
			} else if (disp == DispMode::_32) {
				s += format("+0x%08x", value);
			} else if (disp == DispMode::SIB) {
				return "SIB[...][...]";
			} else if (disp == DispMode::_8_SIB) {
				s += format("::SIB...+0x%02x", value);
			} else if (disp == DispMode::_8_REG2) {
				s += format("%s+0x%02x", reg2->name.c_str(), value);
			} else if (disp == DispMode::REG2) {
				s += "+" + reg2->name;
			}
			return ss + "[" + s + "]" + post;
		} else {
			return reg->name + post;
		}
	} else if (type == ParamType::REGISTER_SET) {
		Array<string> s;
		for (int i=0; i<16; i++)
			if (value & (1<<i))
				s.add(register_by_id[(int)RegID::R0 + i]->name);
		return "{" + implode(s, ",") + "}";
	} else if (type == ParamType::IMMEDIATE) {
		string s = "0x" + i2h(value, deref ? state.addr_size : size);
		if (is_label)
			s = get_size_name(size) + " " + state.get_label(value);
		if (deref)
			return get_size_name(size) + " [" + s + "]";
		return s;
	/*} else if (type == ParamTImmediateExt) {
		//msg_write("im");
		return format("%s:%s", d2h(&((char*)&value)[4], 2).c_str(), d2h(&value, state.ParamSize).c_str());*/
	}
	return "\?\?\?";
}

string ARMConditions[16] = {
	"eq",
	"ne",
	"cs",
	"cc",
	"mi",
	"pl",
	"vs",
	"vc",
	"hi",
	"ls",
	"ge",
	"lt",
	"gt",
	"le",
	"al",
	"???",
};

string InstructionWithParams::str() {
	string s;
	if (condition != ArmCond::Always)
		s += ARMConditions[(int)condition & 0xf] + ":";
	s += get_instruction_name(inst);
	s += "  " + p[0].str(false);
	if (p[1].type != ParamType::NONE)
		s += ",  " + p[1].str(false);
	if (p[2].type != ParamType::NONE)
		s += ",  " + p[2].str(false);
	return s;
}

string show_reg(int r) {
	return format("r%d", r);
}



// convert some opcode into (human readable) assembler language
string disassemble(void *code, int length, bool allow_comments) {
	if ((instruction_set.set == InstructionSet::ARM32) or (instruction_set.set == InstructionSet::ARM64))
		return arm_disassemble(code, length, allow_comments);
	return x86_disassemble(code, length, allow_comments);
}

// skip unimportant code (whitespace/comments)
//    returns true if end of code
bool IgnoreUnimportant(int &pos) {
	bool CommentLine = false;
	
	// ignore comments and "white space"
	for (int i=0;i<1048576;i++) {
		if (code_buffer[pos] == 0) {
			state.end_of_code = true;
			state.end_of_line = true;
			return true;
		}
		if (code_buffer[pos] == '\n') {
			state.line_no ++;
			state.column_no = 0;
			CommentLine = false;
		}
		// "white space"
		if ((code_buffer[pos] == '\n') or (code_buffer[pos] == ' ') or (code_buffer[pos] == '\t')) {
			pos ++;
			state.column_no ++;
			continue;
		}
		// comments
		if ((code_buffer[pos] == ';') or ((code_buffer[pos] == '/') and (code_buffer[pos] == '/'))) {
			CommentLine = true;
			pos ++;
			state.column_no ++;
			continue;
		}
		if (!CommentLine)
			break;
		pos ++;
		state.column_no ++;
	}
	return false;
}

// returns one "word" in the source code
string find_mnemonic(int &pos) {
	state.end_of_line = false;
	char mne[128];
	strcpy(mne, "");

	if (IgnoreUnimportant(pos))
		return mne;
	
	bool in_string = false;
	for (int i=0;i<128;i++) {
		mne[i] = code_buffer[pos];
		mne[i + 1] = 0;
		
		// string like stuff
		if ((mne[i] == '\'') or (mne[i] == '\"'))
			in_string =! in_string;
		// end of code
		if (code_buffer[pos] == 0) {
			mne[i] = 0;
			state.end_of_code = true;
			state.end_of_line = true;
			break;
		}
		// end of line
		if (code_buffer[pos] == '\n') {
			mne[i] = 0;
			state.end_of_line = true;
			break;
		}
		if (!in_string) {
			// "white space" -> complete
			if ((code_buffer[pos] == ' ') or (code_buffer[pos] == '\t') or (code_buffer[pos] == ',')) {
				mne[i] = 0;
				// end of line?
				for (int j=0;j<128;j++) {
					if ((code_buffer[pos+j] != ' ') and (code_buffer[pos+j] != '\t') and (code_buffer[pos+j] != ',')) {
						if ((code_buffer[pos + j] == 0) or (code_buffer[pos + j] == '\n'))
							state.end_of_line = true;
						// comment ending the line
						if ((code_buffer[pos + j] == ';') or ((code_buffer[pos + j] == '/') and (code_buffer[pos + j + 1] == '/')))
							state.end_of_line = true;
						pos += j;
						state.column_no += j;
						if (code_buffer[pos] == '\n')
							state.column_no = 0;
						break;
					}
				}
				break;
			}
		}
		pos ++;
		state.column_no ++;
	}
	/*msg_write>Write(mne);
	if (EndOfLine)
		msg_write>Write("    eol");*/
	return mne;
}

// interpret an expression from source code as an assembler parameter
void parse_parameter(InstructionParam &p, const string &param, InstructionWithParamsList &list, int pn) {
	p.type = ParamType::INVALID;
	p.reg = nullptr;
	p.deref = false;
	p.size = SIZE_UNKNOWN;
	p.disp = DispMode::NONE;
	p.is_label = false;
	//msg_write(param);

	// none
	if (param.num == 0) {
		p.type = ParamType::NONE;

	// deref
	} else if ((param[0] == '[') and (param[param.num-1] == ']')) {
		if (DebugAsm)
			printf("deref:   ");
		so("Deref:");
		//bool u16 = use_mode16;
		parse_parameter(p, param.sub(1, -1), list, pn);
		p.size = state.explicit_size; // SIZE_UNKNOWN, if not explicitly overwritten
		p.deref = true;
		//use_mode16 = u16;

	// string
	} else if ((param[0] == '\"') and (param[param.num-1] == '\"')) {
		if (DebugAsm)
			printf("String:   ");
		char *ps = new char[param.num - 1];
		strcpy(ps, param.sub(1, -1).c_str());
		p.value = (int_p)ps;
		p.type = ParamType::IMMEDIATE;

	// complex...
	} else if (param.find("+") >= 0) {
		if (DebugAsm)
			printf("complex:   ");
		InstructionParam sub;
		
		// first part (must be a register)
		string part;
		for (int i=0;i<param.num;i++)
			if ((param[i] == ' ') or (param[i] == '+'))
				break;
			else
				part.add(param[i]);
		int offset = part.num;
		parse_parameter(sub, part, list, pn);
		if (sub.type == ParamType::REGISTER) {
			//msg_write("reg");
			p.type = ParamType::REGISTER;
			p.size = SIZE_32;
			p.reg = sub.reg;
		} else
			p.type = ParamType::INVALID;

		// second part (...up till now only hex)
		for (int i=offset;i<param.num;i++)
			if ((param[i] != ' ') and (param[i] != '+')) {
				offset = i;
				break;
			}
		part = param.sub(offset);
		parse_parameter(sub, part, list, pn);
		if (sub.type == ParamType::IMMEDIATE) {
			//msg_write("c2 = im");
			if (((int_p)sub.value & 0xffffff00) == 0)
				p.disp = DispMode::_8;
			else
				p.disp = DispMode::_32;
			p.value = sub.value;
		} else
			p.type = ParamType::INVALID;

		

	// hex const
	} else if ((param[0] == '0') and (param[1] == 'x')) {
		p.type = ParamType::IMMEDIATE;
		int64 v = 0;
		for (int i=2;i<param.num;i++) {
			if (param[i] == '.') {
			} else if ((param[i] >= 'a') and (param[i] <= 'f')) {
				v *= 16;
				v += param[i] - 'a' + 10;
			} else if ((param[i] >= 'A') and (param[i] <= 'F')) {
				v *= 16;
				v += param[i]-'A'+10;
			} else if ((param[i]>='0') and (param[i]<='9')) {
				v*=16;
				v+=param[i]-'0';
			/*} else if (param[i]==':') {
				InstructionParam sub;
				GetParam(sub, param.tail(param.num - i - 1), list, pn);
				if (sub.type != ParamTImmediate) {
					raise_error("error in hex parameter:  " + string(param));
					p.type = PKInvalid;
					return;						
				}
				p.value = (int_p)v;
				p.value <<= 8 * sub.size;
				p.value += sub.value;
				p.size = sub.size;
				p.type = ParamTImmediate;//Ext;
				break;*/
			} else {
				raise_error("evil character in hex parameter:  \"" + param + "\"");
				p.type = ParamType::INVALID;
				return;
			}
			p.value = (int_p)v;
			p.size = SIZE_8;
			if (param.num > 4)
				p.size = SIZE_16;
			if (param.num > 6)
				p.size = SIZE_32;
			if (param.num > 10)
				p.size = SIZE_48;
			if (param.num > 14)
				p.size = SIZE_64;
		}
		if (DebugAsm) {
			printf("hex const:  0x%s\n",i2h(p.value,p.size).c_str());
		}

	// char const
	} else if ((param[0] == '\'') and (param[param.num - 1] == '\'')) {
		p = param_imm((int_p)param[1], SIZE_8);
		if (DebugAsm)
			printf("hex const:  0x%s\n",i2h(p.value,1).c_str());

	// label substitude
	} else if (param == "$") {
		int l = list.create_label(param);
		list.insert_location_label(l);
		p = param_label(l, state.default_addr_size);
		
	} else {
		// register
		for (auto &r: registers)
			if (r.name == param) {
				p = param_reg(r.id);
				return;
			}
		// existing label
		for (int i=0;i<list.label.num;i++)
			if (list.label[i].name == param) {
				p = param_label(i, state.default_addr_size);
				return;
			}
		// script variable (global)
		for (int i=0;i<CurrentMetaInfo->global_var.num;i++) {
			if (CurrentMetaInfo->global_var[i].name == param) {
				p = param_deref_imm((int_p)CurrentMetaInfo->global_var[i].pos, CurrentMetaInfo->global_var[i].size);
				return;
			}
		}
		// not yet existing label...
		if (param[0]=='_') {
			so("label as param:  \"" + param + "\"\n");
			p = param_label(list.create_label(param), state.default_addr_size);
			return;
		}
	}
	if (p.type == ParamType::INVALID)
		raise_error("unknown parameter:  \"" + param + "\"\n");
}


void insert_val(char *oc, int &ocs, int64 val, int size) {
	if (size == SIZE_8) {
		oc[ocs] = (char)val;
	} else if (size == SIZE_16) {
		*(short*)&oc[ocs] = (short)val;
	} else if (size == SIZE_24) {
		*(int*)&oc[ocs - 1] = (*(int*)&oc[ocs - 1] & 0xff000000) | ((int)val & 0x00ffffff);
	} else if (size == SIZE_32) {
		*(int*)&oc[ocs] = (int)val;
	} else if (size == SIZE_64) {
		*(int64*)&oc[ocs] = val;
	} else if (size == SIZE_8L4) {
		val = arm_encode_8l4(val);
		*(int*)&oc[ocs - 2] = (*(int*)&oc[ocs - 2] & 0xfffff000) | ((int)val & 0x00000fff);
	} else if (size == SIZE_12) {
		*(int*)&oc[ocs - 2] = (*(int*)&oc[ocs - 2] & 0xfffff000) | ((int)val & 0x00000fff);
	} else if (size == AP_IMM26X4REL_0 or size == AP_IMM19X4REL_5 or size == AP_IMM14X4REL_5) {
		if (!arm_encode_imm(*(unsigned int*)&oc[ocs], size, val, true))
			msg_error("failed to insert target location");
	} else if (size == SIZE_8S2) {
		oc[ocs] = (char)(val >> 2);
	} else if (size > 0) {
		memcpy(&oc[ocs], &val, size);
	}
}

void InstructionWithParamsList::link_wanted_labels(void *oc) {
	foreachib(WantedLabel &w, wanted_label, i) {
		Label &l = label[w.label_no];
		if (l.value == -1)
			continue;
		so("linking label");

		int64 value = l.value;
		if (w.relative) {
			int size = w.size;
			if ((size == SIZE_8L4) or (size == SIZE_12))
				size = 2;
			if (size == SIZE_8S2)
				size = 1;
			if (size == AP_IMM26X4REL_0 or size == AP_IMM19X4REL_5 or size == AP_IMM14X4REL_5)
				size = -4;

			// TODO first byte after command
			if (instruction_set.set == InstructionSet::ARM32 or instruction_set.set == InstructionSet::ARM64) {
				value -= CurrentMetaInfo->code_origin + w.pos + size + 4;
			/*	InstID inst = (*this)[w.inst_no].inst;
				if ((inst == InstID::BL) or (inst == InstID::B) or (inst == InstID::CALL) or (inst == InstID::JMP)) {
					value = value >> 2;
				}*/
			} else {
				value -= CurrentMetaInfo->code_origin + w.pos + size;
			}
		}
		if (w.abs and (value < 0))
			value = - value;

		insert_val((char*)oc, w.pos, value, w.size);


		wanted_label.erase(i);
		_foreach_it_.update();
	}
}

void add_data_inst(InstructionWithParamsList *l, int size) {
	AsmData d;
	d.cmd_pos = l->num;
	d.size = size;
	CurrentMetaInfo->data.add(d);
}

void list_set_bits(InstructionWithParamsList *list, int offset, int size) {
	state.set_bits(size);
	state.reset(list);
	if (CurrentMetaInfo) {
		CurrentMetaInfo->bits_size = size;
		BitChange b;
		b.cmd_pos = offset;
		b.bits_size = size;
		CurrentMetaInfo->bit_change.add(b);
	}
}

void InstructionWithParamsList::append_from_source(const string &_code) {
	const char *code = _code.c_str();

	if (!CurrentMetaInfo)
		raise_error("no CurrentMetaInfo");

	state.line_no = CurrentMetaInfo->line_offset;
	state.column_no = 0;

	// CurrentMetaInfo->CurrentOpcodePos // Anfang aktuelle Zeile im gesammten Opcode
	code_buffer = code; // Asm-Source-Puffer

	int pos = 0;
	InstructionParam p1, p2, p3;
	state.set_bits(instruction_set.pointer_size);
	if (CurrentMetaInfo)
		state.set_bits(CurrentMetaInfo->bits_size);
	state.end_of_code = false;
	while (pos < _code.num - 2) {

		string cmd, param1, param2, param3;

		//msg_write("..");
		state.reset(this);


	// interpret asm code (1 line)
		// find command
		cmd = find_mnemonic(pos);
		current_line = state.line_no;
		current_col = state.column_no;
		//msg_write(cmd);
		if (cmd.num == 0)
			break;
		// find parameters
		if (!state.end_of_line) {
			param1 = find_mnemonic(pos);
			if ((param1 == "dword") or (param1 == "word") or (param1 == "qword") or (param1 == "s48")) {
				if (param1 == "word")
					state.explicit_size = SIZE_16;
				else if (param1 == "dword")
					state.explicit_size = SIZE_32;
				else if (param1 == "qword")
					state.explicit_size = SIZE_64;
				else if (param1 == "s48")
					state.explicit_size = SIZE_48;
				if (!state.end_of_line)
					param1 = find_mnemonic(pos);
			}
		}
		if (!state.end_of_line)
			param2 = find_mnemonic(pos);
		if (!state.end_of_line)
			param3 = find_mnemonic(pos);
		//msg_write(string2("----: %s %s%s %s", cmd, param1, (strlen(param2)>0)?",":"", param2));
		if (state.end_of_code)
			break;
		so("------------------------------");
		so(cmd);
		so(param1);
		so(param2);
		so(param3);
		so("------");

		// parameters
		parse_parameter(p1, param1, *this, 0);
		parse_parameter(p2, param2, *this, 1);
		parse_parameter(p3, param3, *this, 2);
		if ((p1.type == ParamType::INVALID) or (p2.type == ParamType::INVALID) or (p3.type == ParamType::INVALID))
			return;

	// special stuff
		if (cmd == "bits_16") {
			so("16 bit Modus!");
			list_set_bits(this, num, SIZE_16);
			continue;
		} else if (cmd == "bits_32") {
			so("32 bit Modus!");
			list_set_bits(this, num, SIZE_32);
			continue;
		} else if (cmd == "bits_64") {
			so("64 bit Modus!");
			list_set_bits(this, num, SIZE_64);
			continue;

		} else if (cmd == "db") {
			add_data_inst(this, SIZE_8);
		} else if (cmd == "dw") {
			add_data_inst(this, SIZE_16);
		} else if (cmd == "dd") {
			add_data_inst(this, SIZE_32);
		} else if (cmd == "dq") {
			add_data_inst(this, SIZE_64);
		}/*else if ((cmd == "ds") or (cmd == "dz")) {
			so("Daten:   String");
			char *s = (char*)p1.value;
			int l=strlen(s);
			if (cmd == "dz")
				l ++;
			if (CurrentMetaInfo) {
				AsmData d;
				d.cmd_pos = num;
				d.size = l;
				d.data = new char[l];
				memcpy(d.data, s, l);
				CurrentMetaInfo->data.add(d);
			}
			//memcpy(&buffer[CodeLength], s, l);
			//CodeLength += l;
			continue;
		}*/else if (cmd[cmd.num - 1] == ':') {
			so("Label");
			cmd.resize(cmd.num - 1);
			so(cmd);
			int l = find_or_create_label(cmd);
			insert_location_label(l);

			continue;
		}


		InstructionWithParams iwp;
		iwp.condition = ArmCond::Always;

		if (cmd.find(":") >= 0) {
			iwp.condition = ArmCond::Unknown;
			auto l = cmd.explode(":");
			for (int i=0; i<16; i++)
				if (l[0] == ARMConditions[i])
					iwp.condition = (ArmCond)i;
			if (iwp.condition == ArmCond::Unknown)
				raise_error("unknown condition: " + l[0]);
			cmd = l[1];
		}

		// command
		InstID inst = InstID::INVALID;
		for (int i=0; i<(int)InstID::NUM_INSTRUCTION_NAMES; i++)
			if (string(instruction_names[i].name) == cmd)
				inst = instruction_names[i].inst;
		if (inst == InstID::INVALID)
			raise_error("unknown instruction:  " + cmd);
		// prefix
		if (state.param_size != state.default_param_size) {
			//buffer[CodeLength ++] = 0x66;
			//raise_error("prefix unhandled:  " + cmd);
		}
		iwp.inst = inst;
		iwp.p[0] = p1;
		iwp.p[1] = p2;
		iwp.p[2] = p3;
		iwp.line = current_line;
		iwp.col = current_col;
		add(iwp);


		if (state.end_of_code)
			break;
	}
}


// convert human readable asm code into opcode
bool assemble(const char *code, char *oc, int &ocs) {
	/*if (!Instruction)
		SetInstructionSet(InstructionSetDefault);*/

	InstructionWithParamsList list = InstructionWithParamsList(CurrentMetaInfo->line_offset);

	list.append_from_source(code);

	list.optimize(oc, ocs);

	// compile commands
	list.compile(oc, ocs);

	return true;
}

inline bool _size_match_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p) {
	if (inst_p.size == wanted_p.size)
		return true;
	if (wanted_p.reg)
		if ((inst_p.reg_group == RegGroup::XMM) and (wanted_p.reg->group == RegGroup::XMM))
			return true;
	if ((inst_p.size == SIZE_UNKNOWN) or (wanted_p.size == SIZE_UNKNOWN))
		return true;
/*	if ((inst_p.size == SizeVariable) and ((wanted_p.size == Size16) or (wanted_p.size == Size32)))
		return true;*/
	return false;
}

inline bool _deref_match_(InstructionParamFuzzy &inst_p, InstructionParam &wanted_p) {
	if (wanted_p.deref)
		return (inst_p.allow_memory_address) or (inst_p.allow_memory_indirect);
	return true;
}

bool reg_between(RegID r, RegID a, RegID b) {
	return ((int)r >= (int)a) and ((int)r <= (int)b);
}

bool InstructionParamFuzzy::match(InstructionParam &wanted_p) {
	//ParamFuzzyOut(&inst_p);
	
	// none
	if ((wanted_p.type == ParamType::NONE) or (!used))
		return (wanted_p.type == ParamType::NONE) and (!used);

	// xmm register...
	if ((allow_register) and (wanted_p.type == ParamType::REGISTER) and (wanted_p.reg)) {
		if ((reg_group == RegGroup::XMM) and (wanted_p.reg->group == RegGroup::XMM))
			return true;
	}

	// size mismatch?
	if ((size != SIZE_UNKNOWN) and (wanted_p.size != SIZE_UNKNOWN))
		if (size != wanted_p.size)
			return false;

	// immediate
	if (wanted_p.type == ParamType::IMMEDIATE) {
		if ((allow_memory_address) and (wanted_p.deref))
			return true;
		if ((allow_immediate) and (!wanted_p.deref)) {
			//msg_write("imm " + SizeOut(inst_p.size) + " " + SizeOut(wanted_p.size));
			return (size == wanted_p.size);
		}
		return false;
	}

	// immediate double
	/*if (wanted_p.type == ParamTImmediateExt) {
		msg_write("imx");
		if (allow_memory_address)
			return (size == wanted_p.size);
	}*/

	// reg
	if (wanted_p.type == ParamType::REGISTER) {
		// direct match
		if ((allow_register) and (reg)) {
			if (wanted_p.reg) {
				if (reg_between(reg->id, RegID::RAX, RegID::RBP) and ((int)wanted_p.reg->id == (int)reg->id + (int)RegID::R8 - (int)RegID::RAX))
					return true;
			}
			return ((reg == wanted_p.reg) and (_deref_match_(*this, wanted_p)));
		}
		// fuzzy match
		/*if (inst_p.allow_register) {
			msg_write("r2");
			
			return ((inst_p.reg_group == wanted_p.reg->group) and (_size_match_(inst_p, wanted_p)) and (_deref_match_(inst_p, wanted_p)));
		}*/
		// very fuzzy match
		if (allow_register or allow_memory_indirect) {
			if (wanted_p.deref) {
				if (allow_memory_indirect)
					return ((RegGroup::GENERAL == wanted_p.reg->group) and (_deref_match_(*this, wanted_p)));
			} else if (allow_register) {
				return ((reg_group == wanted_p.reg->group) and (_size_match_(*this, wanted_p))); // FIXME (correct?)
			}
		}
	}

	return false;
}


static bool is_jump(InstID id) {
	if (id == InstID::JMP)
		return true;
	if ((id == InstID::JZ) or (id == InstID::JNZ) or (id == InstID::JL) or (id == InstID::JNL) or (id == InstID::JLE) or (id == InstID::JNLE))
		return true;
	if ((id == InstID::JB) or (id == InstID::JNB) or (id == InstID::JS) or (id == InstID::JNS) or (id == InstID::JO) or (id == InstID::JNO))
		return true;
	return false;
}

void InstructionWithParamsList::shrink_jumps(void *oc, int ocs) {
	// first pass compilation (we need real jump distances)
	int _ocs = ocs;
	compile(oc, _ocs);
	wanted_label.clear();

	// try shrinking
	foreachi(InstructionWithParams &iwp, *this, i) {
		if (is_jump(iwp.inst)) {
			if (iwp.p[0].is_label) {
				int target = label[(int)iwp.p[0].value].inst_no;

				// jump distance
				int dist = 0;
				for (int j=i+1;j<target;j++)
					dist += (*this)[j].size;
				for (int j=target;j<=i;j++)
					dist += (*this)[j].size;
				//msg_write(format("%d %d   %d", i, target, dist));

				if (dist < 127) {
					so("really shrink");
					iwp.p[0].size = SIZE_8;
				}
			}
		}
	}
}

void InstructionWithParamsList::optimize(void *oc, int ocs) {
	if ((instruction_set.set == InstructionSet::X86) or (instruction_set.set == InstructionSet::AMD64))
		shrink_jumps(oc, ocs);
}

void InstructionWithParamsList::compile(void *oc, int &ocs) {
	state.reset(this);
	if (!CurrentMetaInfo) {
		DummyMetaInfo.bits_size = instruction_set.pointer_size;
		DummyMetaInfo.code_origin = (int_p)oc;
		CurrentMetaInfo = &DummyMetaInfo;
	}
	state.set_bits(CurrentMetaInfo->bits_size);

	// label lookup acceleration
	Array<Label*> label_sorted;
	label_sorted.resize((num+1)*16);
	Array<int> label_count;
	label_count.resize(num+1);
	for (auto &l: label) {
		int n = label_count[l.inst_no] ++;
		label_sorted[l.inst_no * 16 + n] = &l;
	}

	for (int i=0;i<num+1;i++) {
		state.line_no = i;
		// bit change
		for (BitChange &b: CurrentMetaInfo->bit_change)
			if (b.cmd_pos == i) {
				state.set_bits(b.bits_size);
				b.offset = ocs;
			}

		// data?
		for (AsmData &d: CurrentMetaInfo->data)
			if (d.cmd_pos == i)
				d.offset = ocs;

		// defining a label?
		/*for (auto &l: label)
			if (i == l.inst_no) {*/
		for (int j=0; j<label_count[i]; j++) {
			auto &l = *label_sorted[i*16+j];
			so("defining found: " + l.name);
			l.value = CurrentMetaInfo->code_origin + ocs;
		}
		if (i >= num)
			break;

		// opcode
		if (instruction_set.set == InstructionSet::ARM64)
			add_instruction_arm64((char*)oc, ocs, i);
		else if (instruction_set.set == InstructionSet::ARM32)
			add_instruction_arm32((char*)oc, ocs, i);
		else
			add_instruction((char*)oc, ocs, i);
	}

	link_wanted_labels(oc);

	for (WantedLabel &l: wanted_label) {
		state.line_no = (*this)[l.inst_no].line;
		state.column_no = (*this)[l.inst_no].col;
		raise_error("undeclared label used: " + l.name);
	}
}

void add_instruction(char *oc, int &ocs, InstID inst, const InstructionParam &p1, const InstructionParam &p2, const InstructionParam &p3) {
	/*if (!CPUInstructions)
		SetInstructionSet(InstructionSetDefault);*/
	state.set_bits(instruction_set.pointer_size);
	state.reset(nullptr);
	/*msg_write("--------");
	for (int i=0;i<NUM_INSTRUCTION_NAMES;i++)
		if (InstructionName[i].inst == inst)
			printf("%s\n", InstructionName[i].name);*/

	OCParam = ocs;
	auto list = InstructionWithParamsList(0);
	InstructionWithParams iwp;
	iwp.inst = inst;
	iwp.p[0] = p1;
	iwp.p[1] = p2;
	iwp.p[2] = p3;
	iwp.line = -1;
	list.add(iwp);
	list.add_instruction(oc, ocs, 0);
}

};
