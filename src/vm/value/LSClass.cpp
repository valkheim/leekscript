#include "LSClass.hpp"
#include "LSString.hpp"
#include "LSNumber.hpp"
#include "LSFunction.hpp"
#include "../../standard/Module.hpp"
#include "../../analyzer/semantic/Callable.hpp"
#include "../../vm/VM.hpp"
#include "../../analyzer/semantic/CallableVersion.hpp"
#include "../../analyzer/semantic/SemanticAnalyzer.hpp"
#include "../../analyzer/semantic/Variable.hpp"
#include "../../analyzer/semantic/Class.hpp"

namespace ls {

LSClass* LSClass::constructor(VM* vm, char* name) {
	auto clazz = new LSClass(new Class(vm->env, name));
	vm->function_created.push_back(clazz);
	vm->class_created.push_back(clazz->clazz);
	return clazz;
}
LSClass::LSClass(Class* clazz) : LSValue(CLASS, 1, true), clazz(clazz) {}

LSClass::~LSClass() {}

bool LSClass::to_bool() const {
	return true;
}

bool LSClass::ls_not() const {
	return false;
}

bool LSClass::eq(const LSValue* v) const {
	if (auto clazz = dynamic_cast<const LSClass*>(v)) {
		return clazz->clazz == this->clazz;
	}
	return false;
}

bool LSClass::lt(const LSValue* v) const {
	if (auto clazz = dynamic_cast<const LSClass*>(v)) {
		return this->clazz->name < clazz->clazz->name;
	}
	return LSValue::lt(v);
}

LSValue* LSClass::attr(VM* vm, const std::string& key) const {
	if (key == "name") {
		return new LSString(clazz->name);
	}
	try {
		return clazz->static_fields.at(key).value;
	} catch (std::exception&) {
		return LSValue::attr(vm, key);
	}
}

std::ostream& LSClass::dump(std::ostream& os, int) const {
	os << "<class " << clazz->name << ">";
	return os;
}

std::string LSClass::json() const {
	return "\"<class " + clazz->name + ">\"";
}

LSValue* LSClass::getClass(VM* vm) const {
	return vm->env.class_class.get();
}

}
