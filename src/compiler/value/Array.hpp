#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <vector>
#include <memory>
#include "Value.hpp"
#include "../lexical/Token.hpp"

namespace ls {

class Array : public Value {
public:

	std::unique_ptr<Token> opening_bracket;
	std::unique_ptr<Token> closing_bracket;
	std::vector<Value*> expressions;
	bool interval = false;

	Array();
	virtual ~Array();

	virtual void print(std::ostream&, int indent, bool debug) const override;

	virtual void analyse(SemanticAnalyser*, const Type&) override;
	void elements_will_take(SemanticAnalyser*, const std::vector<Type>&, int level);
	virtual bool will_store(SemanticAnalyser* analyser, const Type& type) override;

	virtual Compiler::value compile(Compiler&) const override;
};

}

#endif
