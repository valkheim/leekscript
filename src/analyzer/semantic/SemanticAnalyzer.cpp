#include "SemanticAnalyzer.hpp"
#include "../instruction/ExpressionInstruction.hpp"
#include "../Program.hpp"
#include "../Context.hpp"
#include "../error/Error.hpp"
#include "../instruction/VariableDeclaration.hpp"
#include "../instruction/While.hpp"
#include "../instruction/For.hpp"
#include "../instruction/Foreach.hpp"
#include "../../standard/Module.hpp"
#include <functional>
#include "Variable.hpp"
#include "FunctionVersion.hpp"
#include "../../colors.h"

namespace ls {

SemanticAnalyzer::SemanticAnalyzer(Environment& env) : env(env) {
	program = nullptr;
}

void SemanticAnalyzer::analyze(Program* program) {

	this->program = program;
	for (const auto& clazz : env.std.classes) {
		auto const_class = env.const_class(clazz.first);
		program->globals.emplace(clazz.first, new Variable(clazz.first, nullptr, VarScope::INTERNAL, const_class, 0, nullptr, nullptr, nullptr, nullptr, clazz.second->clazz.get()));
	}

	program->main->create_default_version(this);
	enter_function(program->main->default_version.get());
	enter_block(program->main->default_version->body.get());

	enter_section(program->main->default_version->body->sections.front().get());

	// Add context variables
	if (program->context) {
		for (auto& var : program->context->vars) {
			// std::cout << "Add context var " << var.first << std::endl;
			var.second.token = std::make_unique<Token>(TokenType::IDENT, program->main_file, 0, 0, 0, var.first);
			var.second.variable = add_var(var.second.token.get(), var.second.type, nullptr);
			var.second.variable->injected = true;
			// std::cout << "variable added " << var.second.variable << " " << (void*) var.second.variable << std::endl;
		}
	}
	leave_section();
	leave_block();
	leave_function();

	program->main->analyze(this);
	program->functions = functions;
}

void SemanticAnalyzer::enter_function(FunctionVersion* f) {
	// std::cout << "enter function" << std::endl;
	blocks.push_back({});
	sections.push_back({});
	loops.push_back({});
	functions_stack.push_back(f);
}

void SemanticAnalyzer::leave_function() {
	// std::cout << "leave function" << std::endl;
	blocks.pop_back();
	sections.pop_back();
	functions_stack.pop_back();
	loops.pop_back();
}

void SemanticAnalyzer::enter_block(Block* block) {
	// std::cout << "enter block" << std::endl;
	blocks.back().push_back(block);
	if (sections.back().size() && current_section()->successors.size() == 1) {
		leave_section(); // If there's a current section, close it
	}
}

void SemanticAnalyzer::leave_block() {
	// std::cout << "leave block" << std::endl;
	blocks.back().pop_back();
}

void SemanticAnalyzer::enter_section(Section* section) {
	// std::cout << "enter section" << std::endl;
	sections.back().push_back(section);
}

void SemanticAnalyzer::leave_section() {
	// std::cout << "leave section" << std::endl;
	sections.back().back()->analyze_end(this);
	sections.back().pop_back();
}

FunctionVersion* SemanticAnalyzer::current_function() const {
	return functions_stack.back();
}
Block* SemanticAnalyzer::current_block() const {
	return blocks.back().back();
}
Section* SemanticAnalyzer::current_section() const {
	assert(sections.back().size());
	return sections.back().back();
}
Instruction* SemanticAnalyzer::current_loop() const {
	return loops.back().back();
}

void SemanticAnalyzer::enter_loop(Instruction* loop) {
	loops.back().push_back(loop);
}

void SemanticAnalyzer::leave_loop() {
	loops.back().pop_back();
}

bool SemanticAnalyzer::in_loop(int deepness) const {
	return loops.back().size() >= deepness;
}

Variable* SemanticAnalyzer::get_var(const std::string& v) {

	// std::cout << "SemanticAnalyzer::get_var " << v << std::endl;

	// Search in global variables
	auto i = program->globals.find(v);
	if (i != program->globals.end()) {
		return i->second.get();
	}

	// Search operators
	if (auto op = program->get_operator(v)) {
		return op;
	}

	// Search recursively in the functions
	int f = functions_stack.size() - 1;
	while (f >= 0) {
		// Search in the local variables of the function
		int b = blocks[f].size() - 1;
		if (b >= 0) {
			auto start_block = blocks[f][b];
			// std::cout << "start block " << start_block->sections.front()->id << std::endl;
			if (sections[f].size()) {
				auto section = sections[f].back();
				while (section != nullptr) {
					// std::cout << "search " << v << " in section " << section->color << section->id << END_COLOR << std::endl;
					auto i = section->variables.find(v);
					if (i != section->variables.end()) {
						auto root = i->second->root ? i->second->root : i->second;
						// std::cout << "root " << root << " " << root->block->sections.front()->id << std::endl;
						if (root->block == start_block) {
							// The root variable's block is matching current block so it's the correct variable
							// std::cout << "OK " << i->second << std::endl;
							return i->second;
						} else {
							// Variable found in section, but the root variable is not matching, it's a shadowed variable
							if (b == 0) {
								section = section->predecessors.size() ? section->predecessors[0] : nullptr;
							} else {
								start_block = blocks[f][--b];
								// std::cout << "start block " << start_block->	sections.front()->id << std::endl;
							}
						}
					} else {
						// std::cout << "not found in " << section->color << section->id << END_COLOR << std::endl;
						section = section->predecessors.size() ? section->predecessors[0] : nullptr;
					}
				}
			}
			// std::cout << "NOT FOUND " << std::endl;
		}

		// Search in the function parameters
		const auto& arguments = functions_stack[f]->arguments;
		auto i = arguments.find(v);
		if (i != arguments.end()) {
			return i->second;
		}

		// const auto& fvars = blocks[f];
		// int b = fvars.size() - 1;
		// while (b >= 0) {
		// 	int s = fvars[b]->sections.size() - 1;
		// 	while (s >= 0) {
		// 		const auto& vars = fvars[b]->sections[s]->variables;
		// 		// std::cout << "Section [" << fvars[b]->sections[s]->id << "] variables : ";
		// 		// for (const auto& v : vars) std::cout << v.first << " " << v.second << ", ";
		// 		// std::cout << std::endl;
		// 		auto i = vars.find(v);
		// 		if (i != vars.end()) {
		// 			return i->second;
		// 		}
		// 		s--;
		// 	}
		// 	b--;
		// }
		f--;
	}
	return nullptr;
}

Variable* SemanticAnalyzer::add_var(Token* v, const Type* type, Value* value) {
	if (program->globals.find(v->content) != program->globals.end()) {
		add_error({Error::Type::VARIABLE_ALREADY_DEFINED, ErrorLevel::ERROR, v->location, v->location, {v->content}});
		return nullptr;
	}
	const auto& block = blocks.back().back();
	if (block->variables.find(v->content) != block->variables.end()) {
		add_error({Error::Type::VARIABLE_ALREADY_DEFINED, ErrorLevel::ERROR, v->location, v->location, {v->content}});
		return nullptr;
	}

	auto var = new Variable(v->content, v, VarScope::LOCAL, type, 0, value, current_function(), current_block(), current_section(), nullptr);
	block->variables.emplace(v->content, var);
	assert(current_section());
	current_section()->variables.emplace(v->content, var);
	current_section()->variable_list.emplace_back(var);
	// std::cout << "var " << v->content << " added in " << block->sections.back()->id << std::endl;

	return var;
}

Variable* SemanticAnalyzer::add_var(Token* v, Variable* var) {
	if (program->globals.find(v->content) != program->globals.end()) {
		add_error({Error::Type::VARIABLE_ALREADY_DEFINED, ErrorLevel::ERROR, v->location, v->location, {v->content}});
		return nullptr;
	}
	const auto& block = blocks.back().back();
	if (block->variables.find(v->content) != block->variables.end()) {
		add_error({Error::Type::VARIABLE_ALREADY_DEFINED, ErrorLevel::ERROR, v->location, v->location, {v->content}});
		return nullptr;
	}
	var->function = current_function();
	var->block = current_block();
	var->section = current_section();

	block->variables.insert({ v->content, var });
	assert(current_section());
	current_section()->variables[v->content] = var;
	// std::cout << "var " << v->content << " added in " << block->sections.back()->id << std::endl;

	return var;
}

Variable* SemanticAnalyzer::add_global_var(Token* v, const Type* type, Value* value) {
	// std::cout << "blocks " << blocks.size() << std::endl;
	for (const auto& section : blocks.begin()->front()->sections) {
		auto& vars = section->variables;
		if (vars.find(v->content) != vars.end()) {
			add_error({Error::Type::VARIABLE_ALREADY_DEFINED, ErrorLevel::ERROR, v->location, v->location, {v->content}});
			return nullptr;
		}
	}
	auto& section = blocks.begin()->front()->sections.front();
	auto& var = section->variable_list.emplace_back(new Variable(v->content, v, VarScope::LOCAL, type, 0, value, current_function(), current_block(), current_section(), nullptr, {}, true));
	section->variables.emplace(v->content, var.get());
	return var.get();
}

void SemanticAnalyzer::add_function(Function* l) {
	functions.push_back(l);
}

Variable* SemanticAnalyzer::update_var(Variable* variable, bool add_mutation) {
	if (not variable) return nullptr;
	if (variable->loop_variable) return variable;
	// std::cout << "update_var " << variable << " " << (int) variable->scope << std::endl;
	Variable* new_variable;
	if (current_block() == variable->block) {
		// std::cout << "same block" << std::endl;
		/* Same block */
		// var a = 12
		// a.1 = 5.5
		// a.2 = 'salut'
		auto root = variable->root ? variable->root : variable;
		new_variable = new Variable(root->name, variable->token, variable->scope, env.void_, root->index, nullptr, current_function(), current_block(), current_section(), nullptr);
		new_variable->id = variable->id + 1;
		new_variable->root = root;
	} else {
		// std::cout << "branch" << std::endl;
		/* Branch */
		// var a = 12
		// a.1 = 5.5
		// if (...) {
		//    a.1.1 = 'salut'
		// }
		auto root = variable->root ? variable->root : variable;
		new_variable = new Variable(variable->name, variable->token, variable->scope, env.void_, variable->index, nullptr, current_function(), current_block(), current_section(), nullptr);
		new_variable->id = variable->id + 1;
		new_variable->root = root;
	}
	new_variable->parent = variable;
	// new_variable->injected = variable->injected;

	// if (variable->scope == VarScope::PARAMETER) {
	// 	// current_function()->arguments[new_variable->name] = new_variable;
	// 	current_section()->variables[new_variable->name] = new_variable;
	// } else {
		current_section()->variables[new_variable->name] = new_variable;
	// }
	current_section()->variable_list.emplace_back(new_variable);

	// Ajout d'une mutation
	if (in_loop(1) && add_mutation) {
		auto loop = current_loop();
		if (auto w = dynamic_cast<While*>(loop)) {
			w->mutations.push_back({ new_variable, new_variable->section });
		} else if (auto f = dynamic_cast<For*>(loop)) {
			f->mutations.push_back({ new_variable, new_variable->section });
		} else if (auto f = dynamic_cast<Foreach*>(loop)) {
			f->mutations.push_back({ new_variable, new_variable->section });
		}
	}
	return new_variable;
}

void SemanticAnalyzer::add_error(Error ex) {
	// ex.underline_code = program->underline_code(ex.location, ex.focus);
	errors.push_back(ex);
}

} // end of namespace ls
