#ifndef MAP_HPP
#define MAP_HPP

#include <vector>
#include <memory>
#include "Value.hpp"
#include "../lexical/Token.hpp"

namespace ls {

class Map : public Value {
public:
	Token* opening_bracket;
	Token* closing_bracket;
	std::vector<std::unique_ptr<Value>> keys;
	std::vector<std::unique_ptr<Value>> values;

	Map(Environment& env);

	virtual void print(std::ostream&, int indent, PrintOptions options) const override;
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
