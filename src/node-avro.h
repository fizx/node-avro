#ifndef NODE_AVRO_H_
#define NODE_AVRO_H_

#include <avro.h>
#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>

namespace node {

class Avro : public ObjectWrap {
 public:
  static void Initialize(v8::Handle<v8::Object> target);
  static v8::Persistent<v8::FunctionTemplate> constructor_template;
  
 protected:
  static v8::Handle<v8::Value> New(const v8::Arguments &args);
 
  // Exposed to js land
  static v8::Handle<v8::Value> Encode(const v8::Arguments &args);
  static v8::Handle<v8::Value> Decode(const v8::Arguments &args);

 private:
  Avro(const char *schema);
  ~Avro();
  
  avro_schema_t _schema;
  
};
}  // namespace node avro
#endif  // NODE_AVRO_H_
