#include "PrefixExpression.hpp"
#include "FunctionCall.hpp"
#include "LeftValue.hpp"
#include "VariableValue.hpp"
#include "../semantic/SemanticAnalyzer.hpp"
#include "../error/Error.hpp"

namespace ls {

PrefixExpression::PrefixExpression(Environment& env, std::shared_ptr<Operator> op, std::unique_ptr<Value> expression)
	: Value(env), operatorr(op), expression(std::move(expression)) {
		assert(this->expression.get());
}

void PrefixExpression::print(std::ostream& os, int indent, PrintOptions options) const {
	operatorr->print(os);
	if (operatorr->type == TokenType::NEW) {
		os << " ";
	}
	expression->print(os, indent, options);
	if (options.debug) {
		os << " " << type;
	}
}

Location PrefixExpression::location() const {
	return { operatorr->token->location.file, operatorr->token->location.start, expression->location().end };
}

void PrefixExpression::pre_analyze(SemanticAnalyzer* analyzer) {
	// std::cout << "PrefixExpression pre_analyze" << std::endl;
	expression->pre_analyze(analyzer);
}

void PrefixExpression::analyze(SemanticAnalyzer* analyzer) {
	const auto& env = analyzer->env;

	expression->analyze(analyzer);
	throws = expression->throws;

	if (operatorr->type == TokenType::TILDE) {
		if (expression->type->is_function()) {
			analyzer->add_error({ Error::Type::NO_SUCH_OPERATOR, ErrorLevel::ERROR, location(), location(), { "~", expression->to_string() } });
		}
		type = expression->type;
		throws |= expression->type->is_polymorphic();
		if (type->is_bool()) {
			type = env.integer;
		}
	}

	if (operatorr->type == TokenType::PLUS_PLUS
		or operatorr->type == TokenType::MINUS_MINUS
		or operatorr->type == TokenType::MINUS or operatorr->type == TokenType::AROBASE) {

		type = expression->type;
		throws |= expression->type->fold()->is_polymorphic();
		if (operatorr->type == TokenType::PLUS_PLUS or operatorr->type == TokenType::MINUS_MINUS) {
			if (expression->type->constant) {
				analyzer->add_error({Error::Type::CANT_MODIFY_CONSTANT_VALUE, ErrorLevel::ERROR, location(), expression->location(), {expression->to_string()}});
			}
			if (not expression->isLeftValue()) {
				analyzer->add_error({Error::Type::VALUE_MUST_BE_A_LVALUE, ErrorLevel::ERROR, location(), expression->location(), {expression->to_string()}});
			}
		}

	} else if (operatorr->type == TokenType::NOT) {

		type = env.boolean;
		throws |= expression->type->is_polymorphic();

	} else if (operatorr->type == TokenType::NEW) {

		type = env.tmp_any;
		if (VariableValue* vv = dynamic_cast<VariableValue*>(expression.get())) {
			if (vv->name == "Number") type = env.integer;
			else if (vv->name == "Boolean") type = env.boolean;
			else if (vv->name == "String") type = env.tmp_string;
			else if (vv->name == "Array") type = Type::tmp_array(env.never);
			else if (vv->name == "Object") type = env.tmp_object;
			else if (vv->name == "Set") type = Type::tmp_set(env.any);
		}
		else if (FunctionCall* fc = dynamic_cast<FunctionCall*>(expression.get())) {
			if (VariableValue* vv = dynamic_cast<VariableValue*>(fc->function.get())) {
				if (vv->name == "Number") {
					if (fc->arguments.size() > 0) {
						type = fc->arguments[0]->type;
					} else {
						type = env.integer;
					}
				}
				else if (vv->name == "Boolean") type = env.boolean;
				else if (vv->name == "String") type = env.tmp_string;
				else if (vv->name == "Array") type = Type::tmp_array(env.never);
				else if (vv->name == "Object") type = env.tmp_object;
				else if (vv->name == "Set") type = Type::tmp_set(env.any);
			}
		}
	}
	if (is_void) {
		type = env.void_;
	}
}

Hover PrefixExpression::hover(SemanticAnalyzer& analyzer, size_t position) const {
	if (expression->location().contains(position)) {
		return expression->hover(analyzer, position);
	}
	return { type, location() };
}

#if COMPILER
Compiler::value PrefixExpression::compile(Compiler& c) const {

	c.mark_offset(location().start.line);

	switch (operatorr->type) {
		case TokenType::PLUS_PLUS: {
			if (expression->type->is_mpz_ptr()) {
				auto x = ((LeftValue*) expression.get())->compile_l(c);
				auto one = c.new_integer(1);
				c.insn_call(c.env.void_, {x, x, one}, "Number.mpz_add_ui");
				return is_void ? Compiler::value(c.env) : c.insn_clone_mpz(x);
			} else if (expression->type->is_primitive()) {
				auto x_addr = ((LeftValue*) expression.get())->compile_l(c);
				auto x = c.insn_load(x_addr);
				auto sum = c.insn_add(x, c.new_integer(1));
				c.insn_store(x_addr, sum);
				return sum;
			} else {
				auto arg = expression->compile(c);
				return c.insn_invoke(type, {arg}, "Value.pre_inc");
			}
		}
		case TokenType::MINUS_MINUS: {
			if (expression->type->is_primitive()) {
				auto x_addr = static_cast<LeftValue*>(expression.get())->compile_l(c);
				auto x = c.insn_load(x_addr);
				auto sum = c.insn_sub(x, c.new_integer(1));
				c.insn_store(x_addr, sum);
				return sum;
			} else {
				auto arg = expression->compile(c);
				return c.insn_invoke(type, {arg}, "Value.pre_dec");
			}
		}
		case TokenType::NOT: {
			if (expression->type->is_primitive()) {
				// std::cout << "PreExpr compile expr " << std::endl;
				auto x = expression->compile(c);
				assert(x.t->llvm(c) == x.v->getType());
				return c.insn_not_bool(x);
			} else {
				auto arg = expression->compile(c);
				return c.insn_invoke(type, {arg}, "Value.not");
			}
		}
		case TokenType::MINUS: {
			if (expression->type->is_mpz_ptr()) {
				auto x = expression->compile(c);
				auto r = x.t->temporary ? x : c.new_mpz();
				c.insn_call(c.env.void_, {r, x}, "Number.mpz_neg");
				return r;
			} else if (expression->type->is_primitive()) {
				auto x = expression->compile(c);
				return c.insn_neg(x);
			} else {
				auto arg = expression->compile(c);
				return c.insn_invoke(type, {arg}, "Value.minus");
			}
		}
		case TokenType::TILDE: {
			if (expression->type->is_primitive()) {
				auto x = expression->compile(c);
				return c.insn_not(x);
			} else {
				auto arg = expression->compile(c);
				return c.insn_invoke(type, {arg}, "Value.pre_tilde");
			}
		}
		case TokenType::NEW: {
			if (VariableValue* vv = dynamic_cast<VariableValue*>(expression.get())) {
				if (vv->name == "Number") {
					return c.new_integer(0);
				}
				else if (vv->name == "Boolean") {
					return c.new_bool(0);
				}
				else if (vv->name == "String") {
					return c.insn_call(c.env.tmp_string, {}, "String.new");
				}
				else if (vv->name == "Array") {
					return c.new_array(c.env.void_, {});
				}
				else if (vv->name == "Object") {
					return c.new_object();
				}
				else if (vv->name == "Set") {
					return c.new_set();
				}
			}
			if (FunctionCall* fc = dynamic_cast<FunctionCall*>(expression.get())) {
				if (VariableValue* vv = dynamic_cast<VariableValue*>(fc->function.get())) {
					if (vv->name == "Number") {
						if (fc->arguments.size() > 0) {
							return fc->arguments[0]->compile(c);
						} else {
							return c.new_integer(0);
						}
					}
					if (vv->name == "Boolean") {
						return c.new_bool(false);
					}
					if (vv->name == "String") {
						if (fc->arguments.size() > 0) {
							return fc->arguments[0]->compile(c);
						}
						return c.insn_call(c.env.tmp_string, {}, "String.new");
					}
					if (vv->name == "Array") {
						return c.new_array(c.env.void_, {});
					}
					if (vv->name == "Object") {
						return c.new_object();
					}
					if (vv->name == "Set") {
						return c.new_set();
					}
				}
				// new A(), convert to new A
				auto clazz = fc->function->compile(c);
				return c.new_object_class(clazz);
			}
			// By default, compile the class expression and construct a LSObject.
			// new A
			auto clazz = expression->compile(c);
			return c.new_object_class(clazz);
		}
		default: {
			assert(false);
		}
	}
}
#endif

std::unique_ptr<Value> PrefixExpression::clone(Block* parent) const {
	return std::make_unique<PrefixExpression>(type->env, operatorr, expression->clone(parent));
}

}
