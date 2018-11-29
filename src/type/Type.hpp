#ifndef TYPE_HPP
#define TYPE_HPP

#include <vector>
#include <iostream>
#include <cassert>
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"

namespace ls {

enum class Nature {
	ANY, VALUE, POINTER
};

class BaseRawType {
public:
	virtual ~BaseRawType() = 0;
	virtual int id() const { return 0; }
	virtual const std::string getName() const = 0;
	virtual const std::string getClass() const { return "?"; }
	virtual const std::string getJsonName() const { return "?"; }
	virtual bool iterable() const { return false; }
	virtual bool is_container() const { return false; }
	virtual int size() const { return 64; }
	virtual bool is_placeholder() const { return false; }
};

class AnyRawType : public BaseRawType {
public:
	AnyRawType() {}
	virtual ~AnyRawType() {}
	virtual const std::string getName() const { return "any"; }
};

class NullRawType : public BaseRawType {
public:
	NullRawType() {}
	virtual int id() const { return 1; }
	virtual const std::string getName() const { return "null"; }
	virtual const std::string getClass() const { return "Null"; }
	virtual const std::string getJsonName() const { return "null"; }
};

class BooleanRawType : public BaseRawType {
public:
	BooleanRawType() {}
	virtual int id() const { return 2; }
	virtual const std::string getName() const { return "bool"; }
	virtual const std::string getClass() const { return "Boolean"; }
	virtual const std::string getJsonName() const { return "boolean"; }
};

class NumberRawType : public BaseRawType {
public:
	NumberRawType() {}
	virtual int id() const { return 3; }
	virtual const std::string getName() const { return "number"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
	virtual bool iterable() const { return true; }
};

class IntegerRawType : public NumberRawType {
public:
	IntegerRawType() {}
	virtual const std::string getName() const { return "int"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
	virtual int size() const { return 32; }
};

class MpzRawType : public NumberRawType {
public:
	MpzRawType() {}
	virtual const std::string getName() const { return "mpz"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
};

class LongRawType : public NumberRawType {
public:
	LongRawType() {}
	virtual const std::string getName() const { return "long"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
};

class FloatRawType : public NumberRawType {
public:
	FloatRawType() {}
	virtual const std::string getName() const { return "real"; }
	virtual const std::string getClass() const { return "Number"; }
	virtual const std::string getJsonName() const { return "number"; }
};

class StringRawType : public BaseRawType {
public:
	StringRawType() {}
	virtual int id() const { return 4; }
	virtual const std::string getName() const { return "string"; }
	virtual const std::string getClass() const { return "String"; }
	virtual const std::string getJsonName() const { return "string"; }
	virtual bool iterable() const { return true; }
	virtual bool is_container() const { return true; }
};

class ArrayRawType : public BaseRawType {
public:
	ArrayRawType() {}
	virtual int id() const { return 5; }
	virtual const std::string getName() const { return "array"; }
	virtual const std::string getClass() const { return "Array"; }
	virtual const std::string getJsonName() const { return "array"; }
	virtual bool iterable() const { return true; }
	virtual bool is_container() const { return true; }
};

class MapRawType : public BaseRawType {
public:
	MapRawType() {}
	virtual int id() const { return 6; }
	virtual const std::string getName() const { return "map"; }
	virtual const std::string getClass() const { return "Map"; }
	virtual const std::string getJsonName() const { return "map"; }
	virtual bool iterable() const { return true; }
	virtual bool is_container() const { return true; }
};

class SetRawType : public BaseRawType {
public:
	SetRawType() {}
	virtual int id() const { return 7; }
	virtual const std::string getName() const { return "set"; }
	virtual const std::string getClass() const { return "Set"; }
	virtual const std::string getJsonName() const { return "set"; }
	virtual bool iterable() const { return true; }
	virtual bool is_container() const { return true; }
};

class IntervalRawType : public BaseRawType {
public:
	IntervalRawType() {}
	virtual int id() const { return 8; }
	virtual const std::string getName() const { return "interval"; }
	virtual const std::string getClass() const { return "Interval"; }
	virtual const std::string getJsonName() const { return "interval"; }
	virtual bool iterable() const { return true; }
	virtual bool is_container() const { return true; }
};

class ObjectRawType : public BaseRawType {
public:
	ObjectRawType() {}
	virtual int id() const { return 10; }
	virtual const std::string getName() const { return "object"; }
	virtual const std::string getClass() const { return "Object"; }
	virtual const std::string getJsonName() const { return "object"; }
	virtual bool iterable() const { return false; } // TODO not iterable for now
	virtual bool is_container() const { return true; }
};

class FunctionRawType : public BaseRawType {
public:
	FunctionRawType() {}
	virtual int id() const { return 9; }
	virtual const std::string getName() const { return "function"; }
	virtual const std::string getClass() const { return "Function"; }
	virtual const std::string getJsonName() const { return "function"; }
};

class ClosureRawType : public FunctionRawType {
public:
	ClosureRawType() {}
	virtual int id() const { return 12; }
};

class ClassRawType : public BaseRawType {
public:
	ClassRawType() {}
	virtual int id() const { return 11; }
	virtual const std::string getName() const { return "class"; }
	virtual const std::string getClass() const { return "Class"; }
	virtual const std::string getJsonName() const { return "class"; }
};

class PlaceholderRawType : public BaseRawType {
public:
	std::string name;
	PlaceholderRawType(std::string name) : name(name) {}
	virtual ~PlaceholderRawType() {}
	virtual int id() const { assert(!"placeholder type mustn't be compiled"); } // LCOV_EXCL_LINE id() method is used at compilation and placeholder types mustn't be compiled.
	virtual const std::string getName() const { return name; }
	virtual bool is_placeholder() const { return true; }
};

class RawType {
public:
	static const AnyRawType _ANY;
	static const NullRawType _NULL;
	static const BooleanRawType _BOOLEAN;
	static const NumberRawType _NUMBER;
	static const MpzRawType _MPZ;
	static const IntegerRawType _INTEGER;
	static const LongRawType _LONG;
	static const FloatRawType _REAL;
	static const StringRawType _STRING;
	static const ArrayRawType _ARRAY;
	static const MapRawType _MAP;
	static const SetRawType _SET;
	static const IntervalRawType _INTERVAL;
	static const ObjectRawType _OBJECT;
	static const FunctionRawType _FUNCTION;
	static const ClosureRawType _CLOSURE;
	static const ClassRawType _CLASS;

