#include "Type.hpp"
#include "../constants.h"
#include "../colors.h"
#include <sstream>
#include "../../lib/utf8.h"
#include "../vm/VM.hpp"

using namespace std;

namespace ls {

BaseRawType::~BaseRawType() {}

const AnyRawType RawType::_ANY;
const NullRawType RawType::_NULL;
const BooleanRawType RawType::_BOOLEAN;
const NumberRawType RawType::_NUMBER;
const MpzRawType RawType::_MPZ;
const IntegerRawType RawType::_INTEGER;
const LongRawType RawType::_LONG;
const FloatRawType RawType::_REAL;
const StringRawType RawType::_STRING;
const ArrayRawType RawType::_ARRAY;
const MapRawType RawType::_MAP;
const SetRawType RawType::_SET;
const IntervalRawType RawType::_INTERVAL;
const ObjectRawType RawType::_OBJECT;
const FunctionRawType RawType::_FUNCTION;
const ClosureRawType RawType::_CLOSURE;
const ClassRawType RawType::_CLASS;

const AnyRawType* const RawType::ANY = &_ANY;
const NullRawType* const RawType::NULLL = &_NULL;
const BooleanRawType* const RawType::BOOLEAN = &_BOOLEAN;
const NumberRawType* const RawType::NUMBER = &_NUMBER;
const IntegerRawType* const RawType::INTEGER = &_INTEGER;
const MpzRawType* const RawType::MPZ = &_MPZ;
const LongRawType* const RawType::LONG = &_LONG;
const FloatRawType* const RawType::REAL = &_REAL;
const StringRawType* const RawType::STRING = &_STRING;
const ArrayRawType* const RawType::ARRAY = &_ARRAY;
const MapRawType* const RawType::MAP = &_MAP;
const SetRawType* const RawType::SET = &_SET;
const IntervalRawType* const RawType::INTERVAL = &_INTERVAL;
const ObjectRawType* const RawType::OBJECT = &_OBJECT;
const FunctionRawType* const RawType::FUNCTION = &_FUNCTION;
const ClosureRawType* const RawType::CLOSURE = &_CLOSURE;
const ClassRawType* const RawType::CLASS = &_CLASS;

llvm::StructType* Type::LLVM_LSVALUE_TYPE;
llvm::Type* Type::LLVM_LSVALUE_TYPE_PTR;
llvm::Type* Type::LLVM_LSVALUE_TYPE_PTR_PTR;
llvm::Type* Type::LLVM_MPZ_TYPE;
llvm::Type* Type::LLVM_MPZ_TYPE_PTR;
llvm::StructType* Type::LLVM_VECTOR_TYPE;
llvm::Type* Type::LLVM_VECTOR_TYPE_PTR;
llvm::StructType* Type::LLVM_VECTOR_INT_TYPE;
llvm::Type* Type::LLVM_VECTOR_INT_TYPE_PTR;
llvm::StructType* Type::LLVM_VECTOR_REAL_TYPE;
llvm::Type* Type::LLVM_VECTOR_REAL_TYPE_PTR;
llvm::StructType* Type::LLVM_FUNCTION_TYPE;
llvm::Type* Type::LLVM_FUNCTION_TYPE_PTR;
llvm::StructType* Type::LLVM_INTEGER_ITERATOR_TYPE;

std::vector<const BaseRawType*> RawType::placeholder_types;

void RawType::clear_placeholder_types() {
	for (const auto& t : placeholder_types)
		delete t;
	placeholder_types.clear();
}

unsigned int Type::placeholder_counter = 0;

const Type Type::VALUE(RawType::ANY, Nature::VALUE);
const Type Type::CONST_VALUE(RawType::ANY, Nature::VALUE, false, false, true);
const Type Type::POINTER(RawType::ANY, Nature::POINTER);
const Type Type::CONST_POINTER(RawType::ANY, Nature::POINTER, false, false, true);

const Type Type::NULLL(RawType::NULLL, Nature::POINTER, true);
const Type Type::BOOLEAN(RawType::BOOLEAN, Nature::VALUE);
const Type Type::CONST_BOOLEAN(RawType::BOOLEAN, Nature::VALUE, false, false, true);
const Type Type::NUMBER(RawType::NUMBER, Nature::VALUE);
const Type Type::CONST_NUMBER(RawType::NUMBER, Nature::VALUE, false, false, true);
const Type Type::INTEGER(RawType::INTEGER, Nature::VALUE);
const Type Type::CONST_INTEGER(RawType::INTEGER, Nature::VALUE, false, false, true);
const Type Type::MPZ(RawType::MPZ, Nature::VALUE);
const Type Type::MPZ_TMP(RawType::MPZ, Nature::VALUE, false, true);
const Type Type::LONG(RawType::LONG, Nature::VALUE);
const Type Type::CONST_LONG(RawType::LONG, Nature::VALUE, false, false, true);
const Type Type::REAL(RawType::REAL, Nature::VALUE);
const Type Type::CONST_REAL(RawType::REAL, Nature::VALUE, false, false, true);
const Type Type::STRING(RawType::STRING, Nature::POINTER);
const Type Type::CONST_STRING(RawType::STRING, Nature::POINTER, false, false, true);
const Type Type::STRING_TMP(RawType::STRING, Nature::POINTER, false, true);
const Type Type::OBJECT(RawType::OBJECT, Nature::POINTER);
const Type Type::OBJECT_TMP(RawType::OBJECT, Nature::POINTER, false, true);

const Type Type::ARRAY(RawType::ARRAY, Nature::POINTER);
const Type Type::PTR_ARRAY(RawType::ARRAY, Nature::POINTER, {});
const Type Type::INT_ARRAY(RawType::ARRAY, Nature::POINTER, Type::INTEGER);
const Type Type::REAL_ARRAY(RawType::ARRAY, Nature::POINTER, Type::REAL);
const Type Type::PTR_ARRAY_TMP(RawType::ARRAY, Nature::POINTER, Type::POINTER, false, true);
const Type Type::INT_ARRAY_TMP(RawType::ARRAY, Nature::POINTER, Type::INTEGER, false, true);
const Type Type::REAL_ARRAY_TMP(RawType::ARRAY, Nature::POINTER, Type::REAL, false, true);
const Type Type::STRING_ARRAY(RawType::ARRAY, Nature::POINTER, Type::STRING);
const Type Type::CONST_PTR_ARRAY(RawType::ARRAY, Nature::POINTER, {}, false, false, true);
const Type Type::CONST_INT_ARRAY(RawType::ARRAY, Nature::POINTER, Type::INTEGER, false, false, true);
const Type Type::CONST_REAL_ARRAY(RawType::ARRAY, Nature::POINTER, Type::REAL, false, false, true);
const Type Type::CONST_STRING_ARRAY(RawType::ARRAY, Nature::POINTER, Type::STRING, false, false, true);

const Type Type::MAP(RawType::MAP, Nature::POINTER);
const Type Type::PTR_PTR_MAP(RawType::MAP, Nature::POINTER, Type::POINTER, Type::POINTER);
const Type Type::PTR_INT_MAP(RawType::MAP, Nature::POINTER, Type::POINTER, Type::INTEGER);
const Type Type::PTR_REAL_MAP(RawType::MAP, Nature::POINTER, Type::POINTER, Type::REAL);
const Type Type::REAL_PTR_MAP(RawType::MAP, Nature::POINTER, Type::REAL, Type::POINTER);
const Type Type::REAL_INT_MAP(RawType::MAP, Nature::POINTER, Type::REAL, Type::INTEGER);
const Type Type::REAL_REAL_MAP(RawType::MAP, Nature::POINTER, Type::REAL, Type::REAL);
const Type Type::INT_PTR_MAP(RawType::MAP, Nature::POINTER, Type::INTEGER, Type::POINTER);
const Type Type::INT_INT_MAP(RawType::MAP, Nature::POINTER, Type::INTEGER, Type::INTEGER);
const Type Type::INT_REAL_MAP(RawType::MAP, Nature::POINTER, Type::INTEGER, Type::REAL);
const Type Type::CONST_PTR_PTR_MAP(RawType::MAP, Nature::POINTER, Type::POINTER, Type::POINTER, false, true);
const Type Type::CONST_PTR_INT_MAP(RawType::MAP, Nature::POINTER, Type::POINTER, Type::INTEGER, false, true);
const Type Type::CONST_PTR_REAL_MAP(RawType::MAP, Nature::POINTER, Type::POINTER, Type::REAL, false, true);
const Type Type::CONST_REAL_PTR_MAP(RawType::MAP, Nature::POINTER, Type::REAL, Type::POINTER, false, true);
const Type Type::CONST_REAL_INT_MAP(RawType::MAP, Nature::POINTER, Type::REAL, Type::INTEGER, false, true);
const Type Type::CONST_REAL_REAL_MAP(RawType::MAP, Nature::POINTER, Type::REAL, Type::REAL, false, true);
const Type Type::CONST_INT_PTR_MAP(RawType::MAP, Nature::POINTER, Type::INTEGER, Type::POINTER, false, true);
const Type Type::CONST_INT_INT_MAP(RawType::MAP, Nature::POINTER, Type::INTEGER, Type::INTEGER, false, true);
const Type Type::CONST_INT_REAL_MAP(RawType::MAP, Nature::POINTER, Type::INTEGER, Type::REAL, false, true);

const Type Type::SET(RawType::SET, Nature::POINTER);
const Type Type::INT_SET(RawType::SET, Nature::POINTER, Type::INTEGER);
const Type Type::REAL_SET(RawType::SET, Nature::POINTER, Type::REAL);
const Type Type::CONST_SET(RawType::SET, Nature::POINTER, {}, false, false, true);
const Type Type::CONST_INT_SET(RawType::SET, Nature::POINTER, Type::INTEGER, false, false, true);
const Type Type::CONST_REAL_SET(RawType::SET, Nature::POINTER, Type::REAL, false, false, true);

const Type Type::INTERVAL(RawType::INTERVAL, Nature::POINTER, Type::INTEGER);
const Type Type::INTERVAL_TMP(RawType::INTERVAL, Nature::POINTER, Type::INTEGER, false, true);
const Type Type::PTR_ARRAY_ARRAY(RawType::ARRAY, Nature::POINTER, Type::PTR_ARRAY);
const Type Type::REAL_ARRAY_ARRAY(RawType::ARRAY, Nature::POINTER, Type::REAL_ARRAY);
const Type Type::INT_ARRAY_ARRAY(RawType::ARRAY, Nature::POINTER, Type::INT_ARRAY);

const Type Type::FUNCTION(RawType::FUNCTION, Nature::VALUE, false, false, true);
const Type Type::FUNCTION_P(RawType::FUNCTION, Nature::POINTER, false, false, true);
const Type Type::CONST_FUNCTION(RawType::FUNCTION, Nature::VALUE, false, true, true);
const Type Type::CONST_FUNCTION_P(RawType::FUNCTION, Nature::POINTER, false, true, true);
const Type Type::CLOSURE(RawType::CLOSURE, Nature::POINTER, false, false, true);
const Type Type::CLASS(RawType::CLASS, Nature::POINTER, true);
const Type Type::CONST_CLASS(RawType::CLASS, Nature::POINTER, true, false, true);

const Type Type::STRING_ITERATOR(RawType::STRING, Nature::VALUE, Type::STRING);
const Type Type::INTERVAL_ITERATOR(RawType::INTERVAL, Nature::VALUE, Type::INTEGER);
const Type Type::SET_ITERATOR(RawType::SET, Nature::VALUE, Type::POINTER);
const Type Type::INTEGER_ITERATOR(RawType::INTEGER, Nature::VALUE, Type::INTEGER);
const Type Type::LONG_ITERATOR(RawType::LONG, Nature::VALUE, Type::INTEGER);
const Type Type::MPZ_ITERATOR(RawType::MPZ, Nature::VALUE, Type::INTEGER);
const Type Type::INT_ARRAY_ITERATOR(RawType::INTEGER, Nature::POINTER, Type::INTEGER);
const Type Type::REAL_ARRAY_ITERATOR(RawType::REAL, Nature::POINTER, Type::REAL);
const Type Type::PTR_ARRAY_ITERATOR(RawType::ANY, Nature::POINTER, Type::POINTER);

Type::Type() {
	raw_type = RawType::ANY;
	nature = (Nature) 3;
	native = false;
	clazz = "?";
}

Type::Type(const BaseRawType* raw_type) {
	this->raw_type = raw_type;
	_types.push_back(raw_type);
}

Type::Type(const BaseRawType* raw_type, Nature nature, bool native, bool temporary, bool constant) {
	this->raw_type = raw_type;
	_types.push_back(raw_type);
	this->nature = nature;
	this->native = native;
	this->clazz = raw_type->getClass();
	this->temporary = temporary;
	this->constant = constant;
}

Type::Type(const BaseRawType* raw_type, Nature nature, const Type& element_type, bool native, bool temporary, bool constant) {
	this->raw_type = raw_type;
	_types.push_back(raw_type);
	this->nature = nature;
	this->native = native;
	this->clazz = raw_type->getClass();
	this->setElementType(element_type);
	this->constant = constant;
	this->temporary = temporary;
}

Type::Type(const BaseRawType* raw_type, Nature nature, const Type& key_type, const Type& element_type, bool native, bool constant) {
	this->raw_type = raw_type;
	_types.push_back(raw_type);
	this->nature = nature;
	this->native = native;
	this->clazz = raw_type->getClass();
	this->setKeyType(key_type);
	this->setElementType(element_type);
	this->constant = constant;
}

int Type::id() const {
	return raw_type->id();
}

bool Type::must_manage_memory() const {
	if (_types.size() == 0) { return false; }
	return !isNumber() and not native;
}

Type Type::getReturnType() const {
	if (return_types.size() == 0) {
		return Type::POINTER;
	}
	return return_types[0];
}

void Type::setReturnType(Type type) {
	if (return_types.size() == 0) {
		return_types.push_back({});
	}
	return_types[0] = type;
}

void Type::addArgumentType(Type type) {
	arguments_types.push_back(type);
}

void Type::setArgumentType(size_t index, Type type, bool has_default) {
	while (arguments_types.size() <= index) {
		arguments_types.push_back(Type::POINTER);
	}
	arguments_types[index] = type;

	while (arguments_has_default.size() <= index) {
		arguments_has_default.push_back(false);
	}
	arguments_has_default[index] = has_default;
}

/*
 * By default, all arguments are type INTEGER, but if we see it's not always
 * a integer, it will switch to UNKNOWN
 */
const Type Type::getArgumentType(size_t index) const {
	if (index >= arguments_types.size()) {
		return Type::POINTER;
	}
	return arguments_types[index];
}

const std::vector<Type> Type::getArgumentTypes() const {
	return arguments_types;
}

bool Type::argumentHasDefault(size_t index) const {
	if (index >= arguments_has_default.size()) {
		return false;
	}
	return arguments_has_default[index];
}

const Type Type::getElementType() const {
	if (element_type.size()) {
		return element_type[0];
	}
	return {};
}

void Type::setElementType(const Type& type) {
	if (element_type.size() == 0) {
		element_type.push_back(type);
	} else {
		element_type[0] = type;
	}
}

const Type Type::getKeyType() const {
	if (key_type.size()) {
		return key_type[0];
	}
	return {};
}
void Type::setKeyType(const Type& type) {
	if (key_type.size() == 0) {
		key_type.push_back(type);
	} else {
		key_type[0] = type;
	}
}

bool Type::will_take(const std::vector<Type>& args_type) {
	bool changed = false;
	for (size_t i = 0; i < args_type.size(); ++i) {
		Type current_type = getArgumentType(i);
		if (current_type.isNumber() and !args_type[i].isNumber()) {
			setArgumentType(i, Type::POINTER);
			changed = true;
		}
	}
	return changed;
}

void Type::add(const Type type) {
	for (const auto& t : type._types) {
		add(t);
	}
	key_type = type.key_type;
	element_type = type.element_type;
	// arguments_types = type.arguments_types;
	// return_types = type.return_types;
	// arguments_has_default = type.arguments_has_default;
	// native = type.native;
	nature = type.nature;
}

void Type::add(const BaseRawType* type) {
	for (const auto& t : _types) {
		if (type == t)
			return;
	}
	raw_type = type;
	_types.push_back(type);
}

void Type::toJson(ostream& os) const {
	os << "{\"type\":\"" << raw_type->getJsonName() << "\"";

	if (raw_type == RawType::FUNCTION) {
		os << ",\"args\":[";
		for (unsigned t = 0; t < arguments_types.size(); ++t) {
			if (t > 0) os << ",";
			arguments_types[t].toJson(os);
		}
		os << "]";
		os << ",\"return\":";
		getReturnType().toJson(os);
	}
	os << "}";
}

std::string Type::to_string() const {
	std::ostringstream oss;
	oss << *this;
	return oss.str();
}

bool Type::isNumber() const {
	return dynamic_cast<const NumberRawType*>(raw_type) != nullptr || raw_type == RawType::BOOLEAN;
}

bool Type::iterable() const {
	return raw_type->iterable();
}

bool Type::is_container() const {
	return raw_type->is_container();
}

int Type::size() const {
	return raw_type->size();
}

bool Type::operator == (const Type& type) const {
	return raw_type == type.raw_type &&
			nature == type.nature &&
			// native == type.native &&
			// clazz == type.clazz &&
			element_type == type.element_type &&
			key_type == type.key_type &&
			// temporary == type.temporary &&
			// reference == type.reference &&
			((raw_type != RawType::FUNCTION and raw_type != RawType::CLOSURE) ||
				(return_types == type.return_types &&
				arguments_types == type.arguments_types));
}

bool Type::operator < (const Type& type) const {
	if ((void*) raw_type != (void*) type.raw_type) {
		return (void*) raw_type < (void*) type.raw_type;
	}
	if (nature != type.nature) {
		return nature < type.nature;
	}
	if (temporary != type.temporary) {
		return temporary < type.temporary;
	}
	if (reference != type.reference) {
		return reference < type.reference;
	}
	if (native != type.native) {
		return native < type.native;
	}
	for (size_t i = 0; i < element_type.size(); ++i) {
		if (i >= type.element_type.size()) return false;
		if (element_type.at(i) != type.element_type.at(i)) {
			return element_type.at(i) < type.element_type.at(i);
		}
	}
	for (size_t i = 0; i < key_type.size(); ++i) {
		if (i >= type.key_type.size()) return false;
		if (key_type.at(i) != type.key_type.at(i)) {
			return key_type.at(i) < type.key_type.at(i);
		}
	}
	return false;
}

Type Type::not_temporary() const {
	Type new_type = *this;
	new_type.temporary = false;
	return new_type;
}
Type Type::add_temporary() const {
	Type new_type = *this;
	new_type.temporary = true;
	return new_type;
}

llvm::Type* Type::llvm_type() const {
	if (_types.size() == 0) {
		return llvm::Type::getVoidTy(LLVMCompiler::context);
	}
	if (*this == Type::INT_ARRAY_ITERATOR) {
		return llvm::Type::getInt32PtrTy(LLVMCompiler::context);
	}
	if (*this == Type::PTR_ARRAY_ITERATOR) {
		return LLVM_LSVALUE_TYPE_PTR->getPointerTo();
	}
	if (raw_type == RawType::ARRAY) {
		if (getElementType() == Type::INTEGER) {
			return LLVM_VECTOR_INT_TYPE_PTR;
		} else if (getElementType() == Type::REAL) {
			return LLVM_VECTOR_REAL_TYPE_PTR;
		} else {
			return LLVM_VECTOR_TYPE_PTR;
		}
	}
	if (reference or !isNumber() or raw_type == RawType::FUNCTION) {
		return LLVM_LSVALUE_TYPE_PTR;
	}
	if (raw_type == RawType::MPZ) {
		return LLVM_MPZ_TYPE;
	}
	if (raw_type == RawType::BOOLEAN) {
		return llvm::Type::getInt1Ty(LLVMCompiler::context);
	}
	if (raw_type == RawType::LONG) {
		return llvm::Type::getInt64Ty(LLVMCompiler::context);
	}
	if (raw_type == RawType::REAL) {
		return llvm::Type::getDoubleTy(LLVMCompiler::context);
	}
	return llvm::Type::getInt32Ty(LLVMCompiler::context);
}

Type Type::add_pointer() const {
	Type new_type = *this;
	new_type.pointer = true;
	return new_type;
}

Type Type::iteratorType() const {
	if (raw_type == RawType::ARRAY) {
		if (getElementType() == Type::INTEGER) return Type::INT_ARRAY_ITERATOR;
		if (getElementType() == Type::REAL) return Type::REAL_ARRAY_ITERATOR;
		else if (getElementType() == Type::POINTER) return Type::PTR_ARRAY_ITERATOR;
	}
	assert(false && "No iterator type available");
}

/*
 * Can we convert type into this ?
 * {float}.compatible({int}) == true
 * {int*}.compatible({int}) == true
 * {int}.compatible({float}) == false
 * {int}.compatible({int*}) == false
 */
bool Type::compatible(const Type& type) const {
	if (this->isNumber() && !type.isNumber()) {
		return false;
	}
	if (this->temporary and not type.temporary) {
		return false; // type not compatible with type&&
	}
	if (not this->constant and type.constant) {
		return false; // 'const type' not compatible with 'type'
	}
	if (this->raw_type != type.raw_type) {
		// Every type is compatible with 'Unknown' type
		if (this->raw_type == RawType::ANY) {
			return true;
		}
		// 'Integer' is compatible with 'Float'
		if (this->raw_type == RawType::REAL and type.raw_type == RawType::INTEGER) {
			return true;
		}
		// 'Boolean' is compatible with 'Integer'
		if (this->raw_type == RawType::INTEGER and type.raw_type == RawType::BOOLEAN) {
			return true;
		}
		// 'Integer' is compatible with 'Long'
		if (this->raw_type == RawType::LONG and type.raw_type == RawType::INTEGER) {
			return true;
		}

		// All numbers types are compatible with the base 'Number' type
		if (this->raw_type == RawType::NUMBER and (
			type.raw_type == RawType::INTEGER or
			type.raw_type == RawType::LONG or
			type.raw_type == RawType::REAL
		)) return true;

		return false;
	}

	if (this->raw_type == RawType::ARRAY || this->raw_type == RawType::SET) {
		const Type e1 = this->getElementType();
		const Type e2 = type.getElementType();
		if (e1.raw_type == RawType::ANY or e2.raw_type == RawType::ANY) {
			return true;
		}
		if (e1._types.size() == 0 or e2._types.size() == 0) {
			return true;
		}
		if (!e1.isNumber() && !e2.isNumber()) return true;
		return e1 == e2;
	}
	if (this->raw_type == RawType::MAP) {
		const Type& k1 = this->getKeyType();
		const Type& k2 = type.getKeyType();
		const Type& v1 = this->getElementType();
		const Type& v2 = type.getElementType();
		if (!k1.isNumber() && !k2.isNumber()) {
			if (!v1.isNumber() && !v2.isNumber()) {
				return true;
			}
			return v1 == v2;
		} else {
			if (!v1.isNumber() && !v2.isNumber()) {
				return k1 == k2;
			}
			return k1 == k2 && v1 == v2;
		}
	}

	return true;
}

bool Type::list_compatible(const std::vector<Type>& expected, const std::vector<Type>& actual) {

	if (expected.size() != actual.size()) return false;

	for (size_t i = 0; i < expected.size(); ++i) {
		// Can we convert type actual[i] into type expected[i] ?
		if (not expected[i].compatible(actual[i])) return false;
	}
	return true;
}

bool Type::may_be_compatible(const Type& type) const {
	if (compatible(type)) {
		return true;
	}
	// Example: Number.abs(number*) => we allow to call with a unknown pointer
	if (!isNumber() and type == Type::POINTER) {
		return true;
	}
	return false;
}

bool Type::list_may_be_compatible(const std::vector<Type>& expected, const std::vector<Type>& actual) {
	if (expected.size() != actual.size()) return false;
	for (size_t i = 0; i < expected.size(); ++i) {
		if (not expected[i].may_be_compatible(actual[i])) return false;
	}
	return true;
}

bool Type::list_more_specific(const std::vector<Type>& old, const std::vector<Type>& neww) {

	if (old.size() != neww.size()) return false;

	for (size_t i = 0; i < old.size(); ++i) {
		if (Type::more_specific(old[i], neww[i])) {
			return true;
		}
	}
	return false;
}

bool Type::more_specific(const Type& old, const Type& neww) {

	if (neww.raw_type != old.raw_type) {
		if (old.raw_type == RawType::ANY) {
			return true;
		}
		if (old.raw_type == RawType::NUMBER && (neww.raw_type == RawType::INTEGER || neww.raw_type == RawType::LONG || neww.raw_type == RawType::REAL)) {
			return true;
		}
		if (old.raw_type == RawType::REAL && (neww.raw_type == RawType::INTEGER || neww.raw_type == RawType::LONG)) {
			return true;
		}
		if (old.raw_type == RawType::LONG && neww.raw_type == RawType::INTEGER) {
			return true;
		}
	}

	if ((neww.raw_type == RawType::ARRAY and old.raw_type == RawType::ARRAY)
			|| (neww.raw_type == RawType::SET and old.raw_type == RawType::SET)) {
		if (Type::more_specific(old.getElementType(), neww.getElementType())) {
			return true;
		}
	}

	if (neww.raw_type == RawType::MAP and old.raw_type == RawType::MAP) {
		if (Type::more_specific(old.getKeyType(), neww.getKeyType())
				|| Type::more_specific(old.getElementType(), neww.getElementType())) {
			return true;
		}
	}

	if ((neww.raw_type == RawType::FUNCTION or neww.raw_type == RawType::CLOSURE) and (old.raw_type == RawType::FUNCTION or old.raw_type == RawType::CLOSURE)) {
		if (Type::more_specific(old.getArgumentType(0), neww.getArgumentType(0))) { //! TODO only the first arg
			return true;
		}
	}

	if (not old.temporary and neww.temporary) {
		return true;
	}

	return false;
}

Type Type::get_compatible_type(const Type& t1, const Type& t2) {
	if (t1 == t2) {
		return t1;
	}
	if (t1.raw_type == RawType::NULLL or t2.raw_type == RawType::NULLL) {
		return Type::NULLL;
	}
	if (t1.nature == Nature::POINTER and t2.isNumber()) {
		return Type::POINTER;
	}
	if (t2.nature == Nature::POINTER and t1.isNumber()) {
		return Type::POINTER;
	}
	if (t1.raw_type == RawType::ANY) {
		return t2;
	}
	if (t2.raw_type == RawType::ANY) {
		return t1;
	}
	if (t2.compatible(t1)) {
		return t2;
	}
	if (t1.compatible(t2)) {
		return t1;
	}
	if (t1.raw_type == RawType::ARRAY and t2.raw_type == RawType::ARRAY) {
		return {RawType::ARRAY, Nature::POINTER, Type::get_compatible_type(t1.getElementType(), t2.getElementType())};
	}
	return Type::POINTER;
}

Type Type::generate_new_placeholder_type() {
	Type type;
	// type.nature = VALUE;
	u_int32_t character = 0x03B1 + placeholder_counter;
	char buff[5];
	u8_toutf8(buff, 5, &character, 1);
	type.raw_type = new PlaceholderRawType(std::string { buff });
	placeholder_counter++;
	RawType::placeholder_types.push_back(type.raw_type);
	return type;
}

ostream& operator << (ostream& os, const Type& type) {

	if (type._types.size() == 0) {
		os << C_GREY << "void" << END_COLOR;
		return os;
	}

	auto color = (type.isNumber()) ? C_GREEN : C_RED;
	os << color;

	if (type.raw_type == RawType::ANY) {
		os << C_YELLOW;
		if (type.constant) os << "const:";
		if (type == Type::INT_ARRAY_ITERATOR) {
			os << "iterator<array<int>>";
		} else if (type == Type::REAL_ARRAY_ITERATOR) {
			os << "iterator<array<real>>";
		} else if (type == Type::PTR_ARRAY_ITERATOR) {
			os << "iterator<array<*>>";
		} else {
			os << "?";
		}
	} else if (type.raw_type == RawType::FUNCTION || type.raw_type == RawType::CLOSURE) {
		os << BLUE_BOLD;
		if (type.constant) os << "const:";
		os << (type.raw_type == RawType::FUNCTION ? "fun(" : "closure(");
		for (unsigned t = 0; t < type.arguments_types.size(); ++t) {
			if (t > 0) os << ", ";
			os << type.arguments_types[t];
			os << BLUE_BOLD;
		}
		os << BLUE_BOLD;
		os << ") → " << type.getReturnType();
	} else if (type.raw_type == RawType::STRING || type.raw_type == RawType::CLASS
		|| type.raw_type == RawType::OBJECT || type.raw_type == RawType::NULLL
		|| type.raw_type == RawType::INTERVAL) {
		os << BLUE_BOLD;
		if (type.constant) os << "const:";
		os << type.raw_type->getName(); // << Type::get_nature_symbol(type.nature);
	} else if (type.raw_type == RawType::ARRAY || type.raw_type == RawType::SET) {
		os << BLUE_BOLD;
		if (type.constant) os << "const:";
		os << type.raw_type->getName(); // << Type::get_nature_symbol(type.nature);
		os << "<" << type.getElementType() << BLUE_BOLD << ">";
	} else if (type.raw_type == RawType::MAP) {
		os << BLUE_BOLD;
		if (type.constant) os << "const:";
		os << type.raw_type->getName();
		os << "<" << type.getKeyType() << BLUE_BOLD
			<< ", " << type.getElementType() << BLUE_BOLD << ">";
	} else {
		if (type.constant) os << "const:";
		os << type.raw_type->getName();
	}
	if (type.temporary && type.raw_type != RawType::REAL) {
		os << "&&";
	} else if (type.reference) {
		os << "&";
	}
	os << END_COLOR;

	return os;
}

ostream& operator << (ostream& os, const std::vector<Type>& types) {
	os << "[";
	for (unsigned i = 0; i < types.size(); ++i) {
		if (i > 0) os << ", ";
		os << types[i];
	}
	os << "]";
	return os;
}

std::ostream& operator << (std::ostream& os, const llvm::Type* type) {
	std::string str;
	llvm::raw_string_ostream rso(str);
	type->print(rso);
	os << rso.str();
	return os;
}

}
