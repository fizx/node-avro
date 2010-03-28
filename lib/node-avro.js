var AvroAddon = require("./node-avro-addon").AvroAddon, sys = require("sys");

var Avro = function(schema) {

  var _schemaObj = null, self = this;
  
  if (typeof schema === "string") {
    _schemaObj = JSON.parse(schema);
  } else {
    _schemaObj = schema;
    schema = JSON.stringify(schema);
  }

  var _avroWrapper = new AvroAddon(schema);

  // proxy event handling to _avroWrapper  
  var proxyToWrapper = function(fn) {
    return function() {
      fn.apply(_avroWrapper, arguments);
    }
  }
  self.addListener        = proxyToWrapper(_avroWrapper.addListener);
  self.removeListener     = proxyToWrapper(_avroWrapper.removeListener);
  self.removeAllListeners = proxyToWrapper(_avroWrapper.removeAllListeners);
  self.emit               = proxyToWrapper(_avroWrapper.emit);


  self.getRecordStub = function(defaults) {
    var ret = {}, i=0, len=_schemaObj.fields.length, field, defaults = defaults || {};
    for (i; i<len; i++) {
      field = _schemaObj.fields[i].name
      ret[field] = defaults[field] || null;
    }
    return ret;
  };
  
  /*
    Encode a js object into bytes
  */
  self.encode = function(jsobj) {
    jsobj = self.getRecordStub(jsobj);
    _avroWrapper.encode(jsobj);
  };
  
  /*
   Decode bytes into a js object 
  */
  self.decode = function(binary) {

  };
  

};
Avro.prototype = {};

exports.Avro = Avro;

