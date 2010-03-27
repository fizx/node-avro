var lib = require("../lib/node-avro"), sys = require("sys");
sys.puts("Module included.");
sys.puts(sys.inspect(lib, true));

if (!lib.Avro) {
  throw new Error("Environment Is not sane, lib.Avro does not exist");
}

var schema = "{\"type\":\"record\",\
  \"name\":\"Person\",\
  \"fields\":[\
     {\"name\": \"ID\", \"type\": \"long\"},\
     {\"name\": \"First\", \"type\": \"string\"},\
     {\"name\": \"Last\", \"type\": \"string\"},\
     {\"name\": \"Phone\", \"type\": \"string\"},\
     {\"name\": \"Age\", \"type\": \"int\"}]}";


var avro = new lib.Avro(schema);
sys.puts(sys.inspect(avro, true));
sys.puts("Avro instance created :: Environment is sane.");
