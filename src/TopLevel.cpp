#include <algorithm>
#include <fstream>
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>
#include <iterator>
#include <math.h>
#include <unistd.h>
#include <string>

#include "leekscript.h"
#include "../lib/utf8.h"
#include "doc/Documentation.hpp"
#include "../benchmark/Benchmark.hpp"
#include "vm/LSValue.hpp"
#include "util/Util.hpp"

using namespace std;

void print_errors(ls::VM::Result& result, std::ostream& os);
void print_result(ls::VM::Result& result, bool json, bool display_time, bool ops);

#define GREY "\033[0;90m"
#define GREEN "\033[0;32m"
#define RED "\033[1;31m"
#define END_COLOR "\033[0m"

int main(int argc, char* argv[]) {

	/** Seed random one for all */
	srand(time(0));

	/** Generate the standard functions documentation */
	if (argc > 1 && string(argv[1]) == "-doc") {
		ls::Documentation().generate(cout);
		return 0;
	}

	/** Arguments */
	bool output_json = false;
	bool display_time = false;
	bool print_version = false;
	bool debug_mode = false;
	bool v1 = false;
	bool ops = true;
	std::string file_or_code;

	for (int i = 1; i < argc; ++i) {
		auto a = string(argv[i]);
		if (a == "-j" or a == "-J" or a == "--json") output_json = true;
		else if (a == "-t" or a == "-T" or a == "--time") display_time = true;
		else if (a == "-v" or a == "-V" or a == "--version") print_version = true;
		else if (a == "-d" or a == "-D" or a == "--debug") debug_mode = true;
		else if (a == "-v1" or a == "-V1") v1 = true;
		else if (a == "-nop" or a == "--no-operations") ops = false;
		else file_or_code = a;
	}

	/** Display version? */
	if (print_version) {
		std::cout << "LeekScript 2.0" << std::endl;
		return 0;
	}

	/** Input file or code snippet? */
	if (file_or_code.size() > 0) {
		/** Get the code **/
		std::string code;
		if (Util::is_file_name(file_or_code)) {
			ifstream ifs(file_or_code.data());
			code = string((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
			ifs.close();
		} else {
			code = file_or_code;
		}
		/** Execute **/
		auto result = ls::VM(v1).execute(code, "{}", debug_mode, ops);
		print_result(result, output_json, display_time, ops);
		return 0;
	}

	/** Interactive console mode */
	cout << "~~~ LeekScript v2.0 ~~~" << endl;
	string code, ctx = "{}";
	ls::VM vm(v1);

	while (!std::cin.eof()) {
		// Get a instruction
		cout << ">> ";
		std::getline(std::cin, code);
		// Execute
		auto result = vm.execute(code, ctx, debug_mode, ops);
		print_result(result, output_json, display_time, ops);
		// Set new context
		ctx = result.context;
	}
	return 0;
}

void print_result(ls::VM::Result& result, bool json, bool display_time, bool ops) {
	if (json) {
		std::ostringstream oss;
		print_errors(result, oss);
		std::string res = oss.str() + result.value;
		res = Util::replace_all(res, "\"", "\\\"");
		cout << "{\"success\":true,\"ops\":" << result.operations
			<< ",\"time\":" << result.execution_time
			<< ",\"ctx\":" << result.context
			<< ",\"res\":\"" << res << "\"}" << endl;
	} else {
		print_errors(result, std::cout);
		if (result.execution_success && result.value != "(void)") {
			cout << result.value << endl;
		}
		if (display_time) {
			double compilation_time = round((float) result.compilation_time / 1000) / 1000;
			double execution_time = round((float) result.execution_time / 1000) / 1000;
			cout << GREY "(";
			if (ops) {
				cout << result.operations << " ops, ";
			}
			cout << compilation_time << "ms + " << execution_time << "ms)" << END_COLOR << endl;
		}
	}
}

void print_errors(ls::VM::Result& result, std::ostream& os) {
	for (const auto& e : result.lexical_errors) {
		os << "Line " << e.line << ": " << e.message() << std::endl;
	}
	for (const auto& e : result.syntaxical_errors) {
		os << "Line " << e.token->line << ": " << e.message() << std::endl;
	}
	for (const auto& e : result.semantical_errors) {
		os << "Line " << e.line << ": " << e.message() << std::endl;
	}
	if (result.exception != ls::VM::Exception::NO_EXCEPTION) {
		os << "Exception: " << ls::VM::exception_message(result.exception) << std::endl;
	}
}
