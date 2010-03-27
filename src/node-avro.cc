#include "node-avro.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

namespace node {

using namespace v8;

Persistent<FunctionTemplate> Avro::constructor_template;

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void Avro::Initialize(v8::Handle<v8::Object> target)
{
  HandleScope scope;
  Local<FunctionTemplate> _constructor = FunctionTemplate::New(New);
  constructor_template = Persistent<FunctionTemplate>::New(_constructor);
  
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Avro"));
  
  // export methods that need to be exposed to javascript
  Local<FunctionTemplate> _encode = FunctionTemplate::New(Encode);
  constructor_template->PrototypeTemplate()->Set(String::NewSymbol("encode"), _encode);
  //NODE_SET_PROTOTYPE_METHOD(t, "encode", Encode);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "decode", Decode);
  
  target->Set(String::NewSymbol("Avro"), constructor_template->GetFunction());
}

v8::Handle<v8::Value> Avro::Encode(const v8::Arguments &args) 
{
  return Undefined();
}

v8::Handle<v8::Value> Avro::Decode(const v8::Arguments &args) 
{  
  return Undefined();  
}

v8::Handle<v8::Value> Avro::New(const v8::Arguments &args) {
  
  // convert first arg to const char *
  v8::String::Utf8Value str(args[0]);
  const char* cstr = ToCString(str);
  
  // create new avro instance (process incoming schema)
  Avro *avro = new Avro(cstr);
  
  // wrap avro instance in the v8 object
  avro->Wrap(args.This());

  return args.This();
}

Avro::Avro(const char *schema)
{
  // convert string schema into schema
  avro_schema_error_t error;
  if (avro_schema_from_json(schema, sizeof(schema), &this->_schema, &error)) {
		fprintf(stderr, "Unable to parse person schema\n");
		exit(EXIT_FAILURE);  
  }
  V8::AdjustAmountOfExternalAllocatedMemory(sizeof(Avro));
}

Avro::~Avro()
{
  // clean up the schema ref
  avro_schema_decref(this->_schema);
}

} // namespace node
