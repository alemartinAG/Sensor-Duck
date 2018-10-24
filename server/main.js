var express = require('express');
var app = express();
var server = require('http').Server(app);
var io = require('socket.io')(server);

var SerialPort = require('serialport');
var serialPort = SerialPort.serialPort;

//unhandledRejection handler
process.on('unhandledRejection', error => {
  console.log("Problemas de conexion serie (no se pudo encontrar el puerto)");
});

// Open the port
var port = new SerialPort("/dev/ttyUSB1", { baudRate: 9600 });


app.use(express.static('public'));

// Read the port data
port.on("open", function () {
    console.log('open');

    try{
    	port.on('data', function(data) {
        	console.log(data);
    	});
    }
    catch(UnhandledPromiseRejectionWarning){
    	console.log("\nNo se pudo leer el puerto");
    }
});

///

app.get('/', function(req, res){
	res.status(200).send("Ale");
});

io.on('connection', function(socket){
	console.log('\nClient connected to the server\n');

	socket.on('score-message', function(data){
		//console.log(data.first);

		socket.emit('messages', {
			value : 3
		});

		try{
			port.write(data.first);
		}
		catch(error){
			//console.error(error);
			console.log("\nNo hay conexion con serial port")
		}
	});
});


server.listen(8080, function(){
	console.log("\nSv running in http://localhost:8080\n");
});

