node-avro
=========

This is an compiled (c/c++) addon for nodejs which integrates with avro-c and enables the encoding of javascript objects to a binary transport format.

see: [avro][avro]

Status
-----

 - Currently uses an export avro trunk (r924091)
 - Module is being built and require'd via tests/sanity
 
Building
-------
**Note:** this currently requires autotools and libtool
 
cd node-avro && node-waf configure build test


  [avro]: http://hadoop.apache.org/avro/[avro]: http://apache.org

