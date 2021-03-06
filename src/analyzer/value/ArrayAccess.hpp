#ifndef ARRAYACCESS_HPP
#define ARRAYACCESS_HPP

#include <memory>
#include "LeftValue.hpp"
#include "Value.hpp"
#include "../lexical/Token.hpp"
#include "../../type/Type.hpp"

namespace ls {

class ArrayAccess : public LeftValue {
public:

	std::unique_ptr<Value> array;
	std::unique_ptr<Value> key;
	std::unique_ptr<Value> key2;
	Token* open_bracket;
	Token* close_bracket;
	const Type* map_key_type;
	bool should_delete_array = false;
	std::unique_ptr<Callable> callable;
	#if COMPILER
	Compiler::value compiled_array;
	#endif

	ArrayAccess(Environment& env);

	virtual bool isLeftValue() const override;

	virtual void print(std::ostream&, int indent, PrintOptions options) const override;
	virtual Location location() const override;

	virtual void pre_analyze(SemanticAnalyzer*) override;
	virtual Call get_callable(SemanticAnalyzer*, int argument_count) const override;
	virtual void analyze(SemanticAnalyzer*) override;
	virtual const Type* will_take(SemanticAnalyzer* analyzer, const std::vector<const Type*>&, int level) override;
	bool array_access_will_take(SemanticAnalyzer* analyzer, const std::vector<const Type*>&, int level);
	virtual bool will_store(SemanticAnalyzer* analyzer, const Type* type) override;
	virtual void change_value(SemanticAnalyzer*, Value*) override;
	virtual const Type* version_type(std::vector<const Type*>) const override;
	virtual Hover hover(SemanticAnalyzer& analyzer, size_t position) const override;

	#if COMPILER
	virtual Compiler::value compile(Compiler&) const override;
	virtual Compiler::value compile_l(Compiler&) const override;
	virtual void compile_end(Compiler&) const override;
	#endif

	virtual std::unique_ptr<Value> clone(Block* parent) const override;
};

}

#endif