	static const AnyRawType* const ANY;
	static const NullRawType* const NULLL;
	static const BooleanRawType* const BOOLEAN;
	static const NumberRawType* const NUMBER;
	static const MpzRawType* const MPZ;
	static const IntegerRawType* const INTEGER;
	static const LongRawType* const LONG;
	static const FloatRawType* const REAL;
	static const StringRawType* const STRING;
	static const ArrayRawType* const ARRAY;
	static const MapRawType* const MAP;
	static const SetRawType* const SET;
	static const IntervalRawType* const INTERVAL;
	static const ObjectRawType* const OBJECT;
	static const FunctionRawType* const FUNCTION;
	static const ClosureRawType* const CLOSURE;
	static const ClassRawType* const CLASS;

	static std::vector<const BaseRawType*> placeholder_types;
	static void clear_placeholder_types();
};

class Type {
public:
	std::vector<const BaseRawType*> _types;
	const BaseRawType* raw_type;
	Nature nature;
	bool native; // A C++ object, memory management is done outside the language
	std::string clazz;
	std::vector<Type> element_type;
	std::vector<Type> key_type;
	std::vector<Type> return_types;
	std::vector<Type> arguments_types;
	std::vector<bool> arguments_has_default;
	bool pointer = false;
	bool temporary = false;
	bool constant = false;
	bool reference = false;

	Type();
	Type(const BaseRawType* raw_type);
	Type(const BaseRawType* raw_type, Nature nature, bool native = false, bool temporary = false, bool constant = false);
	Type(const BaseRawType* raw_type, Nature nature, const Type& elements_type, bool native = false, bool temporary = false, bool constant = false);
	Type(const BaseRawType* raw_type, Nature nature, const Type& key_type, const Type& element_type, bool native = false, bool constant = false);

	int id() const;

	bool must_manage_memory() const;

	Type getReturnType() const;
	void setReturnType(Type type);

	void addArgumentType(Type type);
	void setArgumentType(size_t index, Type type, bool has_default = false);
	const Type& getArgumentType(size_t index) const;
	const std::vector<Type>& getArgumentTypes() const;
	bool argumentHasDefault(size_t index) const;

	const Type& getElementType() const;
	const Type& element() const;
	void setElementType(const Type&);

	const Type& getKeyType() const;
	void setKeyType(const Type&);

	bool will_take(const std::vector<Type>& args_type);

	void add(const Type type);
	void add(const BaseRawType* type);

	void toJson(std::ostream&) const;
	std::string to_string() const;

	bool isNumber() const;
	bool iterable() const;
	bool is_container() const;
	int size() const;
	Type not_temporary() const;
	Type add_temporary() const;
	llvm::Type* llvm_type() const;
	Type add_pointer() const;
	Type iteratorType() const;

	bool operator ==(const Type& type) const;
	inline bool operator !=(const Type& type) const { return !(*this == type); }

	bool operator < (const Type& type) const;

	bool compatible(const Type& type) const;
	bool may_be_compatible(const Type& type) const;

	/*
	 * Static part
	 */
	static unsigned int placeholder_counter;

	static const Type VALUE;
	static const Type CONST_VALUE;
	static const Type POINTER;
	static const Type CONST_POINTER;

