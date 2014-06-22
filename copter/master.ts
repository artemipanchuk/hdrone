/// <reference path="../definitions/node/node.d.ts" />
"use strict";

import cluster = require('cluster');
import assert  = require('assert');
import os      = require('os');

import config  = require('../config');

var timeouts: {[key: string]: any} = {};
var roles: {[key: string]: string} = {};

function timeout(role: string) {
    return (function() {
        console.log('[ERROR] failed to start worker '+role+'. Better check environment. Terminating.');
        process.exit(1);
    });
}

function fork(role: string) {
    var worker = cluster.fork();

    worker.on('message', (message: any) => {
        if(message.role) {
            var role = <string> message.role;

            clearTimeout(timeouts[role]);

            console.log('[REPORT] '+role+' is up.')
        }
    });

    worker.send({role : role});

    roles[worker.id] = role;
    timeouts[role]   = setTimeout(timeout(role), 5000);
}

if(cluster.isMaster) {
    process.title = 'hdrone';
    
    cluster.on('exit', (worker: cluster.Worker, code: number, signal: number) => {
        var role = roles[worker.id];

        clearTimeout(timeouts[role]);
        if(worker.suicide === true)
            console.log('[REPORT] suicide of '+role+'.');
        else {
            console.log('[WARNING] '+role+' died. Reforking.'),
            fork(role);
        }
    });

    ['contact', 'flight', 'vision'].forEach(fork);

} else if(cluster.isWorker) {
    process.on('message', (message: any) => {
        if(message.role) {
            var role = <string> message.role;

            require('./'+role+'/'+role);

            process.send({role : role});
        }
    });
}
