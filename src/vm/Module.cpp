#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Module.hpp"
#include "LSValue.hpp"
#include "value/LSClass.hpp"
#include "value/LSNumber.hpp"

using namespace std;

namespace ls {

bool Method::NATIVE = true;

Module::Module(std::string name) : name(name) {
	clazz = new LSClass(name);
}

Module::~Module() {
	//delete clazz;
}

void Module::operator_(std::string name, std::initializer_list<LSClass::Operator> impl) {
	vector<LSClass::Operator> operators = impl;
	clazz->addOperator(name, operators);
}

void Module::field(std::string name, Type type) {
	clazz->addField(name, type, nullptr);
}

void Module::field(std::string name, Type type, std::function<Compiler::value(Compiler&, Compiler::value)> fun) {
	clazz->addField(name, type, fun);
}
void Module::field(std::string name, Type type, void* fun) {
	clazz->addField(name, type, fun);
}

void Module::static_field(std::string name, Type type, std::function<Compiler::value(Compiler&)> fun) {
	static_fields.push_back(ModuleStaticField(name, type, fun));
	clazz->addStaticField(ModuleStaticField(name, type, fun));
}

void Module::method(std::string name, Method::Option opt, initializer_list<MethodConstructor> methodsConstr) {
	std::vector<Method> inst;
	std::vector<StaticMethod> st;
	for (auto constr : methodsConstr) {
		if (opt == Method::Static || opt == Method::Both) {
			st.emplace_back(constr.return_type, constr.args, constr.addr, constr.native);
		}
		if (opt == Method::Instantiate || opt == Method::Both) {
			assert(constr.args.size() > 0); // must be at least one argument to be the object used in instance
			auto obj_type = constr.args[0];
			constr.args.erase(constr.args.begin());
			inst.emplace_back(obj_type, constr.return_type, constr.args, constr.addr, constr.native);
		}
	}
	if (!inst.empty()) {
		methods.emplace_back(name, inst);
		clazz->addMethod(name, inst);
	}
	if (!st.empty()) {
		static_methods.emplace_back(name, st);
		clazz->addStaticMethod(name, st);
	}
}

void Module::generate_doc(std::ostream& os, std::string translation_file) {

	ifstream f;
	f.open(translation_file);
	if (!f.good()) {
		return; // no file
	}
	stringstream j;
	j << f.rdbuf();
	std::string str = j.str();
	f.close();

	// Erase tabs
	str.erase(std::remove(str.begin(), str.end(), '	'), str.end());

	// Parse json
	Json translation;
	try {
		translation = Json::parse(str);
	} catch (std::exception& e) { // LCOV_EXCL_LINE
		assert(false); // LCOV_EXCL_LINE
	}

	map<std::string, Json> translation_map;

	for (Json::iterator it = translation.begin(); it != translation.end(); ++it) {
		translation_map.insert({it.key(), it.value()});
	}

	os << "\"" << name << "\":{";

	os << "\"attributes\":{";
	for (unsigned e = 0; e < static_fields.size(); ++e) {

		ModuleStaticField& a = static_fields[e];

		std::string desc = (translation_map.find(a.name) != translation_map.end()) ?
				translation_map[a.name] : "";

		if (e > 0) os << ",";
		os << "\"" << a.name << "\":{\"type\":";
		a.type.toJson(os);
		//os << ",\"value\":\"" << a.value << "\"";
		os << ",\"desc\":\"" << desc << "\"";
		os << "}";
	}

	os << "},\"methods\":{";
	for (unsigned e = 0; e < methods.size(); ++e) {
		ModuleMethod& m = methods[e];

		if (e > 0) os << ",";
		os << "\"" << m.name << "\":{\"type\":";
		m.impl[0].type.toJson(os);

		if (translation_map.find(m.name) != translation_map.end()) {
			Json json = translation_map[m.name];
			std::string desc = json["desc"];
			std::string return_desc = json["return"];

			os << ",\"desc\":\"" << desc << "\"";
			os << ",\"return\":\"" << return_desc << "\"";
		}
		os << "}";
	}

	os << "},\"static_methods\":{";
		for (unsigned e = 0; e < static_methods.size(); ++e) {
			ModuleStaticMethod& m = static_methods[e];

			if (e > 0) os << ",";
			os << "\"" << m.name << "\":{\"type\":";
			m.impl[0].type.toJson(os);

			if (translation_map.find(m.name) != translation_map.end()) {
				Json json = translation_map[m.name];
				std::string desc = (json.find("desc") != json.end()) ? json["desc"] : "?";
				std::string return_desc = (json.find("return") != json.end()) ? json["return"] : "?";
				os << ",\"desc\":\"" << desc << "\"";
				os << ",\"return\":\"" << return_desc << "\"";
			}
			os << "}";
		}
	os << "}}";
}

}
