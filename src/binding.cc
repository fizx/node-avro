#include "node-avro.h"

using namespace v8;
using namespace node;

extern "C" void
init (Handle<Object> target) 
{
  HandleScope scope;
  Avro::Initialize(target);
}
