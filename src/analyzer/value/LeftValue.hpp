#ifndef LEFTVALUE_HPP_
#define LEFTVALUE_HPP_

#include "Value.hpp"

namespace ls {

class LeftValue : public Value {
public:

	LeftValue(Environment& env);
	virtual ~LeftValue() = default;

	virtual bool isLeftValue() const override;
	virtual void change_value(SemanticAnalyzer*, Value*);

	#if COMPILER
	virtual Compiler::value compile_l(Compiler&) const = 0;
	#endif
};

}

#endif
