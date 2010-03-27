#include "node-avro.h"

namespace node {

using namespace v8;

void Avro::Initialize(v8::Handle<v8::Object> target)
{
  HandleScope scope;
  Local<FunctionTemplate> t = FunctionTemplate::New(Avro::New);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(String::NewSymbol("Avro"));
  
  // export methods that need to be exposed to javascript
  NODE_SET_PROTOTYPE_METHOD(t, "encode", Avro::encode);
  NODE_SET_PROTOTYPE_METHOD(t, "decode", Avro::decode);

  target->Set(String::NewSymbol("Avro"), t->GetFunction());
}

v8::Handle<v8::Value> Avro::encode(const v8::Arguments &args) 
{
  
}

v8::Handle<v8::Value> Avro::decode(const v8::Arguments &args) 
{  
  
}

v8::Handle<v8::Value> Avro::New(const v8::Arguments &args) {
  // convert first arg to const char *
  
  // create new avro instance
  
  // wrap avro instance

  return args.This();
}

Avro::Avro(const char *schema)
{
  // convert string schema into schema
  avro_schema_error_t error;
  if (avro_schema_from_json(schema, sizeof(schema), &this->_schema, &error)) {
  }  
}
Avro::~Avro()
{
  // clean up the schema ref
}
}
