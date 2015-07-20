'use strict';

/*
 * Client library for official 2048 javascript version
 *
 * Place inside 2048/js directory
 * Add this to the end of the scripts in index.html: <script src="js/socket.js"></script>
 * Launch socket server (see socket-server.sh)
 * Open/refresh page to perform initial socket connection
 */

var HOST = 'localhost:3001';

var requestMove = function(socket) {
	if (window.gameManager.isGameTerminated()) {
		if (window.gameManager.won) {
			//GameManager.prototype.keepPlaying.call(window.gameManager);
			setTimeout(function() {
				document.querySelector('.keep-playing-button').click();
				requestMove();
			}, 3000);
		}

		return;
	}

	socket.emit('find_best_move', window.gameManager.grid);
};

var hook_game = function() {
	if (typeof GameManager == 'undefined') {
		return setTimeout(hook_game, 100);
	}

	window.gameManager = new GameManager(4, KeyboardInputManager, HTMLActuator, LocalStorageManager);
	var move = window.gameManager.move;
	window.gameManager.move = function() {
		move.apply(window.gameManager, arguments);
		requestMove();
	}
	socket_ai();
};

var socket_ai = function() {
	var socket = io('ws://' + HOST);
	requestMove = requestMove.bind(null, socket);

	socket.on('move', function(best) {
		window.gameManager.move([0, 2, 3, 1][best]);
	});

	socket.on('ready', requestMove);
};

// <script src="http://localhost:3001/socket.io/socket.io.js" onload="hook_game();"></script>
var s = document.createElement('script');
s.onload = hook_game;
s.src = 'http://' + HOST + '/socket.io/socket.io.js'
document.body.appendChild(s);
