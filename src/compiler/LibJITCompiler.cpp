#include "LibJITCompiler.hpp"
#include "../vm/VM.hpp"
#include "../vm/value/LSNull.hpp"
#include "../vm/value/LSArray.hpp"
#include "../vm/value/LSMap.hpp"
#include "../vm/value/LSClosure.hpp"
#include "../vm/Program.hpp"
#include "../../lib/utf8.h"
#include "../vm/LSValue.hpp"
#include <jit/jit-dump.h>
#include "../colors.h"

#define log_insn(i) log_instructions && _log_insn((i))

namespace ls {

LibJITCompiler::LibJITCompiler(VM* vm) : vm(vm) {}

LibJITCompiler::~LibJITCompiler() {}

void LibJITCompiler::enter_block() {
	variables.push_back(std::map<std::string, value> {});
	if (!loops_blocks.empty()) {
		loops_blocks.back()++;
	}
	functions_blocks.back()++;
}

void LibJITCompiler::leave_block() {
	delete_variables_block(1);
	variables.pop_back();
	if (!loops_blocks.empty()) {
		loops_blocks.back()--;
	}
	functions_blocks.back()--;
}

void LibJITCompiler::delete_variables_block(int deepness) {
	for (int i = variables.size() - 1; i >= (int) variables.size() - deepness; --i) {
		for (auto it = variables[i].begin(); it != variables[i].end(); ++it) {
			insn_delete(it->second);
		}
	}
}

void LibJITCompiler::delete_function_variables() {
	for (const auto& v : function_variables.back()) {
		insn_delete(v);
	}
}

void LibJITCompiler::enter_function(jit_function_t F, bool is_closure, Function* fun) {
	variables.push_back(std::map<std::string, value> {});
	function_variables.push_back(std::vector<value> {});
	functions.push(F);
	functions_blocks.push_back(0);
	catchers.push_back({});
	function_is_closure.push(is_closure);
	this->F = F;

	std::vector<std::string> args;
	log_insn(0) << "function " << fun->name << "(";
	for (unsigned i = 0; i < fun->arguments.size(); ++i) {
		log_insn(0) << fun->arguments.at(i)->content;
		if (i < fun->arguments.size() - 1) log_insn(0) << ", ";
		args.push_back(fun->arguments.at(i)->content);
	}
	arg_names.push(args);
	log_insn(0) << ") {" << std::endl;
}

void LibJITCompiler::leave_function() {
	variables.pop_back();
	function_variables.pop_back();
	functions.pop();
	functions_blocks.pop_back();
	catchers.pop_back();
	function_is_closure.pop();
	arg_names.pop();
	this->F = functions.top();
	log_insn(0) << "}" << std::endl;
}

int LibJITCompiler::get_current_function_blocks() const {
	return functions_blocks.back();
}

bool LibJITCompiler::is_current_function_closure() const {
	return function_is_closure.size() ? function_is_closure.top() : false;
}

/*
 * Operators
 */
void LibJITCompiler::insn_store(LibJITCompiler::value a, LibJITCompiler::value b) const {
	jit_insn_store(F, a.v, b.v);
	log_insn(4) << "store " << dump_val(a) << " " << dump_val(b) << std::endl;
}
void LibJITCompiler::insn_store_relative(LibJITCompiler::value a, int pos, LibJITCompiler::value b) const {
	jit_insn_store_relative(F, a.v, pos, b.v);
	log_insn(4) << "store_rel " << dump_val(a) << " " << dump_val(b) << std::endl;
}
LibJITCompiler::value LibJITCompiler::insn_not(LibJITCompiler::value v) const {
	LibJITCompiler::value r {jit_insn_not(F, v.v), v.t};
	log_insn(4) << "not " << dump_val(v) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_not_bool(LibJITCompiler::value v) const {
	LibJITCompiler::value r {jit_insn_to_not_bool(F, v.v), Type::BOOLEAN};
	log_insn(4) << "not_bool " << dump_val(v) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_neg(LibJITCompiler::value v) const {
	LibJITCompiler::value r {jit_insn_neg(F, v.v), v.t};
	log_insn(4) << "neg " << dump_val(v) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_and(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_and(F, insn_to_bool(a).v, insn_to_bool(b).v), Type::BOOLEAN};
	log_insn(4) << "and " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_or(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_or(F, insn_to_bool(a).v, insn_to_bool(b).v), Type::BOOLEAN};
	log_insn(4) << "or " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_add(LibJITCompiler::value a, LibJITCompiler::value b) const {
	auto result_type = [&]() {
		if (a.t.nature == Nature::POINTER or b.t.nature == Nature::POINTER) return Type::POINTER;
		if (a.t == Type::REAL or b.t == Type::REAL) return Type::REAL;
		if (a.t == Type::LONG or b.t == Type::LONG) return Type::LONG;
		return Type::INTEGER;
	}();
	LibJITCompiler::value r {jit_insn_convert(F, jit_insn_add(F, a.v, b.v), result_type.jit_type(), 0), result_type};
	log_insn(4) << "add " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_sub(LibJITCompiler::value a, LibJITCompiler::value b) const {
	auto result_type = [&]() {
		if (a.t == Type::POINTER or b.t == Type::POINTER) return Type::POINTER;
		if (a.t == Type::REAL or b.t == Type::REAL) return Type::REAL;
		if (a.t == Type::LONG or b.t == Type::LONG) return Type::LONG;
		return Type::INTEGER;
	}();
	LibJITCompiler::value r {jit_insn_sub(F, a.v, b.v), result_type};
	log_insn(4) << "sub " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_eq(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_eq(F, a.v, b.v), Type::BOOLEAN};
	log_insn(4) << "eq " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_ne(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_ne(F, a.v, b.v), Type::BOOLEAN};
	log_insn(4) << "ne " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_lt(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_lt(F, a.v, b.v), Type::BOOLEAN};
	log_insn(4) << "lt " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_le(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_le(F, a.v, b.v), Type::BOOLEAN};
	log_insn(4) << "le " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_gt(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_gt(F, a.v, b.v), Type::BOOLEAN};
	log_insn(4) << "gt " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_ge(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_ge(F, a.v, b.v), Type::BOOLEAN};
	log_insn(4) << "ge " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_mul(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_mul(F, a.v, b.v), Type::INTEGER};
	log_insn(4) << "mul " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_div(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_div(F, jit_insn_convert(F, a.v, LS_REAL, 0), b.v), Type::REAL};
	log_insn(4) << "div " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_int_div(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_div(F, a.v, b.v), Type::INTEGER};
	log_insn(4) << "idiv " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_bit_and(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_and(F, a.v, b.v), Type::INTEGER};
	log_insn(4) << "bit_and " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_bit_or(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_or(F, a.v, b.v), Type::INTEGER};
	log_insn(4) << "bit_or " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_bit_xor(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_xor(F, a.v, b.v), Type::INTEGER};
	log_insn(4) << "bit_xor " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_mod(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_rem(F, a.v, b.v), Type::INTEGER};
	log_insn(4) << "mod " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_pow(LibJITCompiler::value a, LibJITCompiler::value b) const {
	LibJITCompiler::value r {jit_insn_pow(F, a.v, b.v), Type::INTEGER};
	log_insn(4) << "pow " << dump_val(a) << " " << dump_val(b) << " " << dump_val(r) << std::endl;
	return r;
}
LibJITCompiler::value LibJITCompiler::insn_log10(LibJITCompiler::value a) const {
	LibJITCompiler::value r {jit_insn_log10(F, a.v), Type::INTEGER};
	log_insn(4) << "log10 " << dump_val(a) << " " << dump_val(r) << std::endl;
	return r;
}

/*
 * Values
 */
LibJITCompiler::value LibJITCompiler::clone(LibJITCompiler::value v) const {
	if (v.t.must_manage_memory()) {
		if (v.t.reference) {
			v = insn_load(v);
		}
		auto r = insn_call(v.t, {v}, +[](LSValue* value) {
			return value->clone();
		}, "clone");
		log_insn(4) << "clone " << dump_val(v) << " " << dump_val(r) << std::endl;
		return r;
	}
	return v;
}
LibJITCompiler::value LibJITCompiler::new_null() const {
	return {jit_value_create_long_constant(F, LS_POINTER, (long) LSNull::get()), Type::ANY};
}
LibJITCompiler::value LibJITCompiler::new_bool(bool b) const {
	return {LS_CREATE_BOOLEAN(F, b), Type::BOOLEAN};
}
LibJITCompiler::value LibJITCompiler::new_integer(int i) const {
	return {LS_CREATE_INTEGER(F, i), Type::INTEGER};
}
LibJITCompiler::value LibJITCompiler::new_real(double r) const {
	return {jit_value_create_float64_constant(F, jit_type_float64, r), Type::REAL};
}
LibJITCompiler::value LibJITCompiler::new_long(long l) const {
	return {LS_CREATE_LONG(F, l), Type::LONG};
}
LibJITCompiler::value LibJITCompiler::new_pointer(const void* p) const {
	return {jit_value_create_long_constant(F, LS_POINTER, (long)(void*)(p)), Type::POINTER};
}
LibJITCompiler::value LibJITCompiler::new_mpz(long value) const {
	jit_value_t mpz_struct = jit_value_create(F, VM::mpz_type);
	jit_value_set_addressable(mpz_struct);
	auto mpz_addr = insn_address_of({mpz_struct, Type::MPZ});
	auto jit_value = new_long(value);
	insn_call(Type::VOID, {mpz_addr, jit_value}, &mpz_init_set_ui, "mpz_init_set_ui");
	VM::inc_mpz_counter(F);
	return {mpz_struct, Type::MPZ_TMP};
}
LibJITCompiler::value LibJITCompiler::new_object() const {
	return insn_call(Type::OBJECT_TMP, {}, +[]() {
		// FIXME coverage doesn't work for the one line version
		auto o = new LSObject();
		return o;
	}, "new_object");
}
LibJITCompiler::value LibJITCompiler::new_object_class(LibJITCompiler::value clazz) const {
	return insn_call(Type::POINTER, {clazz}, +[](LSClass* clazz) {
		return new LSObject(clazz);
	});
}
LibJITCompiler::value LibJITCompiler::new_array(Type element_type, std::vector<LibJITCompiler::value> elements) const {
	auto array = [&]() { if (element_type == Type::INTEGER) {
		return insn_call(Type::INT_ARRAY_TMP, {new_integer(elements.size())}, +[](int capacity) {
			auto array = new LSArray<int>();
			array->reserve(capacity);
			return array;
		}, "new_int_array");
	} else if (element_type == Type::REAL) {
		return insn_call(Type::REAL_ARRAY_TMP, {new_integer(elements.size())}, +[](int capacity) {
			auto array = new LSArray<double>();
			array->reserve(capacity);
			return array;
		}, "new_real_array");
	} else {
		return insn_call(Type::PTR_ARRAY_TMP, {new_integer(elements.size())}, +[](int capacity) {
			auto array = new LSArray<LSValue*>();
			array->reserve(capacity);
			return array;
		}, "new_ptr_array");
	}}();
	for (const auto& element : elements) {
		insn_push_array(array, element);
	}
	// size of the array + 1 operations
	inc_ops(elements.size() + 1);
	return array;
}

LibJITCompiler::value LibJITCompiler::new_mpz_init(const mpz_t mpz_value) const {
	jit_value_t mpz_struct = jit_value_create(F, VM::mpz_type);
	jit_value_set_addressable(mpz_struct);
	jit_insn_store_relative(F, jit_insn_address_of(F, mpz_struct), 0, LS_CREATE_INTEGER(F, mpz_value->_mp_alloc));
	jit_insn_store_relative(F, jit_insn_address_of(F, mpz_struct), 4, LS_CREATE_INTEGER(F, mpz_value->_mp_size));
	jit_insn_store_relative(F, jit_insn_address_of(F, mpz_struct), 8, new_pointer(mpz_value->_mp_d).v);
	return {mpz_struct, Type::MPZ};
}

LibJITCompiler::value LibJITCompiler::to_int(LibJITCompiler::value v) const {
	if (v.t.not_temporary() == Type::MPZ) {
		auto v_addr = insn_address_of(v);
		return to_int(insn_call(Type::LONG, {v_addr}, &mpz_get_si, "mpz_get_si"));
	}
	LibJITCompiler::value r {jit_insn_convert(F, v.v, LS_INTEGER, 0), Type::INTEGER};
	log_insn(4) << "to_int " << dump_val(v) << " " << dump_val(r) << std::endl;
	return r;
}

LibJITCompiler::value LibJITCompiler::to_real(LibJITCompiler::value v) const {
	if (v.t.not_temporary() == Type::MPZ) {
		auto v_addr = insn_address_of(v);
		return to_real(insn_call(Type::LONG, {v_addr}, &mpz_get_si, "mpz_get_si"));
	}
	LibJITCompiler::value r {jit_insn_convert(F, v.v, LS_REAL, 0), Type::REAL};
	log_insn(4) << "to_real " << dump_val(v) << " " << dump_val(r) << std::endl;
	return r;
}

LibJITCompiler::value LibJITCompiler::to_long(LibJITCompiler::value v) const {
	if (v.t.not_temporary() == Type::MPZ) {
		auto v_addr = insn_address_of(v);
		return insn_call(Type::LONG, {v_addr}, &mpz_get_si, "mpz_get_si");
	}
	LibJITCompiler::value r {jit_insn_convert(F, v.v, LS_LONG, 0), Type::LONG};
	log_insn(4) << "to_long " << dump_val(v) << " " << dump_val(r) << std::endl;
	return r;
}

LibJITCompiler::value LibJITCompiler::insn_convert(LibJITCompiler::value v, Type t) const {
	if (v.t.not_temporary() == t.not_temporary()) return v;
	if (t == Type::REAL) {
		return to_real(v);
	} else if (t == Type::INTEGER) {
		return to_int(v);
	} else if (t == Type::LONG) {
		return to_long(v);
	}
	return v;
}

LibJITCompiler::value LibJITCompiler::insn_create_value(Type t) const {
	return {jit_value_create(F, t.jit_type()), t};
}

LibJITCompiler::value LibJITCompiler::insn_to_pointer(LibJITCompiler::value v) const {
	if (v.t.nature == Nature::POINTER) {
		return v; // already a pointer
	}
	Type new_type = v.t;
	new_type.nature = Nature::POINTER;
	if (v.t.raw_type == RawType::LONG) {
		return insn_call(new_type, {v}, +[](long n) {
			return LSNumber::get(n);
		}, "new_number");
	} else if (v.t.raw_type == RawType::REAL) {
		return insn_call(new_type, {v}, +[](double n) {
			return LSNumber::get(n);
		}, "new_number");
	} else if (v.t.raw_type == RawType::BOOLEAN) {
		return insn_call(new_type, {v}, +[](bool n) {
			return LSBoolean::get(n);
		}, "new_bool");
	} else {
		return insn_call(new_type, {v}, +[](int n) {
			return LSNumber::get(n);
		}, "new_number");
	}
}

LibJITCompiler::value LibJITCompiler::insn_to_bool(LibJITCompiler::value v) const {
	if (v.t.raw_type == RawType::BOOLEAN) {
		return v;
	}
	if (v.t.raw_type == RawType::INTEGER) {
		LibJITCompiler::value r {jit_insn_to_bool(F, v.v), Type::BOOLEAN};
		log_insn(4) << "to_bool " << dump_val(v) << " " << dump_val(r) << std::endl;
		return r;
	}
	if (v.t.raw_type == RawType::STRING) {
		//return insn_call(Type::BOOLEAN, {v}, (void*) &LSString::to_bool, "String::to_bool");
	}
	if (v.t.raw_type == RawType::ARRAY) {
		// Always take LSArray<int>, but the array is not necessarily of this type
		//return insn_call(Type::BOOLEAN, {v}, (void*) &LSArray<int>::to_bool, "Array::to_bool");
	}
	if (v.t.raw_type == RawType::FUNCTION or v.t.raw_type == RawType::CLOSURE) {
		return new_bool(true);
	}
	if (v.t.raw_type == RawType::MPZ) {
		// TODO
		return v;
	}
	return insn_call(Type::BOOLEAN, {v}, +[](LSValue* v) {
		return v->to_bool();
	}, "Value::to_bool");
}

LibJITCompiler::value LibJITCompiler::insn_address_of(LibJITCompiler::value v) const {
	LibJITCompiler::value r {jit_insn_address_of(F, v.v), Type::POINTER};
	log_insn(4) << "addr " << dump_val(v) << " " << dump_val(r) << std::endl;
	return r;
}

LibJITCompiler::value LibJITCompiler::insn_load(LibJITCompiler::value v, int pos, Type t) const {
	LibJITCompiler::value r {jit_insn_load_relative(F, v.v, pos, t.jit_type()), t};
	log_insn(4) << "load " << dump_val(v) << " " << pos << " " << dump_val(r) << std::endl;
	return r;
}

LibJITCompiler::value LibJITCompiler::insn_typeof(LibJITCompiler::value v) const {
	if (v.t.raw_type == RawType::ANY) return new_integer(LSValue::NULLL);
	if (v.t.raw_type == RawType::BOOLEAN) return new_integer(LSValue::BOOLEAN);
	if (v.t.isNumber()) return new_integer(LSValue::NUMBER);
	if (v.t.raw_type == RawType::STRING) return new_integer(LSValue::STRING);
	if (v.t.raw_type == RawType::ARRAY) return new_integer(LSValue::ARRAY);
	if (v.t.raw_type == RawType::MAP) return new_integer(LSValue::MAP);
	if (v.t.raw_type == RawType::SET) return new_integer(LSValue::SET);
	if (v.t.raw_type == RawType::INTERVAL) return new_integer(LSValue::INTERVAL);
	if (v.t.raw_type == RawType::FUNCTION) return new_integer(LSValue::FUNCTION);
	if (v.t.raw_type == RawType::CLOSURE) return new_integer(LSValue::CLOSURE);
	if (v.t.raw_type == RawType::OBJECT) return new_integer(LSValue::OBJECT);
	if (v.t.raw_type == RawType::CLASS) return new_integer(LSValue::CLASS);
	return insn_call(Type::INTEGER, {v}, +[](LSValue* v) {
		return v->type;
	}, "typeof");
}

LibJITCompiler::value LibJITCompiler::insn_class_of(LibJITCompiler::value v) const {
	// if (v.t.raw_type == RawType::ANY)
		// return new_pointer(vm->system_vars["Any"]);
	if (v.t.raw_type == RawType::BOOLEAN)
		return new_pointer(vm->system_vars["Boolean"]);
	if (v.t.isNumber())
		return new_pointer(vm->system_vars["Number"]);
	if (v.t.raw_type == RawType::STRING)
		return new_pointer(vm->system_vars["String"]);
	if (v.t.raw_type == RawType::ARRAY)
		return new_pointer(vm->system_vars["Array"]);
	if (v.t.raw_type == RawType::MAP)
		return new_pointer(vm->system_vars["Map"]);
	if (v.t.raw_type == RawType::SET)
		return new_pointer(vm->system_vars["Set"]);
	if (v.t.raw_type == RawType::INTERVAL)
		return new_pointer(vm->system_vars["Interval"]);
	if (v.t.raw_type == RawType::FUNCTION)
		return new_pointer(vm->system_vars["Function"]);
	if (v.t.raw_type == RawType::OBJECT)
		return new_pointer(vm->system_vars["Object"]);
	if (v.t.raw_type == RawType::CLASS)
		return new_pointer(vm->system_vars["Class"]);
	return insn_call(Type::CLASS, {v}, +[](LSValue* v) {
		return v->getClass();
	}, "get_class");
}

void LibJITCompiler::insn_delete(LibJITCompiler::value v) const {
	if (v.t.must_manage_memory()) {
		// insn_call(Type::VOID, {v}, (void*) &LSValue::delete_ref);
		insn_if_not(insn_native(v), [&]() {
			auto refs = insn_refs(v);
			insn_if(refs, [&]() {
				insn_if_not(insn_dec_refs(v, refs), [&]() {
					insn_call(Type::VOID, {v}, (void*) &LSValue::free, "Value::free");
				});
			});
		});
	} else if (v.t.not_temporary() == Type::MPZ) {
		insn_delete_mpz(v);
	}
}

LibJITCompiler::value LibJITCompiler::insn_refs(LibJITCompiler::value v) const {
	assert(v.t.must_manage_memory());
	return insn_load(v, 12, Type::INTEGER);
}

LibJITCompiler::value LibJITCompiler::insn_native(LibJITCompiler::value v) const {
	assert(v.t.must_manage_memory());
	return insn_load(v, 16, Type::BOOLEAN);
}

void LibJITCompiler::insn_delete_temporary(LibJITCompiler::value v) const {
	if (v.t.must_manage_memory()) {
		// insn_call(Type::VOID, {v}, (void*) &LSValue::delete_temporary);
		insn_if_not(insn_refs(v), [&]() {
			insn_call(Type::VOID, {v}, (void*) &LSValue::free, "Value::free");
		});
	} else if (v.t == Type::MPZ_TMP) {
		insn_delete_mpz(v);
	}
}

LibJITCompiler::value LibJITCompiler::insn_array_size(LibJITCompiler::value v) const {
	/*
	if (v.t.raw_type == RawType::STRING) {
		return insn_call(Type::INTEGER, {v}, (void*) &LSString::int_size, "string_size");
	} else if (v.t.raw_type == RawType::ARRAY and v.t.getElementType() == Type::INTEGER) {
		return insn_call(Type::INTEGER, {v}, (void*) &LSArray<int>::int_size, "int_array_size");
	} else if (v.t.raw_type == RawType::ARRAY and v.t.getElementType() == Type::REAL) {
		return insn_call(Type::INTEGER, {v}, (void*) &LSArray<double>::int_size, "real_array_size");
	} else {
		return insn_call(Type::INTEGER, {v}, (void*) &LSArray<LSValue*>::int_size, "ptr_array_size");
	}
	*/
	return {};
}

LibJITCompiler::value LibJITCompiler::insn_get_capture(int index, Type type) const {
	LibJITCompiler::value fun = {jit_value_get_param(F, 0), Type::POINTER}; // function pointer
	auto jit_index = new_integer(index);
	auto v = insn_call(Type::POINTER, {fun, jit_index}, +[](LSClosure* fun, int index) {
		LSValue* v = fun->get_capture(index);
//		v->refs++;
		return v;
	});
	if (type.nature == Nature::VALUE) {
		v.v = VM::pointer_to_value(F, v.v, type);
	}
	return {v.v, type};
}

void LibJITCompiler::insn_push_array(LibJITCompiler::value array, LibJITCompiler::value value) const {
	if (array.t.getElementType() == Type::INTEGER) {
		insn_call(Type::VOID, {array, value}, (void*) +[](LSArray<int>* array, int value) {
			array->push_back(value);
		}, "array_push_int");
	} else if (array.t.getElementType() == Type::REAL) {
		value.t = Type::REAL;
		insn_call(Type::VOID, {array, value}, (void*) +[](LSArray<double>* array, double value) {
			array->push_back(value);
		}, "array_push_real");
	} else {
		insn_call(Type::VOID, {array, value}, (void*) +[](LSArray<LSValue*>* array, LSValue* value) {
			array->push_inc(value);
		}, "array_push_ptr");
	}
}

LibJITCompiler::value LibJITCompiler::insn_array_at(LibJITCompiler::value array, LibJITCompiler::value index) const {
	return insn_add(insn_load(array, 24, Type::POINTER), insn_mul(new_integer(array.t.getElementType().size() / 8), index));
}

LibJITCompiler::value LibJITCompiler::insn_move_inc(LibJITCompiler::value value) const {
	if (value.t.must_manage_memory()) {
		if (value.t.reference) {
			insn_inc_refs(value);
			return value;
		} else {
			return insn_call(value.t, {value}, (void*) +[](LSValue* v) {
				return v->move_inc();
			}, "move_inc");
		}
	}
	if (value.t.temporary) {
		return value;
	}
	if (value.t == Type::MPZ) {
		return insn_clone_mpz(value);
	} else {
		return value;
	}
}

LibJITCompiler::value LibJITCompiler::insn_clone_mpz(LibJITCompiler::value mpz) const {
	jit_value_t new_mpz = jit_value_create(F, VM::mpz_type);
	jit_value_set_addressable(new_mpz);
	LibJITCompiler::value r = {new_mpz, Type::MPZ_TMP};
	auto r_addr = insn_address_of(r);
	auto mpz_addr = insn_address_of(mpz);
	insn_call(Type::VOID, {r_addr, mpz_addr}, &mpz_init_set, "mpz_init_set");
	VM::inc_mpz_counter(F);
	return r;
}

void LibJITCompiler::insn_delete_mpz(LibJITCompiler::value mpz) const {
	auto mpz_addr = insn_address_of(mpz);
	insn_call(Type::VOID, {mpz_addr}, &mpz_clear, "mpz_clear");
	// Increment mpz values counter
	jit_value_t jit_counter_ptr = jit_value_create_long_constant(F, LS_POINTER, (long) &vm->mpz_deleted);
	jit_value_t jit_counter = jit_insn_load_relative(F, jit_counter_ptr, 0, jit_type_long);
	jit_insn_store_relative(F, jit_counter_ptr, 0, jit_insn_add(F, jit_counter, LS_CREATE_INTEGER(F, 1)));
}

LibJITCompiler::value LibJITCompiler::insn_inc_refs(value v) const {
	if (v.t.must_manage_memory()) {
		if (v.t.reference) {
			v = insn_load(v);
		}
		auto new_refs = insn_add(insn_refs(v), new_integer(1));
		insn_store_relative(v, 12, new_refs);
		return new_refs;
	}
	return new_integer(0);
}

LibJITCompiler::value LibJITCompiler::insn_dec_refs(value v, value previous) const {
	if (v.t.must_manage_memory()) {
		if (previous.v == nullptr) {
			previous = insn_refs(v);
		}
		auto new_refs = insn_sub(previous, new_integer(1));
		insn_store_relative(v, 12, new_refs);
		return new_refs;
	}
	return new_integer(0);
}

LibJITCompiler::value LibJITCompiler::insn_move(LibJITCompiler::value v) const {
	if (v.t.must_manage_memory() and !v.t.temporary and !v.t.reference) {
		return insn_call(v.t, {v}, (void*) +[](LSValue* v) {
			return v->move();
		}, "move");
	}
	return v;
}

LibJITCompiler::value LibJITCompiler::insn_call(Type return_type, std::vector<LibJITCompiler::value> args, void* func, std::string function_name) const {
	std::vector<jit_value_t> jit_args;
	std::vector<jit_type_t> arg_types;
	for (const auto& arg : args) {
		jit_args.push_back(arg.v);
		arg_types.push_back(arg.t.jit_type());
	}
	jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, return_type.jit_type(), arg_types.data(), arg_types.size(), 1);
	LibJITCompiler::value v = {jit_insn_call_native(F, "call", func, sig, jit_args.data(), arg_types.size(), 0), return_type};
	jit_type_free(sig);
	// Log
	log_insn(4) << "call ";
	if (function_name.size()) {
		log_insn(0) << function_name << " (";
	} else {
		log_insn(0) << std::hex << func << std::dec << " (";
	}
	for (unsigned i = 0; i < args.size(); ++i) {
		log_insn(0) << dump_val(args.at(i));
		if (i < args.size() - 1) log_insn(0) << ", ";
	}
	log_insn(0) << ")";
	if (return_type.nature != Nature::VOID) {
		log_insn(0) << " → " << dump_val(v);
	}
	log_insn(0) << std::endl;
	return v;
}

LibJITCompiler::value LibJITCompiler::insn_call_indirect(Type return_type, LibJITCompiler::value fun, std::vector<LibJITCompiler::value> args) const {
	std::vector<jit_type_t> arg_types;
	std::vector<jit_value_t> jit_args;
	for (const auto& arg : args) {
		jit_args.push_back(arg.v);
		arg_types.push_back(arg.t.jit_type());
	}
	jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, return_type.jit_type(), arg_types.data(), arg_types.size(), 1);
	LibJITCompiler::value v = {jit_insn_call_indirect(F, fun.v, sig, jit_args.data(), jit_args.size(), 0), return_type};
	jit_type_free(sig);
	// Log
	log_insn(4) << "call " << dump_val(fun) << " (";
	for (unsigned i = 0; i < args.size(); ++i) {
		log_insn(0) << dump_val(args.at(i));
		if (i < args.size() - 1) log_insn(0) << ", ";
	}
	log_insn(0) << ")";
	if (return_type.nature != Nature::VOID) {
		log_insn(0) << " → " << dump_val(v);
	}
	log_insn(0) << std::endl;
	return v;
}

void LibJITCompiler::function_add_capture(LibJITCompiler::value fun, LibJITCompiler::value capture) {
	insn_call(Type::VOID, {fun, capture}, +[](LSClosure* fun, LSValue* cap) {
		fun->add_capture(cap);
	});
}

// Debug-only function
// LCOV_EXCL_START
void LibJITCompiler::log(const std::string&& str) const {
	auto s = new_pointer(new std::string(str));
	insn_call(Type::VOID, {s}, +[](std::string* s) {
		std::cout << *s << std::endl;
		delete s;
	});
}
// LCOV_EXCL_STOP

/*
 * Iterators
 */
LibJITCompiler::value LibJITCompiler::iterator_begin(LibJITCompiler::value v) const {
	log_insn_code("iterator.begin()");
	if (v.t.raw_type == RawType::ARRAY) {
		LibJITCompiler::value it = {jit_value_create(F, v.t.jit_type()), v.t};
		insn_store(it, insn_load(v, 24));
		return it;
	}
	if (v.t.raw_type == RawType::INTERVAL) {
		jit_type_t types[2] = {jit_type_void_ptr, jit_type_int};
		auto interval_iterator = jit_type_create_struct(types, 2, 1);
		LibJITCompiler::value it = {jit_value_create(F, interval_iterator), Type::INTERVAL_ITERATOR};
		jit_type_free(interval_iterator);
		auto addr = insn_address_of(it);
		jit_insn_store_relative(F, addr.v, 0, v.v);
		jit_insn_store_relative(F, addr.v, 8, insn_load(v, 20, Type::INTEGER).v);
		return it;
	}
	if (v.t.raw_type == RawType::STRING) {
		jit_type_t types[5] = {jit_type_void_ptr, jit_type_int, jit_type_int, jit_type_int, jit_type_int};
		auto string_iterator = jit_type_create_struct(types, 5, 1);
		LibJITCompiler::value it = {jit_value_create(F, string_iterator), Type::STRING_ITERATOR};
		jit_type_free(string_iterator);
		auto addr = insn_address_of(it);
		insn_call(Type::VOID, {v, addr}, (void*) +[](LSString* str, LSString::iterator* it) {
			auto i = LSString::iterator_begin(str);
			it->buffer = i.buffer;
			it->index = 0;
			it->pos = 0;
			it->next_pos = 0;
			it->character = 0;
		});
		return it;
	}
	if (v.t.raw_type == RawType::MAP) {
		return insn_load(v, 48, v.t);
	}
	if (v.t.raw_type == RawType::SET) {
		jit_type_t types[2] = {jit_type_void_ptr, jit_type_int};
		auto set_iterator = jit_type_create_struct(types, 2, 1);
		LibJITCompiler::value it = {jit_value_create(F, set_iterator), Type::SET_ITERATOR};
		jit_type_free(set_iterator);
		auto addr = insn_address_of(it);
		jit_insn_store_relative(F, addr.v, 0, insn_load(v, 48, v.t).v);
		jit_insn_store_relative(F, addr.v, 8, new_integer(0).v);
		return it;
	}
	if (v.t.raw_type == RawType::INTEGER) {
		jit_type_t types[3] = {jit_type_int, jit_type_int, jit_type_int};
		auto integer_iterator = jit_type_create_struct(types, 3, 1);
		LibJITCompiler::value it = {jit_value_create(F, integer_iterator), Type::INTEGER_ITERATOR};
		jit_type_free(integer_iterator);
		auto addr = jit_insn_address_of(F, it.v);
		jit_insn_store_relative(F, addr, 0, v.v);
		jit_insn_store_relative(F, addr, 4, to_int(insn_pow(new_integer(10), to_int(insn_log10(v)))).v);
		jit_insn_store_relative(F, addr, 8, new_integer(0).v);
		return it;
	}
	if (v.t.raw_type == RawType::LONG) {
		jit_type_t types[3] = {jit_type_long, jit_type_long, jit_type_int};
		auto long_iterator = jit_type_create_struct(types, 3, 1);
		LibJITCompiler::value it = {jit_value_create(F, long_iterator), Type::LONG_ITERATOR};
		jit_type_free(long_iterator);
		auto addr = jit_insn_address_of(F, it.v);
		jit_insn_store_relative(F, addr, 0, v.v);
		jit_insn_store_relative(F, addr, 8, to_long(insn_pow(new_integer(10), to_int(insn_log10(v)))).v);
		jit_insn_store_relative(F, addr, 16, new_long(0).v);
		return it;
	}
	if (v.t.raw_type == RawType::MPZ) {
		jit_type_t types[3] = {VM::mpz_type, VM::mpz_type, jit_type_int};
		auto mpz_iterator = jit_type_create_struct(types, 3, 1);
		LibJITCompiler::value it = {jit_value_create(F, mpz_iterator), Type::MPZ_ITERATOR};
		jit_type_free(mpz_iterator);
		auto addr = jit_insn_address_of(F, it.v);
		jit_insn_store_relative(F, addr, 0, v.v);
		jit_insn_store_relative(F, addr, 16, to_long(insn_pow(new_integer(10), to_int(insn_log10(v)))).v);
		jit_insn_store_relative(F, addr, 32, new_long(0).v);
		return it;
	}
	return {nullptr, Type::VOID};
}

LibJITCompiler::value LibJITCompiler::iterator_end(LibJITCompiler::value v, LibJITCompiler::value it) const {
	log_insn_code("iterator.end()");
	if (v.t.raw_type == RawType::ARRAY) {
		return insn_eq(it, insn_load(v, 32));
	}
	if (it.t == Type::INTERVAL_ITERATOR) {
		auto addr = insn_address_of(it);
		auto interval = insn_load(addr, 0, Type::POINTER);
		auto end = insn_load(interval, 24, Type::INTEGER);
		auto pos = insn_load(addr, 8, Type::INTEGER);
		return insn_gt(pos, end);
	}
	if (it.t == Type::STRING_ITERATOR) {
		auto addr = insn_address_of(it);
		return insn_call(Type::BOOLEAN, {addr}, &LSString::iterator_end);
	}
	if (v.t.raw_type == RawType::MAP) {
		auto end = insn_add(v, new_integer(32)); // end_ptr = &map + 24
		return insn_eq(it, end);
	}
	if (it.t == Type::SET_ITERATOR) {
		auto addr = insn_address_of(it);
		auto ptr = insn_load(addr, 0, Type::POINTER);
		auto end = insn_add(v, new_integer(32)); // end_ptr = &set + 24
		return insn_eq(ptr, end);
	}
	if (it.t == Type::INTEGER_ITERATOR) {
		auto addr = insn_address_of(it);
		auto p = insn_load(addr, 4, Type::INTEGER);
		return insn_eq(p, new_integer(0));
	}
	if (it.t == Type::LONG_ITERATOR) {
		auto addr = insn_address_of(it);
		auto p = insn_load(addr, 8, Type::LONG);
		return insn_eq(p, new_integer(0));
	}
	return {nullptr, Type::VOID};
}

LibJITCompiler::value LibJITCompiler::iterator_key(LibJITCompiler::value v, LibJITCompiler::value it, LibJITCompiler::value previous) const {
	log_insn_code("iterator.key()");
	if (it.t.raw_type == RawType::ARRAY) {
		return insn_int_div(insn_sub(it, insn_load(v, 24)), new_integer(it.t.element().size() / 8));
	}
	if (it.t == Type::INTERVAL_ITERATOR) {
		auto addr = insn_address_of(it);
		auto interval = insn_load(addr, 0);
		auto start = insn_load(interval, 20);
		auto e = insn_load(addr, 8, Type::INTEGER);
		return insn_sub(e, start);
	}
	if (it.t == Type::STRING_ITERATOR) {
		auto addr = insn_address_of(it);
		return insn_call(Type::INTEGER, {addr}, &LSString::iterator_key);
	}
	if (it.t.raw_type == RawType::MAP) {
		if (previous.t.must_manage_memory()) {
			insn_call(Type::VOID, {previous}, +[](LSValue* previous) {
				if (previous != nullptr)
					LSValue::delete_ref(previous);
			});
		}
		auto key = insn_load(it, 32, it.t.getKeyType());
		insn_inc_refs(key);
		return key;
	}
	if (it.t == Type::SET_ITERATOR) {
		auto addr = insn_address_of(it);
		return insn_load(addr, 8, Type::INTEGER);
	}
	if (it.t == Type::INTEGER_ITERATOR) {
		auto addr = insn_address_of(it);
		return insn_load(addr, 8, Type::INTEGER);
	}
	if (it.t == Type::LONG_ITERATOR) {
		auto addr = insn_address_of(it);
		return insn_load(addr, 16, Type::INTEGER);
	}
	return {nullptr, Type::VOID};
}

LibJITCompiler::value LibJITCompiler::iterator_get(LibJITCompiler::value it, LibJITCompiler::value previous) const {
	log_insn_code("iterator.get()");
	if (it.t.raw_type == RawType::ARRAY) {
		if (previous.t.must_manage_memory()) {
			insn_call(Type::VOID, {previous}, +[](LSValue* previous) {
				if (previous != nullptr)
					LSValue::delete_ref(previous);
			});
		}
		auto e = insn_load(it, 0, it.t.getElementType());
		insn_inc_refs(e);
		return e;
	}
	if (it.t == Type::INTERVAL_ITERATOR) {
		auto addr = insn_address_of(it);
		auto e = insn_load(addr, 8, Type::INTEGER);
		return e;
	}
	if (it.t == Type::STRING_ITERATOR) {
		auto addr = insn_address_of(it);
		auto int_char = insn_call(Type::INTEGER, {addr}, &LSString::iterator_get);
		return insn_call(Type::STRING, {int_char, previous}, (void*) +[](unsigned int c, LSString* previous) {
			if (previous != nullptr) {
				LSValue::delete_ref(previous);
			}
			char dest[5];
			u8_toutf8(dest, 5, &c, 1);
			auto s = new LSString(dest);
			s->refs = 1;
			return s;
		});
	}
	if (it.t.raw_type == RawType::MAP) {
		if (previous.t.must_manage_memory()) {
			insn_call(Type::VOID, {previous}, +[](LSValue* previous) {
				if (previous != nullptr)
					LSValue::delete_ref(previous);
			});
		}
		auto e = insn_load(it, 32 + 8, it.t.element());
		insn_inc_refs(e);
		return e;
	}
	if (it.t == Type::SET_ITERATOR) {
		if (previous.t.must_manage_memory()) {
			insn_call(Type::VOID, {previous}, +[](LSValue* previous) {
				if (previous != nullptr)
					LSValue::delete_ref(previous);
			});
		}
		auto addr = insn_address_of(it);
		auto ptr = insn_load(addr, 0, Type::POINTER);
		auto e = insn_load(ptr, 32, previous.t);
		insn_inc_refs(e);
		return e;
	}
	if (it.t == Type::INTEGER_ITERATOR) {
		auto addr = insn_address_of(it);
		auto n = insn_load(addr, 0, Type::INTEGER);
		auto p = insn_load(addr, 4, Type::INTEGER);
		return insn_int_div(n, p);
	}
	if (it.t == Type::LONG_ITERATOR) {
		auto addr = insn_address_of(it);
		auto n = insn_load(addr, 0, Type::LONG);
		auto p = insn_load(addr, 8, Type::LONG);
		return insn_int_div(n, p);
	}
	return {nullptr, Type::VOID};
}

void LibJITCompiler::iterator_increment(LibJITCompiler::value it) const {
	log_insn_code("iterator.increment()");
	if (it.t.raw_type == RawType::ARRAY) {
		insn_store(it, insn_add(it, new_integer(it.t.element().size() / 8)));
		//insn_store(it, insn_add(it, insn_mul(new_integer(16), new_integer(it.t.element().size() / 8)) ));
		return;
	}
	if (it.t == Type::INTERVAL_ITERATOR) {
		auto addr = insn_address_of(it);
		auto pos = insn_load(addr, 8, Type::INTEGER);
		insn_store_relative(addr, 8, insn_add(pos, new_integer(1)));
		return;
	}
	if (it.t == Type::STRING_ITERATOR) {
		auto addr = insn_address_of(it);
		insn_call(Type::VOID, {addr}, &LSString::iterator_next);
		return;
	}
	if (it.t.raw_type == RawType::MAP) {
		insn_store(it, insn_call(Type::POINTER, {it}, (void*) +[](LSMap<int, int>::iterator it) {
			it++;
			return it;
		}));
		return;
	}
	if (it.t == Type::SET_ITERATOR) {
		auto addr = insn_address_of(it);
		auto ptr = insn_load(addr, 0, Type::POINTER);
		insn_store_relative(addr, 8, insn_add(insn_load(addr, 8, Type::INTEGER), new_integer(1)));
		insn_store_relative(addr, 0, insn_call(Type::POINTER, {ptr}, (void*) +[](LSSet<int>::iterator it) {
			it++;
			return it;
		}));
		return;
	}
	if (it.t == Type::INTEGER_ITERATOR) {
		auto addr = insn_address_of(it);
		auto n = insn_load(addr, 0, Type::INTEGER);
		auto p = insn_load(addr, 4, Type::INTEGER);
		auto i = insn_load(addr, 8, Type::INTEGER);
		jit_insn_store_relative(F, addr.v, 0, insn_mod(n, p).v);
		jit_insn_store_relative(F, addr.v, 4, insn_int_div(p, new_integer(10)).v);
		jit_insn_store_relative(F, addr.v, 8, insn_add(i, new_integer(1)).v);
		return;
	}
	if (it.t == Type::LONG_ITERATOR) {
		auto addr = insn_address_of(it);
		auto n = insn_load(addr, 0, Type::LONG);
		auto p = insn_load(addr, 8, Type::LONG);
		auto i = insn_load(addr, 16, Type::INTEGER);
		jit_insn_store_relative(F, addr.v, 0, insn_mod(n, p).v);
		jit_insn_store_relative(F, addr.v, 8, insn_int_div(p, new_integer(10)).v);
		jit_insn_store_relative(F, addr.v, 16, insn_add(i, new_integer(1)).v);
		return;
	}
}

/*
 * Controls
 */
void LibJITCompiler::insn_if(LibJITCompiler::value condition, std::function<void()> then) const {
	label label_end;
	insn_branch_if_not(condition, &label_end);
	then();
	insn_label(&label_end);
}

void LibJITCompiler::insn_if_not(LibJITCompiler::value condition, std::function<void()> then) const {
	label label_end;
	insn_branch_if(condition, &label_end);
	then();
	insn_label(&label_end);
}

void LibJITCompiler::insn_throw(LibJITCompiler::value v) const {
	jit_insn_throw(F, v.v);
	log_insn(4) << "throw " << dump_val(v) << std::endl;
}

void LibJITCompiler::insn_throw_object(vm::Exception type) const {
	// auto t = new_integer(type);
	// auto ex = insn_call(Type::POINTER, {t}, &VM::get_exception_object<0>);
	// insn_throw(ex);
}

void LibJITCompiler::insn_label(label* l) const {
	jit_insn_label(F, &l->l);
	register_label(l);
	log_insn(1) << C_GREY << "label " << label_map.at(l) << ":" << END_COLOR << std::endl;
}

void LibJITCompiler::insn_branch(label* l) const {
	jit_insn_branch(F, &l->l);
	register_label(l);
	log_insn(4) << "goto " << label_map.at(l) << std::endl;
}
void LibJITCompiler::insn_branch_if(LibJITCompiler::value v, LibJITCompiler::label* l) const {
	jit_insn_branch_if(F, v.v, &l->l);
	register_label(l);
	log_insn(4) << "goto " << label_map.at(l) << " if " << dump_val(v) << std::endl;
}
void LibJITCompiler::insn_branch_if_not(LibJITCompiler::value v, LibJITCompiler::label* l) const {
	jit_insn_branch_if_not(F, v.v, &l->l);
	register_label(l);
	log_insn(4) << "goto " << label_map.at(l) << " if not " << dump_val(v) << std::endl;
}
void LibJITCompiler::insn_branch_if_pc_not_in_range(label* a, label* b, label* n) const {
	jit_insn_branch_if_pc_not_in_range(F, a->l, b->l, &n->l);
}

void LibJITCompiler::insn_return(LibJITCompiler::value v) const {
	log_insn(4) << "return " << dump_val(v) << std::endl;
	jit_insn_return(F, v.v);
}

/*
 * Variables
 */
void LibJITCompiler::add_var(const std::string& name, LibJITCompiler::value value) {
	assert((value.v != nullptr) && "value must not be null");
	variables.back()[name] = value;
	var_map.insert({value.v, name});
}

void LibJITCompiler::add_function_var(LibJITCompiler::value value) {
	function_variables.back().push_back(value);
}

LibJITCompiler::value& LibJITCompiler::get_var(const std::string& name) {
	for (int i = variables.size() - 1; i >= 0; --i) {
		auto it = variables[i].find(name);
		if (it != variables[i].end()) {
			return it->second;
		}
	}
	assert(false && "var not found !");
	return *((LibJITCompiler::value*) nullptr); // Should not reach this line
}

void LibJITCompiler::set_var_type(std::string& name, const Type& type) {
	for (int i = variables.size() - 1; i >= 0; --i) {
		auto it = variables[i].find(name);
		if (it != variables[i].end()) {
			variables[i][name].t = type;
			return;
		}
	}
}

void LibJITCompiler::update_var(std::string& name, LibJITCompiler::value value) {
	assert((value.v != nullptr) && "new value must not be null");
	variables.back()[name] = value;
	var_map.insert({value.v, name});
}

void LibJITCompiler::enter_loop(LibJITCompiler::label* end_label, LibJITCompiler::label* cond_label) {
	loops_end_labels.push_back(end_label);
	loops_cond_labels.push_back(cond_label);
	loops_blocks.push_back(0);
}

void LibJITCompiler::leave_loop() {
	loops_end_labels.pop_back();
	loops_cond_labels.pop_back();
	loops_blocks.pop_back();
}

LibJITCompiler::label* LibJITCompiler::get_current_loop_end_label(int deepness) const {
	return loops_end_labels[loops_end_labels.size() - deepness];
}

LibJITCompiler::label* LibJITCompiler::get_current_loop_cond_label(int deepness) const {
	return loops_cond_labels[loops_cond_labels.size() - deepness];
}

int LibJITCompiler::get_current_loop_blocks(int deepness) const {
	int sum = 0;
	for (size_t i = loops_blocks.size() - deepness; i < loops_blocks.size(); ++i) {
		sum += loops_blocks[i];
	}
	return sum;
}

void LibJITCompiler::inc_ops(int amount) const {
	inc_ops_jit(new_integer(amount));
}

void LibJITCompiler::inc_ops_jit(LibJITCompiler::value amount) const {
	// Operations enabled?
	if (not vm->enable_operations) return;

	// Variable counter pointer
	jit_value_t jit_ops_ptr = jit_value_create_long_constant(F, LS_POINTER, (long int) &vm->operations);

	// Increment counter
	jit_value_t jit_ops = jit_insn_load_relative(F, jit_ops_ptr, 0, jit_type_uint);
	jit_insn_store_relative(F, jit_ops_ptr, 0, jit_insn_add(F, jit_ops, amount.v));

	// Compare to the limit
	jit_value_t compare = jit_insn_gt(F, jit_ops, jit_value_create_nint_constant(F, jit_type_uint, vm->operation_limit));
	label label_end;
	insn_branch_if_not({compare, Type::BOOLEAN}, &label_end);

	// If greater than the limit, throw exception
	insn_throw_object(vm::Exception::OPERATION_LIMIT_EXCEEDED);

	// End
	insn_label(&label_end);
}

void LibJITCompiler::mark_offset(int line) {
	jit_insn_mark_offset(F, line);
}

void LibJITCompiler::add_catcher(label start, label end, label handler) {
	catchers.back().push_back({start, end, handler, {}});
}

void LibJITCompiler::insn_check_args(std::vector<LibJITCompiler::value> args, std::vector<LSValueType> types) const {
	// TODO too much cheks sometimes
	for (size_t i = 0; i < args.size(); ++i) {
		auto arg = args[i];
		auto type = types[i];
		if (arg.t.nature != Nature::VALUE and type != arg.t.id() and type != 0) {
			auto type = types[i];
			insn_if(insn_ne(insn_typeof(arg), new_integer(type)), [&]() {
				for (auto& a : args) {
					insn_delete_temporary(a);
				}
				insn_throw_object(vm::Exception::WRONG_ARGUMENT_TYPE);
			});
		}
	}
}

std::ostringstream& LibJITCompiler::_log_insn(int indent) const {
	for (int i = 0; i < indent; ++i) {
		((LibJITCompiler*) this)->instructions_debug << " ";
	}
	return ((LibJITCompiler*) this)->instructions_debug;
}

std::string LibJITCompiler::dump_val(LibJITCompiler::value v) const {
	if (v.v == nullptr) {
		return "<null>";
	}
	if (var_map.find(v.v) != var_map.end()) {
		return var_map.at(v.v);
	}
	char buf[256];
	auto fp = fmemopen(buf, sizeof(buf), "w");
	jit_dump_value(fp, F, v.v, nullptr);
	fclose(fp);
	auto r = std::string(buf);
	// r += std::string(" ") + v.t.to_string();
	if (jit_value_is_parameter(v.v)) {
		for (unsigned i = 0; i < arg_names.top().size(); ++i) {
			if (v.v == jit_value_get_param(F, i)) {
				return arg_names.top().at(i);
			}
		}
	}
	if (jit_value_is_constant(v.v) && v.t.nature == Nature::POINTER) {
		long x = std::stol(r);
		// known literal?
		if (literals.find((void*) x) != literals.end()) {
			return std::string(BOLD) + literals.at((void*) x) + std::string(END_STYLE);
		}
		std::stringstream ss;
		ss << "0x" << std::hex << x;
		r = ss.str();
	}
	return r;
}

void LibJITCompiler::register_label(label* l) const {
	if (label_map.find(l) == label_map.end()) {
		((LibJITCompiler*) this)->label_map.insert({l, std::string(1, 'A' + (char) label_map.size())});
	}
}

void LibJITCompiler::log_insn_code(std::string instruction) const {
	log_insn(0) << C_BLUE << instruction << END_COLOR << std::endl;
}

void LibJITCompiler::add_literal(void* ptr, std::string value) const {
	((LibJITCompiler*) this)->literals.insert({ptr, value});
}

}