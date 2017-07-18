#include "FunctionSTD.hpp"
#include "../value/LSFunction.hpp"

namespace ls {

FunctionSTD::FunctionSTD() : Module("Function") {

	LSFunction::clazz = clazz;

	field("return", Type::CLASS);
	field("args", Type::PTR_ARRAY);
}


}