	static const Type ANY;
	static const Type NULLL;
	static const Type CONST_ANY;
	static const Type BOOLEAN;
	static const Type BOOLEAN_P;
	static const Type CONST_BOOLEAN;
	static const Type CONST_BOOLEAN_P;
	static const Type NUMBER;
	static const Type NUMBER_P;
	static const Type CONST_NUMBER;
	static const Type CONST_NUMBER_P;
	static const Type INTEGER;
	static const Type CONST_INTEGER;
	static const Type MPZ;
	static const Type MPZ_TMP;
	static const Type LONG;
	static const Type CONST_LONG;
	static const Type REAL;
	static const Type CONST_REAL;
	static const Type STRING;
	static const Type CONST_STRING;
	static const Type STRING_TMP;
	static const Type OBJECT;
	static const Type OBJECT_TMP;
	static const Type ARRAY;
	static const Type PTR_ARRAY;
	static const Type INT_ARRAY;
	static const Type REAL_ARRAY;
	static const Type PTR_ARRAY_TMP;
	static const Type INT_ARRAY_TMP;
	static const Type REAL_ARRAY_TMP;
	static const Type STRING_ARRAY;
	static const Type CONST_PTR_ARRAY;
	static const Type CONST_INT_ARRAY;
	static const Type CONST_REAL_ARRAY;
	static const Type CONST_STRING_ARRAY;
	static const Type MAP;
	static const Type PTR_PTR_MAP;
	static const Type PTR_INT_MAP;
	static const Type PTR_REAL_MAP;
	static const Type REAL_PTR_MAP;
	static const Type REAL_INT_MAP;
	static const Type REAL_REAL_MAP;
	static const Type INT_PTR_MAP;
	static const Type INT_INT_MAP;
	static const Type INT_REAL_MAP;
	static const Type CONST_PTR_PTR_MAP;
	static const Type CONST_PTR_INT_MAP;
	static const Type CONST_PTR_REAL_MAP;
	static const Type CONST_REAL_PTR_MAP;
	static const Type CONST_REAL_INT_MAP;
	static const Type CONST_REAL_REAL_MAP;
	static const Type CONST_INT_PTR_MAP;
	static const Type CONST_INT_INT_MAP;
	static const Type CONST_INT_REAL_MAP;
	static const Type SET;
	static const Type PTR_SET;
	static const Type INT_SET;
	static const Type REAL_SET;
	static const Type CONST_PTR_SET;
	static const Type CONST_INT_SET;
	static const Type CONST_REAL_SET;
	static const Type INTERVAL;
	static const Type INTERVAL_TMP;
	static const Type FUNCTION;
	static const Type CONST_FUNCTION;
	static const Type FUNCTION_P;
	static const Type CONST_FUNCTION_P;
	static const Type CLOSURE;
	static const Type CLASS;
	static const Type CONST_CLASS;
	static const Type PTR_ARRAY_ARRAY;
	static const Type REAL_ARRAY_ARRAY;
	static const Type INT_ARRAY_ARRAY;
	static const Type INT_ARRAY_ITERATOR;
	static const Type REAL_ARRAY_ITERATOR;
	static const Type PTR_ARRAY_ITERATOR;
	static const Type STRING_ITERATOR;
	static const Type INTERVAL_ITERATOR;
	static const Type SET_ITERATOR;
	static const Type INTEGER_ITERATOR;
	static const Type LONG_ITERATOR;
	static const Type MPZ_ITERATOR;

	static llvm::StructType* LLVM_LSVALUE_TYPE;
	static llvm::Type* LLVM_LSVALUE_TYPE_PTR;
	static llvm::Type* LLVM_LSVALUE_TYPE_PTR_PTR;
	static llvm::Type* LLVM_MPZ_TYPE;
	static llvm::Type* LLVM_MPZ_TYPE_PTR;
	static llvm::StructType* LLVM_VECTOR_TYPE;
	static llvm::Type* LLVM_VECTOR_TYPE_PTR;
	static llvm::StructType* LLVM_VECTOR_INT_TYPE;
	static llvm::Type* LLVM_VECTOR_INT_TYPE_PTR;
	static llvm::StructType* LLVM_VECTOR_REAL_TYPE;
	static llvm::Type* LLVM_VECTOR_REAL_TYPE_PTR;
	static llvm::StructType* LLVM_FUNCTION_TYPE;
	static llvm::Type* LLVM_FUNCTION_TYPE_PTR;
	static llvm::StructType* LLVM_INTEGER_ITERATOR_TYPE;

	static bool list_compatible(const std::vector<Type>& expected, const std::vector<Type>& actual);
	static bool list_may_be_compatible(const std::vector<Type>& expected, const std::vector<Type>& actual);
	static bool list_more_specific(const std::vector<Type>& old, const std::vector<Type>& neww);
	static bool more_specific(const Type& old, const Type& neww);
	static Type get_compatible_type(const Type& t1, const Type& t2);
	static Type generate_new_placeholder_type();
};

std::ostream& operator << (std::ostream&, const Type&);
std::ostream& operator << (std::ostream&, const std::vector<Type>&);
std::ostream& operator << (std::ostream&, const llvm::Type*);

}

#endif
