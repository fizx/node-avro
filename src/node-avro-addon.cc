#include "node-avro-addon.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace v8;
namespace node {

static Persistent<String> data_symbol;
static Persistent<String> end_symbol;

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void AvroAddon::Initialize(Handle<Object> target)
{
  HandleScope scope;
  Local<FunctionTemplate> t = FunctionTemplate::New(New);
  
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(String::NewSymbol("AvroAddon"));
  t->Inherit(EventEmitter::constructor_template);
  
  // export methods that need to be exposed to javascript
  NODE_SET_PROTOTYPE_METHOD(t, "encode", Encode);
  NODE_SET_PROTOTYPE_METHOD(t, "decode", Decode);
  
  // setup event symbols
  data_symbol = NODE_PSYMBOL("data");
  end_symbol = NODE_PSYMBOL("end");
  
  target->Set(String::NewSymbol("AvroAddon"), t->GetFunction());
}

Handle<Value> AvroAddon::Encode(const Arguments &args) 
{
 /* 
  avro_datum_t person = avro_record("Person", NULL);

	avro_datum_t id_datum = avro_int64(++id);
	avro_datum_t first_datum = avro_string(first);
	avro_datum_t last_datum = avro_string(last);
	avro_datum_t age_datum = avro_int32(age);
	avro_datum_t phone_datum = avro_string(phone);

	if (avro_record_set(person, "ID", id_datum)
	    || avro_record_set(person, "First", first_datum)
	    || avro_record_set(person, "Last", last_datum)
	    || avro_record_set(person, "Age", age_datum)
	    || avro_record_set(person, "Phone", phone_datum)) {
		fprintf(stderr, "Unable to create Person datum structure");
		exit(EXIT_FAILURE);
	}

	if (avro_file_writer_append(db, person)) {
		fprintf(stderr,
			"Unable to write Person datum to memory buffer");
		exit(EXIT_FAILURE);
	}

	
	avro_datum_decref(id_datum);
	avro_datum_decref(first_datum);
	avro_datum_decref(last_datum);
	avro_datum_decref(age_datum);
	avro_datum_decref(phone_datum);
	avro_datum_decref(person);
  
  */
  
  /*
  This is probably the best way to get data out to javascript as the 
  avro_memory_writer takes a length param and from what I can tell there is
  no way to get the length of what will be written to the buffer before hand.
  
  So, using next tick and a set length, this should be close to streaming.
  */
  /*
  AvroAddon *instance = ObjectWrap::Unwrap<AvroAddon>(args.This());
  HandleScope scope;
  Handle<Value>test = String::New("test");
  instance->Emit(String::NewSymbol("data"), 1, &test);*/
  return args.This();
}

Handle<Value> AvroAddon::Decode(const Arguments &args) 
{  
  return args.This();  
}

Handle<Value> AvroAddon::New(const Arguments &args) {
  
  // convert first arg to const char *
  String::Utf8Value str(args[0]);
  const char* cstr = ToCString(str);
  
  // create new avro instance (process incoming schema)
  AvroAddon *avro = new AvroAddon(cstr);
  
  // wrap avro instance in the v8 object
  avro->Wrap(args.This());

  return args.This();
}

AvroAddon::AvroAddon(const char *schema) : EventEmitter()
{
  // convert string schema into schema
  avro_schema_error_t error;
  if (avro_schema_from_json(schema, sizeof(schema), &this->_schema, &error)) {
    ThrowException(Exception::Error(String::New("Invalid Schema.")));
  
//		return false;
  }

  V8::AdjustAmountOfExternalAllocatedMemory(sizeof(AvroAddon));
//  return true;
}

AvroAddon::~AvroAddon()
{
  // clean up the schema ref
  avro_schema_decref(this->_schema);
}

} // namespace node
