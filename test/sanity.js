var Avro = require("../lib/node-avro").Avro, sys = require("sys");
sys.puts("Module included.");

if (!Avro) {
  throw new Error("Environment Is not sane, lib.Avro does not exist");
}

var schema = {
  type   : "record",
  name   : "Person",
  fields : [
     {name: "ID",    type: "long"},
     {name: "First", type: "string"},
     {name: "Last",  type: "string"},
     {name: "Phone", type: "string"},
     {name: "Age",   type: "int"}
  ]
};

// invalid schema's cause exceptions
var caught = false;
try {
  new Avro({});
} catch (e) {
  caught = true;
}

if (!caught) {
  throw new Error("Invalid schema's should make the constructor throw..");
}

var avro = new Avro(schema);
avro.addListener("data", function(data) {
  sys.puts("Data: " + data);
});


if (!avro.encode || !avro.decode) {
  throw new Error("Encode/Decode method(s) are non-existant!");
}

sys.puts("Avro instance created (Schema parsed)");

// encode an object
var orig = {ID: 1, First:"Tmp", Last:"Var", Phone:"123", Age: 1};
var result = avro.encode(orig);
sys.puts("Original object encoded");

// decode the resulting binary
var decoded = avro.decode(result);
sys.puts("Encoded object was decoded");

// compare the two objects
// TODO: better check
if (JSON.stringify(orig) !== JSON.stringify(decoded)) {
  throw new Error("Decoded object is not equal to the original");
}
sys.puts("Original and decoded are the same in json land");

sys.puts("### Environment is sane. ###");


