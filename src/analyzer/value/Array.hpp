#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <vector>
#include <memory>
#include "Value.hpp"
#include "../lexical/Token.hpp"

namespace ls {

class Array : public Value {
public:

	Token* opening_bracket;
	Token* closing_bracket;
	std::vector<std::unique_ptr<Value>> expressions;

	Array(Environment& env);

	virtual void print(std::ostream&, int indent, PrintOptions options) const override;
	virtual Location location() const override;

	virtual void pre_analyze(SemanticAnalyzer*) override;
	virtual void analyze(SemanticAnalyzer*) override;
	void elements_will_take(SemanticAnalyzer*, const std::vector<const Type*>&, int level);
	virtual bool will_store(SemanticAnalyzer* analyzer, const Type* type) override;
	virtual bool elements_will_store(SemanticAnalyzer* analyzer, const Type* type, int level) override;

	virtual Json hover(SemanticAnalyzer& analyzer, size_t position) const override;

	#if COMPILER
	virtual Compiler::value compile(Compiler&) const override;
	#endif

	virtual std::unique_ptr<Value> clone(Block* parent) const override;
};

}

#endif
