#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <vector>
#include "../lexical/Ident.hpp"
#include "Value.hpp"
#include "Block.hpp"

namespace ls {

class Variable;
class SemanticAnalyzer;
class FunctionVersion;

class Function : public Value {
public:

	static int id_counter;

	std::string name;
	std::string internal_name;
	bool lambda = false;
	Token* token = nullptr;
	std::vector<Token*> arguments;
	std::vector<std::unique_ptr<Value>> defaultValues;
	std::vector<bool> references;
	std::unique_ptr<Block> body;

	bool function_added;
	Function* parent;
	bool is_main_function = false;
	std::unique_ptr<FunctionVersion> default_version;
	std::map<std::vector<const Type*>, std::unique_ptr<FunctionVersion>> versions;
	bool generate_default_version = false;
	FunctionVersion* current_version = nullptr;
	bool analyzed = false;
	int default_values_count = 0;
	std::vector<Variable*> captures;
	std::unordered_map<std::string, Variable*> captures_map;
	bool captures_compiled = false;
	std::unique_ptr<Callable> callable;

	Function(Environment& env, Token* token);

	void addArgument(Token* token, Value* defaultValue, bool reference);

	virtual void print(std::ostream&, int indent, PrintOptions options) const override;
	virtual Location location() const override;

	virtual void pre_analyze(SemanticAnalyzer*) override;
	void create_default_version(SemanticAnalyzer* analyzer);
	void analyse_default_method(SemanticAnalyzer* analyzer);
	const Type* will_take(SemanticAnalyzer*, const std::vector<const Type*>&, int level) override;
	void set_version(SemanticAnalyzer* analyser, const std::vector<const Type*>& args, int level) override;
	virtual const Type* version_type(std::vector<const Type*>) const override;
	virtual void must_return_any(SemanticAnalyzer*) override;
	virtual Call get_callable(SemanticAnalyzer*, int argument_count) const override;
	void create_version(SemanticAnalyzer* analyzer, const std::vector<const Type*>& args);
	virtual void analyze(SemanticAnalyzer*) override;
	virtual Completion autocomplete(SemanticAnalyzer& analyzer, size_t position) const override;
	virtual Hover hover(SemanticAnalyzer& analyzer, size_t position) const override;
	Hover hover(SemanticAnalyzer& analyzer, File* file, size_t position) const;

	#if COMPILER
	virtual Compiler::value compile(Compiler&) const override;
	virtual Compiler::value compile_version(Compiler&, std::vector<const Type*>) const override;
	Compiler::value compile_default_version(Compiler&) const;
	void compile_captures(Compiler& c) const;
	void export_context(Compiler& c) const;
	#endif

	virtual std::unique_ptr<Value> clone(Block* parent) const override;
};

}

#endif
