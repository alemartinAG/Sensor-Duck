var express = require('express');
var app = express();
var server = require('http').Server(app);
var io = require('socket.io')(server);

var SerialPort = require('serialport');
var serialPort = SerialPort.serialPort;

var clientSocket;

//unhandledRejection handler
process.on('unhandledRejection', error => {
  console.log(error);
});

// Open the port
var port = new SerialPort("/dev/ttyUSB0", { baudRate: 9600 });


app.use(express.static('public'));

io.on('connection', function(socket){
	
	console.log('\nClient connected to the server\n');

	//Recibo highscore
	socket.on('score-message', function(data){

		//Lo mando por el puerto serie
		try{
			port.write(data.toString());
		}
		catch(error){
			//console.error(error);
			console.log("\nNo hay conexion con serial port")
		}
	});


});

// Read the port data
port.on("open", function () {
    
    console.log('open');

	port.on('data', function(data) {

		//Lo convierto de ascii a decimal
		var dato = data.toString();
		dato = dato.charCodeAt(0);

		//console.log("dato: "+dato);

    	//Envio data por el socket como message
    	try{
    		io.sockets.emit('distancia', dato);
    	}
    	catch(error){
    		console.log(error);
    	}
    	
	});
   
});

///

app.get('/', function(req, res){
	res.status(200).send("Ale");
});

server.listen(8080, function(){
	console.log("\nSv running in http://localhost:8080\n");
});