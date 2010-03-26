#ifndef NODE_AVRO_H_
#define NODE_AVRO_H_

#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>

namespace node {

class Avro : public ObjectWrap {
 public:
  static void Initialize(v8::Handle<v8::Object> target);
  static inline bool HasInstance(v8::Handle<v8::Value> val) {
    if (!val->IsObject()) return false;
    v8::Local<v8::Object> obj = val->ToObject();
    return constructor_template->HasInstance(obj);
  }

 protected:
  static v8::Persistent<v8::FunctionTemplate> constructor_template;
  static v8::Handle<v8::Value> New(const v8::Arguments &args);

 private:
  Avro(const char *schema);
  ~Avro();
};


}  // namespace node avro

#endif  // NODE_AVRO_H_
