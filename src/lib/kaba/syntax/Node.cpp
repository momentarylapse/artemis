/*
 * Node.cpp
 *
 *  Created on: 18.02.2019
 *      Author: michi
 */
#include "../kaba.h"
#include "../asm/asm.h"
#include "../../base/iter.h"
#include "../../os/msg.h"
#include <stdio.h>

namespace kaba {

string kind2str(NodeKind kind) {
	if (kind == NodeKind::Placeholder)
		return "placeholder";
	if (kind == NodeKind::VarLocal)
		return "local";
	if (kind == NodeKind::VarGlobal)
		return "global";
	if (kind == NodeKind::Function)
		return "function name";
	if (kind == NodeKind::Constant)
		return "constant";
	if (kind == NodeKind::ConstantByAddress)
		return "constant by addr";
	if (kind == NodeKind::CallFunction)
		return "call";
	if (kind == NodeKind::CallRawPointer)
		return "raw pointer call";
	if (kind == NodeKind::CallInline)
		return "inline";
	if (kind == NodeKind::CallVirtual)
		return "virtual call";
	if (kind == NodeKind::Statement)
		return "statement";
	if (kind == NodeKind::CallSpecialFunction)
		return "special function call";
	if (kind == NodeKind::SpecialFunctionName)
		return "special function name";
	if (kind == NodeKind::Operator)
		return "operator";
	if (kind == NodeKind::NamedParameter)
		return "named parameter";
	if (kind == NodeKind::AbstractToken)
		return "token";
	if (kind == NodeKind::AbstractOperator)
		return "abstract operator";
	if (kind == NodeKind::AbstractElement)
		return "abstract element";
	if (kind == NodeKind::AbstractCall)
		return "abstract call";
	if (kind == NodeKind::AbstractTypeReference)
		return "reference";
	if (kind == NodeKind::AbstractTypeList)
		return "list";
	if (kind == NodeKind::AbstractTypeDict)
		return "dict";
	if (kind == NodeKind::AbstractTypeOptional)
		return "optional";
	if (kind == NodeKind::AbstractTypeCallable)
		return "callable type";
	if (kind == NodeKind::AbstractVar)
		return "var";
	if (kind == NodeKind::Block)
		return "block";
	if (kind == NodeKind::AddressShift)
		return "address shift";
	if (kind == NodeKind::Array)
		return "array element";
	if (kind == NodeKind::DynamicArray)
		return "dynamic array element";
	if (kind == NodeKind::PointerAsArray)
		return "pointer as array element";
	if (kind == NodeKind::Reference)
		return "reference operator";
	if (kind == NodeKind::Dereference)
		return "dereferencing";
	if (kind == NodeKind::DereferenceAddressShift)
		return "deref address shift";
	if (kind == NodeKind::Definitely)
		return "definitely";
	if (kind == NodeKind::Class)
		return "class";
	if (kind == NodeKind::ArrayBuilder)
		return "array builder";
	if (kind == NodeKind::ArrayBuilderFor)
		return "array builder for";
	if (kind == NodeKind::ArrayBuilderForIf)
		return "array builder for if";
	if (kind == NodeKind::DictBuilder)
		return "dict builder";
	if (kind == NodeKind::Tuple)
		return "tuple";
	if (kind == NodeKind::TupleExtraction)
		return "tuple extract";
	if (kind == NodeKind::ConstructorAsFunction)
		return "constructor function";
	if (kind == NodeKind::Slice)
		return "slice";
	if (kind == NodeKind::VarTemp)
		return "temp";
	if (kind == NodeKind::DereferenceVarTemp)
		return "deref temp";
	if (kind == NodeKind::Register)
		return "register";
	if (kind == NodeKind::Address)
		return "address";
	if (kind == NodeKind::Memory)
		return "memory";
	if (kind == NodeKind::LocalAddress)
		return "local address";
	if (kind == NodeKind::LocalMemory)
		return "local memory";
	if (kind == NodeKind::DereferenceRegister)
		return "deref register";
	if (kind == NodeKind::Label)
		return "label";
	if (kind == NodeKind::DereferenceLabel)
		return "deref label";
	if (kind == NodeKind::GlobalLookup)
		return "global lookup";
	if (kind == NodeKind::DereferenceGlobalLookup)
		return "deref global lookup";
	if (kind == NodeKind::Immediate)
		return "immediate";
	if (kind == NodeKind::DerefereceLocalMemory)
		return "deref local";
	return format("UNKNOWN KIND: %d", (int)kind);
}


string Node::signature(const Class *ns) const {
	//string t = (kind == NodeKind::ABSTRACT_TOKEN) ? " " : type->cname(ns) + " ";
	string t = ": " + type->cname(ns);
	if (kind == NodeKind::Placeholder)
		return "";
	if (kind == NodeKind::VarLocal)
		return as_local()->name + t;
	if (kind == NodeKind::VarGlobal)
		return as_global()->name + t;
	if (kind == NodeKind::Function)
		return as_func()->cname(ns) + t;
	if (kind == NodeKind::Constant)
		return as_const()->str() + t;
	if (kind == NodeKind::CallFunction)
		return as_func()->signature(ns) + t;
	if (kind == NodeKind::CallRawPointer)
		return "(...)" + t;
	if (kind == NodeKind::CallInline)
		return as_func()->signature(ns);
	if (kind == NodeKind::CallVirtual)
		return as_func()->signature(ns);
	if (kind == NodeKind::ConstructorAsFunction)
		return as_func()->signature(ns);
	if (kind == NodeKind::Slice)
		return t;
	if (kind == NodeKind::Statement)
		return as_statement()->name + t;
	if (kind == NodeKind::CallSpecialFunction)
		return as_special_function()->name + t;
	if (kind == NodeKind::SpecialFunctionName)
		return as_special_function()->name + t;
	if (kind == NodeKind::Operator)
		return as_op()->sig(ns);
	if (kind == NodeKind::AbstractToken)
		return as_token();
	if (kind == NodeKind::AbstractOperator)
		return as_abstract_op()->name;
	if (kind == NodeKind::Block)
		return (type == TypeVoid) ? "" : t; //p2s(as_block());
	if (kind == NodeKind::AddressShift)
		return ::str(link_no) + t;
	if (kind == NodeKind::Array)
		return t;
	if (kind == NodeKind::DynamicArray)
		return t;
	if (kind == NodeKind::PointerAsArray)
		return t;
	if (kind == NodeKind::Reference)
		return t;
	if (kind == NodeKind::Dereference)
		return t;
	if (kind == NodeKind::Definitely)
		return t;
	if (kind == NodeKind::DereferenceAddressShift)
		return ::str(link_no) + t;
	if (kind == NodeKind::Class)
		return as_class()->cname(ns);
	if (kind == NodeKind::Register)
		return Asm::get_reg_name((Asm::RegID)link_no) + t;
	if (kind == NodeKind::Address)
		return i2h(link_no, config.target.pointer_size) + t;
	if (kind == NodeKind::Memory)
		return i2h(link_no, config.target.pointer_size) + t;
	if (kind == NodeKind::LocalAddress)
		return i2h(link_no, config.target.pointer_size) + t;
	if (kind == NodeKind::LocalMemory)
		return i2h(link_no, config.target.pointer_size) + t;
	return ::str(link_no) + t;
}

string Node::str(const Class *ns) const {
	return "<" + kind2str(kind) + ">  " + signature(ns);
}


void Node::show(const Class *ns) const {
	string orig;
	msg_write(str(ns) + orig);
	msg_right();
	for (auto p: params)
		if (p)
			p->show(ns);
		else
			msg_write("<NULL>");
	msg_left();
}





// policy:
//  don't change after creation...
//  edit the tree by shallow copy, relink to old parameters
//  relinked params count as "new" Node!
// ...(although, Block are allowed to be edited)
Node::Node(NodeKind _kind, int64 _link_no, const Class *_type, Flags _flags, int _token_id) {
	type = _type;
	kind = _kind;
	link_no = _link_no;
	flags = _flags;
	token_id = _token_id;
}

Node::~Node() {
	if (kind == NodeKind::Block)
		as_block()->vars.clear();
}

bool Node::is_mutable() const {
	return flags_has(flags, Flags::Mutable);
}

void Node::set_mutable(bool _mutable) {
	if (_mutable)
		flags_set(flags, Flags::Mutable);
	else
		flags_clear(flags, Flags::Mutable);
}

bool Node::is_call() const {
	return (kind == NodeKind::CallFunction) or (kind == NodeKind::CallVirtual) or (kind == NodeKind::CallRawPointer);
}

bool Node::is_function() const {
	return (kind == NodeKind::CallFunction) or (kind == NodeKind::CallVirtual) or (kind == NodeKind::CallInline) or (kind == NodeKind::ConstructorAsFunction);
}

Block *Node::as_block() const {
	return (Block*)this;
}

Function *Node::as_func() const {
	return (Function*)link_no;
}

const Class *Node::as_class() const {
	return (const Class*)link_no;
}

Constant *Node::as_const() const {
	return (Constant*)link_no;
}

Operator *Node::as_op() const {
	return (Operator*)link_no;
}
void *Node::as_func_p() const {
	return (void*)as_func()->address;
}

// will be the address at runtime...(not the current location...)
void *Node::as_const_p() const {
	return as_const()->address_runtime;
}

void *Node::as_global_p() const {
	return as_global()->memory;
}

Variable *Node::as_global() const {
	return (Variable*)link_no;
}

Variable *Node::as_local() const {
	return (Variable*)link_no;
}

Statement *Node::as_statement() const {
	return (Statement*)link_no;
}

SpecialFunction *Node::as_special_function() const {
	return (SpecialFunction*)link_no;
}

AbstractOperator *Node::as_abstract_op() const {
	return (AbstractOperator*)link_no;
}

string Node::as_token() const {
	return reinterpret_cast<SyntaxTree*>((int_p)link_no)->expressions.get_token(token_id);
}

void Node::set_instance(shared<Node> p) {
#ifndef NDEBUG
	if (params.num == 0)
		msg_write("no inst...dfljgkldfjg");
#endif
	params[0] = p;
	if (this->_pointer_ref_counter > 1) {
		msg_write("iii");
		msg_write(msg_get_trace());
	}
}

void Node::set_type(const Class *t) {
	type = t;
	if (this->_pointer_ref_counter > 1) {
		msg_write("ttt");
		msg_write(msg_get_trace());
	}
}

void Node::set_num_params(int n) {
	params.resize(n);
	if (this->_pointer_ref_counter > 1) {
		msg_write("nnn");
		msg_write(msg_get_trace());
	}
}

void Node::set_param(int index, shared<Node> p) {
#ifndef NDEBUG
	/*if ((index < 0) or (index >= uparams.num)){
		show();
		throw Exception(format("internal: Node.set_param...  %d %d", index, params.num), "", 0);
	}*/
#endif
	params[index] = p;
#if 0
	if (this->_pointer_ref_counter > 1) {
		msg_write("ppp");
		msg_write(msg_get_trace());
	}
#endif
}

shared<Node> Node::shallow_copy() const {
	auto r = new Node(kind, link_no, type, flags, token_id);
	r->params = params;
	return r;
}

shared<Node> Node::ref(const Class *t) const {
	shared<Node> c = new Node(NodeKind::Reference, 0, t, flags, token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}

shared<Node> Node::ref(SyntaxTree *tree) const {
	return ref(tree->request_implicit_class_reference(type, token_id));
}

shared<Node> Node::deref(const Class *override_type) const {
	if (!override_type)
		override_type = type->param[0];
	shared<Node> c = new Node(NodeKind::Dereference, 0, override_type, flags, token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}

shared<Node> Node::shift(int64 shift, const Class *type, int token_id) const {
	shared<Node> c = new Node(NodeKind::AddressShift, shift, type, flags, token_id >= 0 ? token_id : this->token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}

shared<Node> Node::deref_shift(int64 shift, const Class *type, int token_id) const {
	shared<Node> c = new Node(NodeKind::DereferenceAddressShift, shift, type, flags, token_id);
	c->set_num_params(1);
	c->set_param(0, const_cast<Node*>(this));
	return c;
}

shared<Node> Node::change_type(const Class *type, int token_id) const {
	return shift(0, type, token_id);
	// FIXME: simply changing the type causes bugs (due to temp vars/constructors etc missing)
	/*auto c = shallow_copy();
	c->type = type;
	return c;*/
}


// recursive
shared<Node> cp_node(shared<Node> c, Block *parent_block) {
	shared<Node> cmd;
	if (c->kind == NodeKind::Block) {
		if (!parent_block)
			parent_block = c->as_block()->parent;
		cmd = new Block(c->as_block()->function, parent_block, c->type);
		cmd->as_block()->vars = c->as_block()->vars;
		parent_block = cmd->as_block();
	} else {
		cmd = new Node(c->kind, c->link_no, c->type, c->flags);
	}
	cmd->token_id = c->token_id;
	cmd->set_num_params(c->params.num);
	for (int i=0;i<c->params.num;i++)
		if (c->params[i])
			cmd->set_param(i, cp_node(c->params[i], parent_block));
	return cmd;
}



shared<Node> add_node_constructor(const Function *f, int token_id) {
	auto *dummy = new Node(NodeKind::Placeholder, 0, f->name_space, Flags::Mutable);
	auto n = add_node_member_call(f, dummy, token_id); // temp var added later...
	n->kind = NodeKind::ConstructorAsFunction;
	n->type = f->name_space;
	return n;
}

shared<Node> add_node_const(const Constant *c, int token_id) {
	return new Node(NodeKind::Constant, (int_p)c, c->type.get(), Flags::None, token_id);
}

/*shared<Node> add_node_block(Block *b) {
	return new Node(NodeKind::BLOCK, (int_p)b, TypeVoid);
}*/

shared<Node> add_node_statement(StatementID id, int token_id, const Class *type) {
	auto *s = statement_from_id(id);
	auto c = new Node(NodeKind::Statement, (int_p)s, type, Flags::None, token_id);
	c->set_num_params(s->num_params);
	return c;
}

shared<Node> add_node_special_function_call(SpecialFunctionID id, int token_id, const Class *type) {
	auto *s = special_function_from_id(id);
	auto c = new Node(NodeKind::CallSpecialFunction, (int_p)s, type, Flags::None, token_id);
	c->set_num_params(s->max_params);
	return c;
}

shared<Node> add_node_special_function_name(SpecialFunctionID id, int token_id, const Class *type) {
	auto *s = special_function_from_id(id);
	auto c = new Node(NodeKind::SpecialFunctionName, (int_p)s, type, Flags::None, token_id);
	return c;
}

// virtual call, if func is virtual
shared<Node> add_node_member_call(const Function *f, const shared<Node> inst, int token_id, const shared_array<Node> &params, bool force_non_virtual) {
	shared<Node> c;
	if ((f->virtual_index >= 0) and !force_non_virtual) {
		c = new Node(NodeKind::CallVirtual, (int_p)f, f->literal_return_type, Flags::None, token_id);
	} else {
		c = new Node(NodeKind::CallFunction, (int_p)f, f->literal_return_type, Flags::None, token_id);
	}
	c->set_num_params(f->num_params);
	c->set_instance(inst);
	for (auto&& [i,p]: enumerate(params))
		c->set_param(i + 1, p);
	return c;
}

// non-member!
shared<Node> add_node_call(const Function *f, int token_id) {
	// FIXME: literal_return_type???
	shared<Node> c = new Node(NodeKind::CallFunction, (int_p)f, f->literal_return_type, Flags::None, token_id);
		c->set_num_params(f->num_params);
	return c;
}

shared<Node> add_node_func_name(const Function *f, int token_id) {
	return new Node(NodeKind::Function, (int_p)f, TypeUnknown, Flags::None, token_id);
}

shared<Node> add_node_class(const Class *c, int token_id) {
	return new Node(NodeKind::Class, (int_p)c, TypeClassRef, Flags::None, token_id);
}


shared<Node> add_node_operator(const Operator *op, const shared<Node> p1, const shared<Node> p2, int token_id, const Class *override_type) {
	if (!override_type)
		override_type = op->return_type;
	shared<Node> cmd = new Node(NodeKind::Operator, (int_p)op, override_type, Flags::None, token_id);
	if (op->abstract->is_binary()) {
		cmd->set_num_params(2); // binary
		cmd->set_param(0, p1);
		cmd->set_param(1, p2);
	} else {
		cmd->set_num_params(1); // unary
		cmd->set_param(0, p1);
	}
	return cmd;
}


shared<Node> add_node_local(const Variable *v, const Class *type, int token_id) {
	return new Node(NodeKind::VarLocal, (int_p)v, type, v->flags, token_id);
}

shared<Node> add_node_local(const Variable *v, int token_id) {
	return new Node(NodeKind::VarLocal, (int_p)v, v->type, v->flags, token_id);
}

shared<Node> add_node_global(const Variable *v, int token_id) {
	return new Node(NodeKind::VarGlobal, (int_p)v, v->type, v->flags, token_id);
}

shared<Node> add_node_parray(shared<Node> p, shared<Node> index, const Class *type) {
	shared<Node> cmd_el = new Node(NodeKind::PointerAsArray, 0, type, p->flags, index->token_id);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, p);
	cmd_el->set_param(1, index);
	return cmd_el;
}

shared<Node> add_node_dyn_array(shared<Node> array, shared<Node> index) {
	shared<Node> cmd_el = new Node(NodeKind::DynamicArray, 0, array->type->get_array_element(), array->flags, index->token_id);
	cmd_el->set_num_params(2);
	cmd_el->set_param(0, array);
	cmd_el->set_param(1, index);
	return cmd_el;
}

shared<Node> add_node_array(shared<Node> array, shared<Node> index, const Class *type) {
	if (!type)
		type = array->type->param[0];
	auto *el = new Node(NodeKind::Array, 0, type, array->flags, index->token_id);
	el->set_num_params(2);
	el->set_param(0, array);
	el->set_param(1, index);
	return el;
}

shared<Node> add_node_slice(shared<Node> start, shared<Node> end) {
	auto n = new Node(NodeKind::Slice, 0, TypeUnknown);
	n->set_num_params(2);
	n->set_param(0, start);
	n->set_param(1, end);
	return n;
}

shared<Node> make_constructor_static(shared<Node> n, const string &name) {
	for (auto *f: weak(n->type->functions))
		if (f->name == name) {
			auto nn = add_node_call(f, n->token_id);
			for (int i=0; i<n->params.num-1; i++)
				nn->set_param(i, n->params[i+1]);
			//nn->params = n->params.sub(1,-1);
			return nn;
		}
	return n;
}

shared<Node> add_node_named_parameter(SyntaxTree* tree, int name_token_id, shared<Node> param) {
	auto n = new Node(NodeKind::NamedParameter, 0, param->type, Flags::None, name_token_id);
	n->set_num_params(2);
	n->set_param(0, add_node_token(tree, name_token_id));
	n->set_param(1, param);
	return n;
}
shared<Node> add_node_token(SyntaxTree* tree, int token_id) {
	return new Node(NodeKind::AbstractToken, (int_p)tree, TypeUnknown, Flags::None, token_id);
}

shared<Node> add_node_operator_by_inline(InlineID inline_index, const shared<Node> p1, const shared<Node> p2, int token_id, const Class *override_type) {
	for (auto op: weak(default_context->global_operators))
		if (op->f->inline_no == inline_index)
			return add_node_operator(op, p1, p2, token_id, override_type);

	throw Exception(format("INTERNAL ERROR: operator inline index not found: %d", (int)inline_index), "", -1, -1, nullptr);
	return nullptr;
}



Array<const Class*> node_extract_param_types(const shared<Node> n) {
	Array<const Class*> classes;
	for (auto p: weak(n->params))
		classes.add(p->type);
	return classes;
}

bool node_is_member_function_with_instance(shared<Node> n) {
	if (!n->is_function())
		return false;
	auto *f = n->as_func();
	if (f->is_static())
		return false;
	return n->params.num == 0 or n->params[0];
}

bool is_type_tuple(const shared<Node> n) {
	if (n->kind != NodeKind::Tuple)
		return false;
	for (auto p: weak(n->params))
		if (p->kind != NodeKind::Class)
			return false;
	return true;
}

Array<const Class*> class_tuple_extract_classes(const shared<Node> n) {
	Array<const Class*> classes;
	for (auto p: weak(n->params))
		classes.add(p->as_class());
	return classes;
}

}

