#ifndef NODE_AVRO_ADDON_H_
#define NODE_AVRO_ADDON_H_

#include <avro.h>
#include <node.h>
#include <node_events.h>
#include <node_object_wrap.h>
#include <v8.h>

namespace node {
using namespace v8;

class AvroAddon : public EventEmitter {
 public:
  static void Initialize(Handle<Object> target);
  
 protected:
  static Handle<Value> New(const Arguments &args);
 
  // Exposed to js land
  static Handle<Value> Encode(const Arguments &args);
  static Handle<Value> Decode(const Arguments &args);

 private:
  AvroAddon(const char *schema);
  ~AvroAddon();
  
  avro_schema_t _schema;
  
};
}  // namespace node avro
#endif  // NODE_AVRO_ADDON_H_
