var socket = io.connect('http://localhost:8080', {'forceNew' : true});
var hand_position = 10;

var canvas = document.getElementById("canvas");

var context = canvas.getContext("2d");
context.font = 'bold 25pt Arial';

///
var bird = new Image();
var bird_2 = new Image();
var background = new Image();
var pipe_up = new Image();
var pipe_dw = new Image();

bird.src = "images/bird.png";
bird_2.src = "images/bird2.png";
background.src = "images/background.png";
pipe_up.src = "images/obstaculo.png";
pipe_dw.src = "images/obstaculo2.png";
///

///
var scorebg = new Image();
scorebg.src = "images/score.png";
///

///
var quack = new Audio();
quack.src = "sounds/quack.mp3";

var song = new Audio();
song.src = "sounds/song.mp3";
///

///
var pausa = 1;
var pausa_on = new Audio();
var pausa_off = new Audio();
pausa_on.src = "sounds/pausa.mp3";
pausa_off.src = "sounds/pausaoff.wav";
///

///
var x_pos = 100;
var y_pos = 10;
var scale = 0.9;
var seccion = 186;

var gravity = 2;
var velocity = 3;

var gap = seccion;
var constant;
///

var pipe = [];

pipe[0] = {
    x : canvas.width,
    y : (-10 - pipe_up.height) + seccion * Math.floor(Math.random()*5),
    flag_n : 0,
    flag_s : 0
};

var score = 0;
var highscores = [0, 0, 0];

socket.on('messages', function(data){
    //console.log(data);
    hand_position = 10 + seccion * data;
});

document.addEventListener("keydown", press);
document.addEventListener("keyup", release);

alert("Play");

draw();

function press(event){

    //const keyName = event.key;
    let keyName = event.key;

    if(keyName == 'p'){
        pausa = !pausa;

        if(pausa){
            pausa_off.play();
        }else{
            pausa_on.play();
        }

    }

    if(keyName == 'a'){
        y_pos -= seccion*pausa;
    }

    if(keyName == 's'){
        y_pos += seccion*pausa;
    }

    if(y_pos < 0){
        y_pos = 10;
    }

    if(y_pos > canvas.height-bird.height*scale){
        //y_pos = canvas.height-bird.height*scale;
        y_pos = 754;
    }

    bird.src = "images/bird2.png";
}

function release(){
    bird.src = "images/bird.png";
}

function draw(){

    context.drawImage(background, 0, 0);
    context.drawImage(bird, x_pos, y_pos, bird.width*scale, bird.height*scale);

    //song.play();

    for(var i=0; i<pipe.length; i++){

        constant = pipe_up.height+gap;

        context.drawImage(pipe_dw, pipe[i].x, pipe[i].y);
        context.drawImage(pipe_up, pipe[i].x, pipe[i].y+constant);

        pipe[i].x -= 5*velocity*pausa;

        if( pipe[i].x <= canvas.width/3 && !pipe[i].flag_n){

            pipe[i].flag_n = 1;

            pipe.push({
                x : canvas.width,
                //y : Math.floor(Math.random()*pipe_up.height) - pipe_up.height,
                y: (-10 - pipe_up.height) + seccion * Math.floor(Math.random()*5),
                flag_n : 0,
                flag_s : 0
            });
        }

        incrementScore(i);

        checkCollision(i);
    }

    context.fillText("score: "+score,5,30);

    //Tabla de puntajes
    if(!pausa){
        context.drawImage(scorebg, canvas.width/2-scorebg.width, canvas.height/2-scorebg.height/2);
        for(let j=highscores.length-1; j>=0; j--){
            context.fillText(highscores[(j-2)*(-1)], canvas.width/2-scorebg.width/2-10-10*(highscores[(j-2)*(-1)] > 10), (canvas.height/2) + 20 + 45*j );
        }
    }
    else{
        y_pos = hand_position;
    }
    

    requestAnimationFrame(draw);
}

function checkCollision(i){

    if(x_pos + (bird.width*scale) >= pipe[i].x && x_pos <= pipe[i].x + pipe_up.width && (y_pos <= pipe[i].y + pipe_up.height || y_pos+ (bird.height*scale) >= pipe[i].y+constant)){

        for(var j=pipe.length-1; j>0; j--){
            pipe.pop();
        }

        pipe[0] = {
            x : canvas.width,
            y : (-10 - pipe_up.height) + seccion * Math.floor(Math.random()*5),
            flag_n : 0,
            flag_s : 0
        };

        compareScores();
        sortScores();

        score = 0;

        x_pos = 100;
        y_pos = 10;


        //envio los puntajes
        var payload = {
        	first : highscores[2],
        	second : highscores[1],
        	third : highscores[0]
        };

        socket.emit('score-message', payload);
    }
}

function incrementScore(i){

    if(pipe[i].x <= 5 && !pipe[i].flag_s){
        score++;
        pipe[i].flag_s = 1;

        quack.play();
    }
}

function sortScores(){

    var aux;

    for(var j=0; j<highscores.length-1; j++){
        if(highscores[j] > highscores[j+1]){
            aux = highscores[j];
            highscores[j] = highscores[j+1];
            highscores[j+1] = aux;
        }
    }
}

function compareScores(){
    for(var j=0; j<highscores.length; j++){
        if(score > highscores[j]){
            highscores[j] = score;
            break;
        }
    }
}