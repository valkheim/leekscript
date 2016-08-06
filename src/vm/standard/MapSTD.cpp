#include "MapSTD.hpp"
#include "../value/LSNumber.hpp"
#include "../value/LSMap.hpp"

using namespace std;

namespace ls {


MapSTD::MapSTD() : Module("Map") {

	method("size", Type::MAP, Type::INTEGER, {}, (void*) map_size);

	method("insert", {
			   {Type::PTR_PTR_MAP, Type::PTR_PTR_MAP, {Type::POINTER, Type::POINTER}, (void*) &LSMap<LSValue*,LSValue*>::ls_insert},
			   {Type::PTR_FLOAT_MAP, Type::PTR_FLOAT_MAP, {Type::POINTER, Type::FLOAT}, (void*) &LSMap<LSValue*,double>::ls_insert},
			   {Type::PTR_INT_MAP, Type::PTR_INT_MAP, {Type::POINTER, Type::INTEGER}, (void*) &LSMap<LSValue*,int>::ls_insert},
			   {Type::INT_PTR_MAP, Type::INT_PTR_MAP, {Type::INTEGER, Type::POINTER}, (void*) &LSMap<int,LSValue*>::ls_insert},
			   {Type::INT_FLOAT_MAP, Type::INT_FLOAT_MAP, {Type::INTEGER, Type::FLOAT}, (void*) &LSMap<int,double>::ls_insert},
			   {Type::INT_INT_MAP, Type::INT_INT_MAP, {Type::INTEGER, Type::INTEGER}, (void*) &LSMap<int,int>::ls_insert},
		   });


	method("clear", {
			   {Type::PTR_PTR_MAP, Type::PTR_PTR_MAP, {}, (void*) &LSMap<LSValue*,LSValue*>::ls_clear},
			   {Type::PTR_FLOAT_MAP, Type::PTR_FLOAT_MAP, {}, (void*) &LSMap<LSValue*,double>::ls_clear},
			   {Type::PTR_INT_MAP, Type::PTR_INT_MAP, {}, (void*) &LSMap<LSValue*,int>::ls_clear},
			   {Type::INT_PTR_MAP, Type::INT_PTR_MAP, {}, (void*) &LSMap<int,LSValue*>::ls_clear},
			   {Type::INT_FLOAT_MAP, Type::INT_FLOAT_MAP, {}, (void*) &LSMap<int,double>::ls_clear},
			   {Type::INT_INT_MAP, Type::INT_INT_MAP, {}, (void*) &LSMap<int,int>::ls_clear},
		   });

	method("erase", {
			   {Type::PTR_PTR_MAP, Type::PTR_PTR_MAP, {Type::POINTER}, (void*) &LSMap<LSValue*,LSValue*>::ls_erase},
			   {Type::PTR_FLOAT_MAP, Type::PTR_FLOAT_MAP, {Type::POINTER}, (void*) &LSMap<LSValue*,double>::ls_erase},
			   {Type::PTR_INT_MAP, Type::PTR_INT_MAP, {Type::POINTER}, (void*) &LSMap<LSValue*,int>::ls_erase},
			   {Type::INT_PTR_MAP, Type::INT_PTR_MAP, {Type::INTEGER}, (void*) &LSMap<int,LSValue*>::ls_erase},
			   {Type::INT_FLOAT_MAP, Type::INT_FLOAT_MAP, {Type::INTEGER}, (void*) &LSMap<int,double>::ls_erase},
			   {Type::INT_INT_MAP, Type::INT_INT_MAP, {Type::INTEGER}, (void*) &LSMap<int,int>::ls_erase},
		   });

	method("look", {
			   {Type::PTR_PTR_MAP, Type::PTR_PTR_MAP, {Type::POINTER, Type::POINTER}, (void*) &LSMap<LSValue*,LSValue*>::ls_look},
			   {Type::PTR_FLOAT_MAP, Type::PTR_FLOAT_MAP, {Type::POINTER, Type::FLOAT}, (void*) &LSMap<LSValue*,double>::ls_look},
			   {Type::PTR_INT_MAP, Type::PTR_INT_MAP, {Type::POINTER, Type::INTEGER}, (void*) &LSMap<LSValue*,int>::ls_look},
			   {Type::INT_PTR_MAP, Type::INT_PTR_MAP, {Type::INTEGER, Type::POINTER}, (void*) &LSMap<int,LSValue*>::ls_look},
			   {Type::INT_FLOAT_MAP, Type::INT_FLOAT_MAP, {Type::INTEGER, Type::FLOAT}, (void*) &LSMap<int,double>::ls_look},
			   {Type::INT_INT_MAP, Type::INT_INT_MAP, {Type::INTEGER, Type::INTEGER}, (void*) &LSMap<int,int>::ls_look},
		   });
}

int map_size(const LSMap<LSValue*,LSValue*>* map) {
	return (int) map->size();
}


}
