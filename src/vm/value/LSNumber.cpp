#include "LSNumber.hpp"
#include "LSNull.hpp"
#include "LSFunction.hpp"
#include <math.h>
#include "LSBoolean.hpp"

using namespace std;

namespace ls {

LSClass* LSNumber::number_class = new LSClass("Number");

LSNumber* LSNumber::cache[CACHE_HIGH - CACHE_LOW + 1];

void LSNumber::build_cache() {
	for (int i = CACHE_LOW; i <= CACHE_HIGH; ++i) {
		cache[-CACHE_LOW + i] = new LSNumber(i);
	}
}

LSNumber* LSNumber::get(NUMBER_TYPE i) {
#if USE_CACHE
	if ((i == (int) i) and i >= CACHE_LOW and i <= CACHE_HIGH) {
		return cache[(int) (-CACHE_LOW + i)];
	}
#endif
//	std::cout << "Number() " << i << endl;
	return new LSNumber(i);
}

LSNumber::LSNumber() : value(0) {}

LSNumber::LSNumber(NUMBER_TYPE value) : value(value) {}

LSNumber::LSNumber(Json& json) : value(json) {}

LSNumber::~LSNumber() {
//	cout << "delete LSNumber : " << value << endl;
}

bool LSNumber::isTrue() const {
	return value != 0;
}

LSValue* LSNumber::operator - () const {
	return LSNumber::get(-value);
}

LSValue* LSNumber::operator ! () const {
	return LSBoolean::get(value == 0);
}

LSValue* LSNumber::operator ~ () const {
	return LSNumber::get(~(int)value);
}

LSValue* LSNumber::operator ++ () {
#if !USE_CACHE
	++value;
#endif
	return this;
}
LSValue* LSNumber::operator ++ (int) {
	NUMBER_TYPE old = value;
#if !USE_CACHE
	++value;
#endif
	return LSNumber::get(old);
}

LSValue* LSNumber::operator -- () {
#if !USE_CACHE
	--value;
#endif
	return this;
}
LSValue* LSNumber::operator -- (int) {
	NUMBER_TYPE old = value;
#if !USE_CACHE
	--value;
#endif
	return LSNumber::get(old);
}

LSValue* LSNumber::ls_radd(LSValue* value) {
	return value->ls_add(this);
}
LSValue* LSNumber::ls_add(LSNull*) {
	return this;
}
LSValue* LSNumber::ls_add(LSBoolean* boolean) {
	if (boolean->value) {
		if (refs == 0) {
			this->value += 1;
			return this;
		}
		return LSNumber::get(value + 1);
	}
	return this;
}
LSValue* LSNumber::ls_add(LSString* string) {
	LSValue* r = new LSString(toString() + *string);
	if (refs == 0) delete this;
	if (string->refs == 0) delete string;
	return r;
}
LSValue* LSNumber::ls_add(LSNumber* number) {
	if (refs == 0) {
		value += number->value;
		return this;
	}
	if (number->refs == 0) {
		number->value += value;
		return number;
	}
	return LSNumber::get(this->value + number->value);
}

LSValue* LSNumber::operator += (LSValue* value) {
	return value->operator += (this);
}
LSValue* LSNumber::operator += (const LSNull*) {
	return LSNull::get();
}
LSValue* LSNumber::operator += (const LSNumber* number) {
#if !USE_CACHE
	value += number->value;
#endif
//	this->refs++;
	return this;
}
LSValue* LSNumber::operator += (const LSBoolean* boolean) {
#if !USE_CACHE
	value -= boolean->value;
#endif
//	this->refs++;
	return this;
}
LSValue* LSNumber::operator += (const LSString*) {
	return LSNull::get();
}

LSValue* LSNumber::operator - (const LSValue* value) const {
	return value->operator - (this);
}
LSValue* LSNumber::operator - (const LSNull*) const {
	return LSNull::get();
}
LSValue* LSNumber::operator - (const LSBoolean* boolean) const {
	return LSNumber::get(this->value - boolean->value);
}
LSValue* LSNumber::operator - (const LSNumber* number) const {
	return LSNumber::get(this->value - number->value);
}
LSValue* LSNumber::operator - (const LSString* value) const {
	return new LSString(*value + this->toString());
}


LSValue* LSNumber::operator -= (LSValue* value) {
	return value->operator -= (this);
}
LSValue* LSNumber::operator -= (const LSNull*) {
	return LSNull::get();
}
LSValue* LSNumber::operator -= (const LSBoolean*) {
	return LSNull::get();
}
LSValue* LSNumber::operator -= (const LSNumber* number) {
#if !USE_CACHE
	value -= number->value;
#endif
//	this->refs++;
	return this;
}
LSValue* LSNumber::operator -= (const LSString*) {
	return LSNull::get();
}

LSValue* LSNumber::operator * (const LSValue* value) const {
	return value->operator * (this);
}
LSValue* LSNumber::operator * (const LSNull*) const {
	return LSNull::get();
}
LSValue* LSNumber::operator * (const LSBoolean* boolean) const {
	return LSNumber::get(this->value * boolean->value);
}
LSValue* LSNumber::operator * (const LSNumber* number) const {
	return LSNumber::get(this->value * number->value);
}
LSValue* LSNumber::operator * (const LSString* value) const {
	return value->operator *(this);
}

LSValue* LSNumber::operator *= (LSValue* value) {
	return value->operator *= (this);
}
LSValue* LSNumber::operator *= (const LSNull*) {
#if !USE_CACHE
	value = 0;
#endif
//	this->refs++;
	return this;
}
LSValue* LSNumber::operator *= (const LSBoolean*) {
	return LSNull::get();
}
LSValue* LSNumber::operator *= (const LSNumber* number) {
#if !USE_CACHE
	value *= number->value;
#endif
//	this->refs++;
	return this;
}
LSValue* LSNumber::operator *= (const LSString*) {
	return LSNull::get();
}

LSValue* LSNumber::operator / (const LSValue* value) const {
	return value->operator / (this);
}
LSValue* LSNumber::operator / (const LSNull*) const {
	return LSNull::get();
}
LSValue* LSNumber::operator / (const LSBoolean*) const {
	return LSNull::get();
}
LSValue* LSNumber::operator / (const LSNumber* number) const {
	return LSNumber::get(this->value / number->value);
}
LSValue* LSNumber::operator / (const LSString*) const {
	return LSNull::get();
}

LSValue* LSNumber::operator /= (LSValue* value) {
	return value->operator /= (this);
}
LSValue* LSNumber::operator /= (const LSNull*) {
	return this->clone();
}
LSValue* LSNumber::operator /= (const LSBoolean*) {
	return this->clone();
}
LSValue* LSNumber::operator /= (const LSNumber* number) {
#if !USE_CACHE
	value /= number->value;
#endif
//	this->refs++;
	return this;
}
LSValue* LSNumber::operator /= (const LSString*) {
	return LSNull::get();
}

LSValue* LSNumber::poww(const LSValue* value) const {
	return value->poww(this);
}
LSValue* LSNumber::poww(const LSNull*) const {
	return LSNull::get();
}
LSValue* LSNumber::poww(const LSBoolean*) const {
	return LSNull::get();
}
LSValue* LSNumber::poww(const LSNumber* value) const {
	return LSNumber::get((NUMBER_TYPE) pow(this->value, value->value));
}
LSValue* LSNumber::poww(const LSString*) const {
	return LSNull::get();
}

LSValue* LSNumber::pow_eq(LSValue*) {
	return LSNull::get();
}
LSValue* LSNumber::pow_eq(const LSNull*) {
	return LSNull::get();
}
LSValue* LSNumber::pow_eq(const LSBoolean*) {
	return LSNull::get();
}
LSValue* LSNumber::pow_eq(const LSNumber* number) {
#if !USE_CACHE
	value = pow(value, number->value);
#endif
	return LSNull::get();
}
LSValue* LSNumber::pow_eq(const LSString*) {
	return LSNull::get();
}

LSValue* LSNumber::operator % (const LSValue* value) const {
	return value->operator % (this);
}
LSValue* LSNumber::operator % (const LSNull*) const {
	return LSNull::get();
}
LSValue* LSNumber::operator % (const LSBoolean*) const {
	return LSNull::get();
}
LSValue* LSNumber::operator % (const LSNumber* value) const {
	return LSNumber::get(fmod(this->value, value->value));
}
LSValue* LSNumber::operator % (const LSString*) const {
	return LSNull::get();
}

LSValue* LSNumber::operator %= (LSValue* value) {
	return value->operator %= (this);
}
LSValue* LSNumber::operator %= (const LSNull*) {
	return LSNull::get();
}
LSValue* LSNumber::operator %= (const LSBoolean*) {
	return LSNull::get();
}
LSValue* LSNumber::operator %= (const LSNumber* number) {
#if !USE_CACHE
	value = fmod(value, number->value);
#endif
//	this->refs++;
	return this;
}
LSValue* LSNumber::operator %= (const LSString*) {
	return LSNull::get();
}

bool LSNumber::operator == (const LSValue* v) const {
	return v->operator == (this);
}
bool LSNumber::operator == (const LSNumber* v) const {
	return this->value == v->value;
}

bool LSNumber::operator < (const LSValue* v) const {
	return v->operator < (this);
}
bool LSNumber::operator < (const LSNumber* v) const {
	return this->value < v->value;
}

bool LSNumber::operator > (const LSValue* v) const {
	return v->operator > (this);
}
bool LSNumber::operator > (const LSNumber* v) const {
	return this->value > v->value;
}

LSValue* LSNumber::at(const LSValue*) const {
	return LSNull::get();
}

LSValue** LSNumber::atL(const LSValue*) {
	return nullptr;
}

LSValue* LSNumber::attr(const LSValue* key) const {
	if (*((LSString*) key) == "class") {
		return getClass();
	}
	return LSNull::get();
}
LSValue** LSNumber::attrL(const LSValue*) {
	return nullptr;
}

LSValue* LSNumber::abso() const {
	return LSNumber::get(abs((int) value));
}

LSValue* LSNumber::clone() const {
	return LSNumber::get(this->value);
}

bool LSNumber::isInteger() const {
	return value == (int)value;
}

void append_dbl2str(std::string &s, double d) {

	size_t len = snprintf(0, 0, "%.18f", d);
	size_t oldsize = s.size();
	s.resize(oldsize + len + 1);

	// technically non-portable
	snprintf(&s[oldsize], len+1, "%.18f", d);
	// remove nul terminator
	s.pop_back();
	// remove trailing zeros
	s.erase(s.find_last_not_of('0') + 1, string::npos);
	// remove trailing point
	if (s.back() == L'.') {
		s.pop_back();
	}
}

string LSNumber::toString() const {
	if (isInteger()) return to_string((int)value);
	string s;
	append_dbl2str(s, value);
	return s;
}
string LSNumber::json() const {
	return toString();
}

std::ostream& LSNumber::print(std::ostream& os) const {
	os << toString();
	return os;
}

LSValue* LSNumber::getClass() const {
	return LSNumber::number_class;
}

int LSNumber::typeID() const {
	return 3;
}

const BaseRawType* LSNumber::getRawType() const {
	return RawType::INTEGER;
}

}
