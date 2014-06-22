"use strict";

import assert  = require('assert');
import child   = require('child_process');

import transport = require('../../common/transport');
import protocol  = require('../../common/protocol');
import os        = require('../../libs/os');

import config = require('../../config');

var stateContact = new transport.UdpTransport({
    port    : config.STATE_PORT,
    timeout : config.STATE_TIMEOUT
});

stateContact.on('connect', () => {
    var encoder = new protocol.Encoder;
    encoder.pipe(stateContact);

    process.on('message', onstate);
    function onstate(state: number[]) {
        encoder.encode({
            memory : os.memory(),
            load   : os.load(),
            cpu    : os.cpu()
        });
    }

    stateContact.once('close', () => {
        process.removeListener('message', onstate);
    });
});

var videoContact = new transport.TcpTransport({
    port    : config.VIDEO_PORT,
    timeout : config.VIDEO_TIMEOUT
});
