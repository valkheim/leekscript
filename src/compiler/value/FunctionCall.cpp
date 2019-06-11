#include "FunctionCall.hpp"
#include <sstream>
#include <string>
#include "../../vm/Module.hpp"
#include "../../vm/Program.hpp"
#include "../../type/Type.hpp"
#include "../../vm/value/LSArray.hpp"
#include "../../vm/value/LSClass.hpp"
#include "../../vm/value/LSNumber.hpp"
#include "../../vm/value/LSObject.hpp"
#include "../../vm/value/LSString.hpp"
#include "../Compiler.hpp"
#include "../lexical/Token.hpp"
#include "../semantic/SemanticAnalyzer.hpp"
#include "../error/Error.hpp"
#include "ObjectAccess.hpp"
#include "VariableValue.hpp"
#include "../semantic/Callable.hpp"
#include "../semantic/FunctionVersion.hpp"
#include "../../type/Function_type.hpp"

namespace ls {

FunctionCall::FunctionCall(std::shared_ptr<Token> t) : token(t) {
	function = nullptr;
	std_func = nullptr;
	this_ptr = nullptr;
	constant = false;
}

FunctionCall::~FunctionCall() {
	delete function;
	for (const auto& arg : arguments) {
		delete arg;
	}
}

void FunctionCall::print(std::ostream& os, int indent, bool debug, bool condensed) const {

	auto parenthesis = condensed && dynamic_cast<const Function*>(function);
	if (parenthesis) os << "(";
	function->print(os, indent, debug, condensed);
	if (parenthesis) os << ")";

	os << "(";
	for (unsigned i = 0; i < arguments.size(); ++i) {
		arguments.at(i)->print(os, indent, debug, condensed);
		if (i < arguments.size() - 1) {
			os << ", ";
		}
	}
	os << ")";
	if (debug) {
		os << " " << type;
	}
}

Location FunctionCall::location() const {
	return {closing_parenthesis->location.file, function->location().start, closing_parenthesis->location.end};
}

Call FunctionCall::get_callable(SemanticAnalyzer*, int argument_count) const {
	std::vector<const Type*> arguments_types;
	for (const auto& argument : arguments) {
		arguments_types.push_back(argument->type);
	}
	auto type = function->version_type(arguments_types);
	return { new CallableVersion { "<fc>", type->return_type(), this, {}, {} } };
}

void FunctionCall::analyze(SemanticAnalyzer* analyzer) {

	// std::cout << "FC analyse " << std::endl;

	// Analyse the function (can be anything here)
	function->analyze(analyzer);
	throws = function->throws;

	// Analyse arguments
	for (const auto& argument : arguments) {
		argument->analyze(analyzer);
		throws |= argument->throws;
	}

	// Perform a will_take to prepare eventual versions
	std::vector<const Type*> arguments_types;
	for (const auto& argument : arguments) {
		arguments_types.push_back(argument->type);
	}
	function->will_take(analyzer, arguments_types, 1);
	function->set_version(arguments_types, 1);

	// std::cout << "FC function " << function->version_type(arguments_types) << std::endl;

	if (not function->type->can_be_callable()) {
		analyzer->add_error({Error::Type::CANNOT_CALL_VALUE, location(), function->location(), {function->to_string()}});
	}

	// Retrieve the callable version
	call = function->get_callable(analyzer, arguments_types.size());
	if (call.callable) {
		// std::cout << "Callable: " << (void*) call.callable << std::endl;
		callable_version = call.resolve(analyzer, arguments_types);
		if (callable_version) {
			// std::cout << "Version: " << callable_version << std::endl;
			type = callable_version->type->return_type();
			throws |= callable_version->flags & Module::THROWS;
			call.apply_mutators(analyzer, callable_version, arguments);
			
			int offset = call.object ? 1 : 0;
			for (size_t a = 0; a < arguments.size(); ++a) {
				auto argument_type = callable_version->type->argument(a + offset);
				if (argument_type->is_function()) {
					arguments.at(a)->will_take(analyzer, argument_type->arguments(), 1);
					arguments.at(a)->set_version(argument_type->arguments(), 1);
				}
			}
			if (callable_version->value) {
				function->will_take(analyzer, arguments_types, 1);
				function->set_version(arguments_types, 1);
				function_type = function->version_type(arguments_types);
				type = function_type->return_type();
				auto vv = dynamic_cast<VariableValue*>(function);
				if (vv and vv->var) {
					if (callable_version->user_fun == analyzer->current_function()->current_version) {
						analyzer->current_function()->current_version->recursive = true;
						type = analyzer->current_function()->getReturnType();
					}
				}
			}
			if (callable_version->unknown) {
				for (const auto& arg : arguments) {
					if (arg->type->is_function()) {
						arg->must_return_any(analyzer);
					}
				}
			}
			if (type->is_mpz()) {
				type = type == Type::tmp_mpz ? Type::tmp_mpz_ptr : Type::mpz_ptr;
			}
			return;
		}
	}
	// Find the function object
	function_object = dynamic_cast<Function*>(function);
	if (!function_object) {
		auto vv = dynamic_cast<VariableValue*>(function);
		if (vv && vv->var && vv->var->value) {
			if (auto f = dynamic_cast<Function*>(vv->var->value)) {
				function_object = f;
			}
		}
	}

	// Detect standard library functions
	auto oa = dynamic_cast<ObjectAccess*>(function);
	if (oa != nullptr) {
		auto arguments_count = arguments_types.size() + (call.object ? 1 : 0);
		if (not call.callable or (not callable_version and call.callable->versions[0]->type->arguments().size() == arguments_count)) {
			auto field_name = oa->field->content;
			auto object_type = oa->object->type;
			std::vector<const Type*> arg_types;
			for (auto arg : arguments) {
				arg_types.push_back(arg->type->fold());
			}
			std::ostringstream args_string;
			for (unsigned i = 0; i < arg_types.size(); ++i) {
				if (i > 0) args_string << ", ";
				args_string << arg_types[i];
			}
			if (object_type->is_class()) { // String.size("salut")
				std::string clazz = ((VariableValue*) oa->object)->name;
				analyzer->add_error({Error::Type::STATIC_METHOD_NOT_FOUND, location(), oa->field->location, {clazz + "::" + oa->field->content + "(" + args_string.str() + ")"}});
				return;
			} else {  // "salut".size()
				bool has_unknown_argument = false;
				if (!object_type->fold()->is_any() && !has_unknown_argument) {
					std::ostringstream obj_type_ss;
					obj_type_ss << object_type;
					analyzer->add_error({Error::Type::METHOD_NOT_FOUND, location(), oa->field->location, {obj_type_ss.str() + "." + oa->field->content + "(" + args_string.str() + ")"}});
					return;
				} else {
					is_unknown_method = true;
					object = oa->object;
				}
			}
		}
	} else if (call.callable and call.callable->versions.size() and not call.callable->versions[0]->user_fun) {
		std::ostringstream args_string;
		for (unsigned i = 0; i < arguments_types.size(); ++i) {
			if (i > 0) args_string << ", ";
			args_string << arguments_types[i];
		}
		analyzer->add_error({Error::Type::METHOD_NOT_FOUND, location(), function->location(), {function->to_string() + "(" + args_string.str() + ")"}});
		return;
	}

	// Check arguments count
	arg_types.clear();
	auto arguments_count = arguments.size();
	if (this_ptr != nullptr) arguments_count++;
	bool arguments_valid = arguments_count <= function->type->arguments().size();
	auto total_arguments_passed = std::max(arguments.size(), function->type->arguments().size());
	int offset = this_ptr != nullptr ? 1 : 0;
	size_t a = 0;
	for (auto& argument_type : function->type->arguments()) {
		if (a == 0 and this_ptr != nullptr) {
			// OK it's the object for method call
		} else if (a < arguments_count) {
			// OK, the argument is present in the call
			arg_types.push_back(arguments.at(a - offset)->type);
		} else if (function_object && function_object->defaultValues.at(a - offset) != nullptr) {
			// OK, there's no argument in the call but a default value is set.
			arg_types.push_back(function_object->defaultValues.at(a - offset)->type);
		} else {
			// Missing argument
			arguments_valid = false;
			total_arguments_passed--;
		}
		a++;
	}
	if (function->type->is_function() and !arguments_valid) {
		analyzer->add_error({Error::Type::WRONG_ARGUMENT_COUNT,	location(), location(), {
			function->to_string(),
			std::to_string(function->type->arguments().size()),
			std::to_string(total_arguments_passed)
		}});
		return;
	}
}

bool FunctionCall::will_take(SemanticAnalyzer* analyzer, const std::vector<const Type*>& args, int level) {
	// std::cout << "FC " << this << " will_take " << args << std::endl;
	function->will_take(analyzer, args, level + 1);
	return false;
}

void FunctionCall::set_version(const std::vector<const Type*>& args, int level) {
	function->set_version(args, level + 1);
}

const Type* FunctionCall::version_type(std::vector<const Type*> version) const {
	// std::cout << "FunctionCall " << this << " ::version_type(" << version << ") " << std::endl;
	auto function_type = function->version_type(function->version);
	auto ft = dynamic_cast<const Function_type*>(function_type->return_type());
	assert(ft != nullptr);
	return ft->function()->version_type(version);
}

Compiler::value FunctionCall::compile(Compiler& c) const {
	
	c.mark_offset(location().start.line);

	// std::cout << "FunctionCall::compile(" << function_type << ")" << std::endl;
	assert(callable_version);

	std::vector<LSValueType> types;
	std::vector<Compiler::value> args;
	// Pre-compile the call (compile the potential object first)
	if (call.object) {
		args.push_back(call.pre_compile_call(c));
		types.push_back(args.at(0).t->id());
	}

	int offset = call.object ? 1 : 0;
	auto fun = dynamic_cast<const Function_type*>(callable_version->type);
	auto f = fun ? dynamic_cast<const Function*>(fun->function()) : nullptr;

	for (unsigned i = 0; i < callable_version->type->arguments().size(); ++i) {
		if (i < arguments.size()) {
			types.push_back((LSValueType) callable_version->type->argument(i + offset)->id());
			auto arg = arguments.at(i)->compile(c);
			if (arguments.at(i)->type->is_primitive()) {
				args.push_back(c.insn_convert(arg, callable_version->type->argument(i + offset)));
				arguments.at(i)->compile_end(c);
			} else if (arguments.at(i)->type->is_polymorphic() and callable_version->type->argument(i + offset)->is_primitive()) {
				arguments.at(i)->compile_end(c);
				args.push_back(c.insn_convert(arg, callable_version->type->argument(i + offset), true));
			} else {
				args.push_back(arg);
				arguments.at(i)->compile_end(c);
			}
		} else if (f and f->defaultValues.at(i)) {
			types.push_back((LSValueType) f->defaultValues.at(i)->type->id());
			args.push_back(f->defaultValues.at(i)->compile(c));
		}
	}
	// Check arguments
	c.insn_check_args(args, types);
	auto r = call.compile_call(c, callable_version, args, is_void);
	c.inc_ops(1);
	if (r.t->is_mpz()) {
		auto r2 = c.create_entry("m", r.t);
		c.insn_store(r2, r);
		r = r2;
	}
	return r;
}

Value* FunctionCall::clone() const {
	auto fc = new FunctionCall(token);
	fc->function = function->clone();
	for (const auto& a : arguments) {
		fc->arguments.push_back(a->clone());
	}
	fc->closing_parenthesis = closing_parenthesis;
	return fc;
}

}
