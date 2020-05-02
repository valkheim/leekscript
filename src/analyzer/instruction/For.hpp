#ifndef FOR_HPP
#define FOR_HPP

#include <vector>
#include "../value/Value.hpp"
#include "../value/Block.hpp"
#include "Instruction.hpp"
#include "../semantic/Mutation.hpp"

namespace ls {

class Block;
class Variable;

class For : public Instruction {
public:

	Token* token;
	std::unique_ptr<Block> init;
	std::unique_ptr<Block> condition;
	std::unique_ptr<Block> increment;
	std::unique_ptr<Block> body;
	std::vector<Mutation> mutations;

	For(Environment& env);

	virtual void print(std::ostream&, int indent, PrintOptions options) const override;
	virtual Location location() const override;

	virtual void pre_analyze(SemanticAnalyzer* analyzer) override;
	virtual void analyze(SemanticAnalyzer*, const Type* req_type) override;
	virtual Hover hover(SemanticAnalyzer& analyzer, size_t position) const override;

	#if COMPILER
	virtual Compiler::value compile(Compiler&) const override;
	#endif

	virtual std::unique_ptr<Instruction> clone(Block* parent) const override;
};

}

#endif
