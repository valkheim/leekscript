#include "Test.hpp"
#include "../src/analyzer/Context.hpp"
#include "../src/vm/value/LSString.hpp"
#include "../src/vm/value/LSArray.hpp"
#include "../src/vm/value/LSSet.hpp"
#include "../src/type/Type.hpp"

void Test::test_toplevel() {
	auto& env = getEnv();
	header("Top-level");
	ls::Context ctx { env };
	code("var a = 2").context(&ctx).works();
	test("Type of a", ctx.vars.at("a").type, env.integer);
	test("Value of a", ctx.vars.at("a").value.int_value, 2);
	test("Size of context", ctx.vars.size(), 1ul);
	code("a * 7").context(&ctx).equals("14");
	code("a + 'salut'").context(&ctx).equals("'2salut'");
	code("a").error(ls::Error::UNDEFINED_VARIABLE, {"a"});

	code("var b = 3.14").context(&ctx).works();
	test("Size of context", ctx.vars.size(), 2ul);
	test("Type of b", ctx.vars.at("b").type, env.real);
	test("Value of b", ctx.vars.at("b").value.real_value, 3.14);

	code("var c = 'salut'").context(&ctx).works();
	test("Size of context", ctx.vars.size(), 3ul);
	test("Type of c", ctx.vars.at("c").type, env.any);
	test("Value of c", *(ls::LSString*)ctx.vars.at("c").value.ls_value, ls::LSString("salut"));
	code("c + b + a").context(&ctx).equals("'salut3.142'");

	code("c += ' !'").context(&ctx).works();
	test("Size of context", ctx.vars.size(), 3ul);
	test("Type of c", ctx.vars.at("c").type, env.any);
	test("Value of c", *static_cast<ls::LSString*>(ctx.vars.at("c").value.ls_value), ls::LSString("salut !"));

	code("c = <1, 2, 3>").context(&ctx).works();
	test("Size of context", ctx.vars.size(), 3ul);
	test("Type of c", ctx.vars.at("c").type, env.any);
	test("Value of c", *static_cast<ls::LSSet<int>*>(ctx.vars.at("c").value.ls_value), ls::LSSet<int>({1, 2, 3}));

	code("c = 12").context(&ctx).works();
	test("Size of context", ctx.vars.size(), 3ul);
	test("Type of c", ctx.vars.at("c").type, env.integer);
	test("Value of c", ctx.vars.at("c").value.int_value, 12);

	code("a = [3.14, 2.71]").context(&ctx).works();
	test("Size of context", ctx.vars.size(), 3ul);
	test("Type of a", ctx.vars.at("a").type, env.any);
	test("Value of a", *static_cast<ls::LSArray<double>*>(ctx.vars.at("a").value.ls_value), ls::LSArray<double>({ 3.14, 2.71 }));
}
