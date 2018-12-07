#include "Interval_type.hpp"
#include "../colors.h"
#include "Type.hpp"
#include "Any_type.hpp"

namespace ls {

bool Interval_type::operator == (const Base_type* type) const {
	return dynamic_cast<const Interval_type*>(type);
}
Type Interval_type::element() const {
	return Type::INTEGER;
}
std::string Interval_type::clazz() const {
	return "Interval";
}
llvm::Type* Interval_type::llvm() const {
	return Any_type::get_any_type();
}

}