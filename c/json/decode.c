/*
 * yubo@yubo.org
 * see also: json-c/tests
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "json-c/json.h"

static void bar(int i, struct json_object *obj){
	struct json_object *o;
	json_object_object_get_ex(obj, "tid", &o);
	printf("\t[%d].tid=%ld\n", i, json_object_get_int64(o));
	json_object_object_get_ex(obj, "stid", &o);
	printf("\t[%d].stid=%ld\n", i, json_object_get_int64(o));
	json_object_object_get_ex(obj, "metric", &o);
	printf("\t[%d].metric=%s\n", i, json_object_get_string(o));
	json_object_object_get_ex(obj, "host", &o);
	printf("\t[%d].host=%s\n", i, json_object_get_string(o));
	json_object_object_get_ex(obj, "ip", &o);
	printf("\t[%d].ip=%s\n", i, json_object_get_string(o));
}

static void foo(void){
	int i, len;
	struct json_object *obj;

	char ctx[] = "["
		"{\"tid\":1,\"stid\":2,\"metric\":\"ping\",\"host\":\"www.mi.com\",\"ip\":\"8.8.8.8\"},"
		"{\"tid\":1,\"stid\":2,\"metric\":\"ping\",\"host\":\"www.mi.com\",\"ip\":\"1.1.1.1\"}"
		"]";
	obj = json_tokener_parse(ctx);
	for(i=0, len= json_object_array_length(obj); i < len; i++){
		bar(i, json_object_array_get_idx(obj, i));
	}

	json_object_put(obj);
}

int main(int argc, char **argv)
{
	struct json_object *new_obj;

	MC_SET_DEBUG(1);
	// I added some new lines... not in real program
	new_obj = json_tokener_parse("/* more difficult test case */ { \"glossary\": { \"title\":\"example glossary\", \"pageCount\": 100, \"GlossDiv\": { \"title\": \"S\", \"GlossList\":[ { \"ID\": \"SGML\", \"SortAs\": \"SGML\", \"GlossTerm\": \"Standard Generalized Markup Language\", \"Acronym\": \"SGML\", \"Abbrev\": \"ISO 8879:1986\", \"GlossDef\":\"A meta-markup language, used to create markup languages such as DocBook.\",\"GlossSeeAlso\": [\"GML\", \"XML\", \"markup\"] } ] } } }");

	printf("new_obj.to_string()=%s\n", json_object_to_json_string(new_obj));

	json_object_object_get_ex(new_obj, "glossary", &new_obj);
	printf("new_obj.to_string()=%s\n", json_object_to_json_string(new_obj));

	json_object_object_get_ex(new_obj, "pageCount", &new_obj);

	int pageCount = json_object_get_int(new_obj);
	printf("Page count = %d\n", pageCount);

	json_object_put(new_obj);

	foo();

	return 0;
}
