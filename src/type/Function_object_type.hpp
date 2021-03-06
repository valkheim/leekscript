#ifndef FUNCTION_OBJECT_TYPE_HPP
#define FUNCTION_OBJECT_TYPE_HPP

#include "Pointer_type.hpp"

namespace ls {

class Type;
class Value;

class Function_object_type : public Pointer_type {
	const Type* const _return_type;
	std::vector<const Type*> _arguments;
	bool _closure;
	const Value* _function;
public:
	Function_object_type(const Type*, const std::vector<const Type*>&, bool closure = false, const Value* function = nullptr);
	bool closure() const { return _closure; }
	const Value* function() const override { return _function; }
	virtual int id() const override { return 9; }
	virtual const std::string getName() const override { return "function"; };
	virtual Json json() const override;
	virtual bool callable() const override { return true; }
	virtual bool operator == (const Type*) const override;
	virtual int distance(const Type* type) const override;
	virtual const Type* return_type() const override;
	virtual const std::vector<const Type*>& arguments() const override;
	virtual const Type* argument(size_t) const override;
	virtual std::string class_name() const override;
	virtual std::ostream& print(std::ostream& os) const override;
	virtual Type* clone() const override;
};

}

#endif