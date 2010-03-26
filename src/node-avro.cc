#include "node-avro.h"

namespace node {

using namespace v8;

Persistent<FunctionTemplate> Avro::constructor_template;

void Avro::Initialize(v8::Handle<v8::Object> target)
{
  HandleScope scope;
  Local<FunctionTemplate> t = FunctionTemplate::New(Avro::New);
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Avro"));
  
  // export methods that need to be exposed to javascript
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "encode", Avro::encode);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "decode", Avro::decode);

  target->Set(String::NewSymbol("Avro"), constructor_template->GetFunction());

}

v8::Handle<v8::Value> Avro::encode(const v8::Arguments &args) {}
v8::Handle<v8::Value> Avro::decode(const v8::Arguments &args) {}


v8::Handle<v8::Value> Avro::New(const v8::Arguments &args)
{
}


Avro::Avro(const char *schema)
{
  // convert string schema into schema
	avro_schema_error_t error;
	if (avro_schema_from_json(schema, sizeof(schema),
				  &this->_schema, &error)) {
		//fprintf(stderr, "Unable to parse person schema\n");
		//exit(EXIT_FAILURE);
	}  
}
Avro::~Avro()
{
  
}

}
