#ifndef LSCLASS_HPP_
#define LSCLASS_HPP_

#include <string>
#include <map>
#include "../LSValue.hpp"
#include "../../compiler/Compiler.hpp"
#include "../../standard/TypeMutator.hpp"
#include "../../analyzer/semantic/Callable.hpp"

namespace ls {

class Method;
class ModuleStaticField;
class Type;
class Class;

class LSClass : public LSValue {
public:
	static LSClass* constructor(VM* vm, char* name);

	Class* clazz;

	LSClass(std::string);
	LSClass(Class* clazz);

	virtual ~LSClass();

	bool to_bool() const override;
	virtual bool ls_not() const override;

	bool eq(const LSValue*) const override;
	bool lt(const LSValue*) const override;

	LSValue* attr(VM* vm, const std::string& key) const override;

	std::ostream& dump(std::ostream& os, int level) const override;
	std::string json() const override;

	LSValue* getClass(VM* vm) const override;
};

}

#endif
