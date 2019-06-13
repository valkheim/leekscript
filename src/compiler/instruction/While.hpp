#ifndef WHILE_HPP
#define WHILE_HPP

#include <vector>
#include "../../compiler/value/Value.hpp"
#include "../value/Block.hpp"
class Variable;

namespace ls {

class While : public Instruction {
public:

	Value* condition;
	Block* body;
	std::shared_ptr<Token> token;

	While();
	virtual ~While();

	virtual void print(std::ostream&, int indent, bool debug, bool condensed) const override;
	virtual Location location() const override;

	virtual void pre_analyze(SemanticAnalyzer* analyzer) override;
	virtual void analyze(SemanticAnalyzer*, const Type* req_type) override;

	virtual Compiler::value compile(Compiler&) const override;

	virtual Instruction* clone() const override;
};

}

#endif
