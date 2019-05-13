#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <vector>
#include <memory>
#include "Value.hpp"
#include "../lexical/Token.hpp"

namespace ls {

class Array : public Value {
public:

	std::shared_ptr<Token> opening_bracket;
	std::shared_ptr<Token> closing_bracket;
	std::vector<Value*> expressions;

	Array();
	virtual ~Array();

	virtual void print(std::ostream&, int indent, bool debug, bool condensed) const override;
	virtual Location location() const override;

	virtual void analyze(SemanticAnalyzer*) override;
	void elements_will_take(SemanticAnalyzer*, const std::vector<Type>&, int level);
	virtual bool will_store(SemanticAnalyzer* analyzer, const Type& type) override;
	virtual bool elements_will_store(SemanticAnalyzer* analyzer, const Type& type, int level) override;

	virtual Compiler::value compile(Compiler&) const override;

	virtual Value* clone() const override;
};

}

#endif
