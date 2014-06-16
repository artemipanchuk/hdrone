"use strict";

import os = require('os');
import fs = require('fs');

export var totalmemory = os.totalmem() / 1024;
export function memory(): number {
    try {
        var meminfo = fs.readFileSync('/proc/meminfo', 'utf8');
    } catch(err) {
        return 0;
    }

    var freemem = os.freemem() / 1024
        + +meminfo.match(/Buffers:\s+(\d+)/)[1]
        + +meminfo.match(/Cached:\s+(\d+)/)[1];

    return 1 - freemem / this.totalmemory;
}

export function load(): number[] {
    return os.loadavg();
}

var times = os.cpus()[0].times;
export function cpu(): number {
    var prev = times, curr: typeof prev;
    times = curr = os.cpus()[0].times;
    var shot = (prev.sys - curr.sys) + (prev.user - curr.user)
             + (prev.irq - curr.irq) + (prev.nice - curr.nice);

    return shot / (shot + prev.idle - curr.idle) || 0;
}
