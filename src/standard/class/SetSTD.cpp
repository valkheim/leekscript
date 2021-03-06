#include "SetSTD.hpp"
#include "../../type/Type.hpp"
#include "../../environment/Environment.hpp"
#if COMPILER
#include "../../vm/value/LSSet.hpp"
#endif

namespace ls {

#if COMPILER
const std::_Rb_tree_node_base* iterator_end(LSSet<int>* set) {
	return set->end()._M_node;
}
LSSet<int>::iterator iterator_inc(LSSet<int>::iterator it) {
	it++;
	return it;
}
#endif

SetSTD::SetSTD(Environment& env) : Module(env, "Set") {

	#if COMPILER
	env.set_class = std::make_unique<LSClass>(clazz.get());
	lsclass = env.set_class.get();
	#endif

	/*
	 * Constructor
	 */
	constructor_({
		{Type::tmp_set(env.any), {}, ADDR((void*) &LSSet<LSValue*>::constructor), PRIVATE},
		{Type::tmp_set(env.real), {}, ADDR((void*) &LSSet<double>::constructor), PRIVATE},
		{Type::tmp_set(env.integer), {}, ADDR((void*) &LSSet<int>::constructor), PRIVATE},
		{Type::tmp_set(env.any), {}, ADDR(new_)},
	});

	/*
	 * Operators
	 */
	operator_("in", {
		{Type::const_set(env.void_), env.any, env.boolean, ADDR((void*) LSSet<LSValue*>::std_in_v)},
		{Type::const_set(env.void_), env.integer, env.boolean, ADDR(in_any)},
		{Type::const_set(env.real), env.real, env.boolean, ADDR((void*) LSSet<double>::std_in_v)},
		{Type::const_set(env.integer), env.integer, env.boolean, ADDR((void*) LSSet<int>::std_in_v)}
	});

	auto pqT = env.template_("T");
	auto pqE = env.template_("E");
	template_(pqT, pqE).
	operator_("+=", {
		{Type::set(pqT), pqE, Type::set(Type::meta_mul(pqT, pqE)), ADDR(set_add_eq), 0, { env.convert_mutator }, true},
	});

	/*
	 * Methods
	 */
	method("size", {
		{env.integer, {Type::const_set(env.void_)}, ADDR((void*) LSSet<LSValue*>::std_size)},
		{env.integer, {Type::const_set(env.real)}, ADDR((void*) LSSet<double>::std_size)},
		{env.integer, {Type::const_set(env.integer)}, ADDR((void*) LSSet<int>::std_size)},
	});
	method("insert", {
		{env.boolean, {Type::set(env.any), env.any}, ADDR((void*) LSSet<LSValue*>::std_insert_ptr)},
		{env.boolean, {Type::set(env.any), env.any}, ADDR(insert_any)},
		{env.boolean, {Type::set(env.real), env.real}, ADDR(insert_real)},
		{env.boolean, {Type::set(env.integer), env.integer}, ADDR(insert_int)},
	});
	method("clear", {
		{Type::set(env.void_), {Type::set(env.any)}, ADDR((void*) LSSet<LSValue*>::std_clear)},
		{Type::set(env.real), {Type::set(env.real)}, ADDR((void*) LSSet<double>::std_clear)},
		{Type::set(env.integer), {Type::set(env.integer)}, ADDR((void*) LSSet<int>::std_clear)},
	});
	method("erase", {
		{env.boolean, {Type::set(env.void_), env.any}, ADDR((void*) LSSet<LSValue*>::std_erase)},
		{env.boolean, {Type::set(env.real), env.real}, ADDR((void*) LSSet<double>::std_erase)},
		{env.boolean, {Type::set(env.integer), env.integer}, ADDR((void*) LSSet<int>::std_erase)},
	});
	method("contains", {
		{env.boolean, {Type::const_set(env.void_), env.any}, ADDR((void*) LSSet<LSValue*>::std_contains)},
		{env.boolean, {Type::const_set(env.real), env.real}, ADDR((void*) LSSet<double>::std_contains)},
		{env.boolean, {Type::const_set(env.integer), env.integer}, ADDR((void*) LSSet<int>::std_contains)},
	});

	/** Internal **/
	method("vinsert", {
		{env.void_, {Type::const_set(env.any), env.any}, ADDR((void*) LSSet<LSValue*>::vinsert)},
		{env.void_, {Type::const_set(env.real), env.real}, ADDR((void*) LSSet<double>::vinsert)},
		{env.void_, {Type::const_set(env.integer), env.integer}, ADDR((void*) LSSet<int>::vinsert)},
	}, PRIVATE);
	method("iterator_end", {
		{Type::set(env.void_)->iterator(), {Type::set(env.void_)}, ADDR((void*) iterator_end)}
	}, PRIVATE);
	method("iterator_inc", {
		{Type::set(env.void_)->iterator(), {Type::set(env.void_)->iterator()}, ADDR((void*) iterator_inc)}
	}, PRIVATE);
	method("insert_fun", {
		{env.boolean, {Type::const_set(env.any), env.any}, ADDR((void*) LSSet<LSValue*>::std_insert)},
		{env.boolean, {Type::const_set(env.real), env.real}, ADDR((void*) LSSet<double>::std_insert)},
		{env.boolean, {Type::const_set(env.integer), env.integer}, ADDR((void*) LSSet<int>::std_insert)},
	}, PRIVATE);
	method("int_to_any", {
		{Type::set(env.any), {Type::set(env.integer)}, ADDR((void*) LSSet<int>::to_any_set)}
	}, PRIVATE);
	method("real_to_any", {
		{Type::set(env.any), {Type::set(env.real)}, ADDR((void*) LSSet<double>::to_any_set)}
	}, PRIVATE);
}

#if COMPILER

Compiler::value SetSTD::new_(Compiler& c, std::vector<Compiler::value> args, int) {
	return c.new_set();
}

Compiler::value SetSTD::in_any(Compiler& c, std::vector<Compiler::value> args, int) {
	return c.insn_call(c.env.any, {args[0], c.insn_to_any(args[1])}, "Value.operatorin");
}

Compiler::value SetSTD::set_add_eq(Compiler& c, std::vector<Compiler::value> args, int) {
	return c.insn_call(c.env.any, {args[0], c.insn_to_any(args[1])}, "Value.operator+=");
}

Compiler::value SetSTD::insert_any(Compiler& c, std::vector<Compiler::value> args, int) {
	return c.insn_call(c.env.boolean, {args[0], c.insn_to_any(args[1])}, "Set.insert_fun");
}
Compiler::value SetSTD::insert_real(Compiler& c, std::vector<Compiler::value> args, int) {
	return c.insn_call(c.env.boolean, {args[0], c.to_real(args[1])}, "Set.insert_fun.1");
}
Compiler::value SetSTD::insert_int(Compiler& c, std::vector<Compiler::value> args, int) {
	return c.insn_call(c.env.boolean, {args[0], c.to_int(args[1])}, "Set.insert_fun.2");
}

#endif

}
