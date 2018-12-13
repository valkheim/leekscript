#include "Struct_type.hpp"
#include "../colors.h"
#include <iostream>
#include "Type.hpp"
#include "Any_type.hpp"

namespace ls {

Struct_type::Struct_type(const std::string name, std::initializer_list<Type> types) : _name(name), _types(types) {
	std::vector<llvm::Type*> llvm_types;
	for (const auto& type : types) {
		llvm_types.push_back(type.llvm_type());
	}
	_llvm_type = llvm::StructType::create(llvm_types, name);
}
Type Struct_type::member(int p) const {
	return _types.at(p);
}
bool Struct_type::operator == (const Base_type* type) const {
	if (auto s = dynamic_cast<const Struct_type*>(type)) {
		return s->_types == _types;
	}
	return false;
}
int Struct_type::distance(const Base_type* type) const {
	if (dynamic_cast<const Any_type*>(type)) { return 1; }
	if (auto s = dynamic_cast<const Struct_type*>(type)) {
		if (s->_types == _types) {
			return 0;
		}
	}
	return -1;
}
llvm::Type* Struct_type::llvm() const {
	return _llvm_type;
}
std::string Struct_type::clazz() const {
	return "Struct";
}
std::ostream& Struct_type::print(std::ostream& os) const {
	os << BLUE_BOLD << _name << END_COLOR;
	return os;
}

}