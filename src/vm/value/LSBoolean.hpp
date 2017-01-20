#ifndef LSBOOLEAN_HPP_
#define LSBOOLEAN_HPP_

#include <string>
#include "../../../lib/json.hpp"
#include "../LSValue.hpp"
#include "../Type.hpp"

namespace ls {

class LSBoolean : public LSValue {
private:

	LSBoolean(bool value);

public:

	const bool value;

	static LSValue* boolean_class;
	static LSBoolean* false_val;
	static LSBoolean* true_val;
	static LSBoolean* get(bool value) {
		return value ? true_val : false_val;
	}

	LSBoolean(Json& data);

	virtual ~LSBoolean();

	bool isTrue() const override;

	LSValue* ls_not() override;
	LSValue* ls_tilde() override;

	LSValue* add(LSValue* v) override;
	LSValue* sub(LSValue* v) override;
	LSVALUE_OPERATORS

	bool eq(const LSBoolean*) const override;
	bool lt(const LSBoolean*) const override;

	LSValue* at (const LSValue* value) const override;
	LSValue** atL (const LSValue* value) override;

	LSValue* clone() const;

	std::ostream& dump(std::ostream& os) const;
	std::string json() const override;

	LSValue* getClass() const override;

	int typeID() const override { return 2; }
};

}

#endif
