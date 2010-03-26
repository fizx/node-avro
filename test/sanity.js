var avro = require("../lib/node-avro"), sys = require("sys");
sys.puts("Module included.");


sys.puts(sys.inspect(new avro.Avro("asdf"), true));

sys.puts("Avro instance created :: Environment is sane.");
