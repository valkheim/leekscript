#include "Callable.hpp"
#include "../../colors.h"
#include "../value/LeftValue.hpp"
#include "../../type/Function_type.hpp"

namespace ls {

void Callable::add_version(CallableVersion v) {
	versions.push_back(v);
}

CallableVersion* Callable::resolve(SemanticAnalyser* analyser, std::vector<Type> arguments) {
	// std::cout << "Callable::resolve(" << arguments << ")" << std::endl;
	CallableVersion* best = nullptr;
	int best_score = std::numeric_limits<int>::max();
	for (auto& version : versions) {
		std::vector<Type> version_arguments = arguments;
		if (version.object) {
			version_arguments.insert(version_arguments.begin(), version.object->type);
		}
		if (version.type.arguments().size() != version_arguments.size() and not version.unknown) continue;
		// std::cout << "templates : " << implementation.templates << std::endl;
		// implementation.templates[0].implement(Type::real());
		// implementation.templates[1].implement(Type::real());
		for (size_t i = 0; i < std::min(version.type.arguments().size(), version_arguments.size()); ++i) {
			const auto& a = version_arguments.at(i);
			const auto implem_arg = version.type.arguments().at(i);
			if (auto fun = dynamic_cast<const Function_type*>(a._types[0].get())) {
				if (fun->function() and implem_arg.is_function()) {
					auto version = implem_arg.arguments();
					((Function*) fun->function())->will_take(analyser, version, 1);
					version_arguments.at(i) = fun->function()->versions.at(version)->type;
				}
			}
		}
		int d = 0;
		for (size_t i = 0; i < version_arguments.size(); ++i) {
			auto di = version_arguments.at(i).distance(version.type.arguments().at(i));
			if (di < 0) { d = std::numeric_limits<int>::max(); break; };
			d += di;
		}
		// std::cout << implementation.type.arguments() << " distance " << d << std::endl;
		if (best == nullptr or d <= best_score) {
			best_score = d;
			best = &version;
		}
	}
	return best;
}

Compiler::value CallableVersion::compile_call(Compiler& c, std::vector<Compiler::value> args) const {
	// std::cout << "CallableVersion::compile_call(" << args << ")" << std::endl;
	// Add the object if it's a method call
	if (object) {
		auto obj = [&]() { if (object->isLeftValue()) {
			return c.insn_load(((LeftValue*) object)->compile_l(c));
		} else {
			return object->compile(c);
		}}();
		object->compile_end(c);
		args.insert(args.begin(), obj);
	}
	// Check arguments
	std::vector<LSValueType> types;
	for (unsigned i = 0; i < args.size(); ++i) {
		types.push_back((LSValueType) type.argument(i).id());
	}
	c.insn_check_args(args, types);
	// Do the call
	auto r = [&]() { if (addr) {
		return c.insn_invoke(type.return_type(), args, addr);
	} else if (func) {
		return func(c, args);
	} else {
		auto fun = value->compile(c);
		if (fun.t.is_closure()) {
			args.insert(args.begin(), fun);
		}
		return c.insn_invoke(type.return_type(), args, fun);
	}}();
	c.inc_ops(1);
	return r;
}

}

namespace std {
	std::ostream& operator << (std::ostream& os, const ls::CallableVersion& v) {
		os << v.name << " ";
		if (v.object) os << "★ " << v.object << ":" << v.object->type << " ";
		os << v.type.arguments() << BLUE_BOLD << " => " << END_COLOR << v.type.return_type();
		if (v.addr) {
			os << " (native " << v.addr << ")";
		} else if (v.func) {
			os << " (compiler func)";
		} else {
			os << " (user function)";
		}
		if (v.unknown) os << " (unknown)";
		return os;
	}
	std::ostream& operator << (std::ostream& os, const ls::CallableVersion* v) {
		os << *v;
		return os;
	}
	std::ostream& operator << (std::ostream& os, const ls::Callable* callable) {
		os << callable->name << " [" << std::endl;
		for (const auto& v : callable->versions) {
			os << "    " << v << std::endl;
		}
		os << "]";
		return os;
	}
}