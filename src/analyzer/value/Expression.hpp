#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <vector>
#include "../../analyzer/lexical/Operator.hpp"
#include "../../analyzer/value/Value.hpp"
#include "../semantic/CallableVersion.hpp"

namespace ls {

class Callable;
class CallableVersion;

class Expression : public Value {
public:

	std::unique_ptr<Value> v1;
	std::unique_ptr<Value> v2;
	std::shared_ptr<Operator> op;
	int operations;
	CallableVersion callable_version;

	Expression(Environment& env, Value* = nullptr);

	void append(std::shared_ptr<Operator>, Value*);

	void print(std::ostream&, int indent, PrintOptions options) const override;
	virtual Location location() const override;

	virtual void pre_analyze(SemanticAnalyzer*) override;
	virtual void analyze(SemanticAnalyzer*) override;
	virtual Hover hover(SemanticAnalyzer& analyzer, size_t position) const override;

	#if COMPILER
	virtual Compiler::value compile(Compiler&) const override;
	#endif

	virtual std::unique_ptr<Value> clone(Block* parent) const override;
};

}

#endif
