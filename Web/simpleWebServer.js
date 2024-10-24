
// A simple node.js webserver running on port 5050
// Credits to Derek Molloy's exploring beaglebone LISTING 2-2

var http = require('http'); // Require the http module
var server = http.createServer(function(req, res) {
    res.writeHead(200, {'Content-Type':'text/plain'});
    res.end('Hello from the BeagleBone Black!\n');
			      });
server.listen(5050);

console.log('BBB Web Server running at http://192.168.1.2/');