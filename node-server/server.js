#!/usr/bin/env node

'use strict';

/*
 * Why node? Already established websocket features via socket.io
 * and FF 'remote control' slows to a crawl after spamming moves,
 * plus this can work with any browser that supports websockets
 */


var ffi = require('ffi'),
	ref = require('ref'),
	UINT64 = require('cuint').UINT64;

var libai = ffi.Library('bin/libai', {
	find_best_move: [ref.types.int, [ref.types.uint64]],
	print_grid: [ref.types.void, [ref.types.uint64]],
	score_move: [ref.types.float, [ref.types.uint64, ref.types.int]],
	create_new_grid: [ref.types.uint64, []],
	init_lookup_tables: [ref.types.void, []]
});
console.log('Initialising lookup tables');
libai.init_lookup_tables();

console.log('Socket.IO binding');
var io = require('socket.io').listen(3001);
console.log('Ready for client');

var print_grid = function(grid) {
	var x, y;
	for (x = 0; x < 4; x++) {
		console.log(grid[x].join(' '));
	}
};

var to_c_grid = function(grid) {
	var c_grid = UINT64(0);

	var x, y, val, i = 0;
	for (x = 0; x < 4; x++) {
		for (y = 0; y < 4; y++) {
			if (grid[x][y] === 0) {
				val = UINT64(0);
			} else {
				val = UINT64(Math.round(Math.log(grid[x][y]) / Math.log(2)));
			}
			c_grid = c_grid.or(val.shiftLeft(4 * i));
			i++;
		}
	}

	return c_grid.toString();
}

var findBestMoveSync = function(socket, raw_grid) {
	var best = libai.find_best_move(to_c_grid(process_grid(raw_grid.cells)));
	if (best !== -1) {
		socket.emit('move', best);
	} else {
		console.error('No more moves');
	}
};

var findBestMoveAsync = function(socket, raw_grid) {
	var scores = [0, 0, 0, 0];
	var i = 4;
	var c_grid = to_c_grid(process_grid(raw_grid.cells));

	var done = function() {
		if (--i === 0) {
			var bestScore = 0,
				bestDir = -1;

			scores.forEach(function(score, dir) {
				if (score > bestScore) {
					bestScore = score;
					bestDir = dir;
                } else if (score === bestScore && bestScore > 0) {
                    // coin flip
                    if (Math.random() < 0.5) {
                        bestDir = dir;
                    }
                }
			});
			console.log(bestDir);

			if (bestDir !== -1) {
				socket.emit('move', bestDir);
			}
		}
	};

	scores.map(function(_, dir) {
		libai.score_move.async(c_grid, dir, function(err, score) {
			if (err) {
				throw err;
			}
			scores[dir] = score;
			done();
		});
	});
};

var process_grid = function(grid) {
	var x, y;
	var new_grid = [[], [], [], []];

	for (y = 0; y < 4; y++) {
		for (x = 0; x < 4; x++) {
			if (!grid[y][x]) {
				new_grid[x][y] = 0;
			} else {
				new_grid[x][y] = grid[y][x].value;
			}
		}
	}

	return new_grid;
};

io.on('connection', function(socket) {
	console.log('Client connected');

	socket.on('disconnect', function() {
		console.log('Client disconnected');
	});

	//socket.on('find_best_move', findBestMoveSync.bind(null, socket));
	socket.on('find_best_move', findBestMoveAsync.bind(null, socket));

	socket.emit('ready');
});