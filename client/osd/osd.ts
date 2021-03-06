"use strict";

import CompassWidget = require('./compass_widget');
import HorizonWidget = require('./horizon_widget');
import CPUMEMWidget  = require('./cpumem_widget');
import LoadWidget    = require('./load_widget');
import TempWidget    = require('./temp_widget');

import Protocol = require('../../common/protocol');

var atan2 = Math.atan2,
    asin  = Math.asin;
function quaternionToEuler(quaternion: number[]) {
    var q1 = quaternion[0],
        q2 = quaternion[1],
        q3 = quaternion[2],
        q4 = quaternion[3];

    var yaw   =  atan2(2*(q2*q3 - q1*q4), 2*(q1*q1 + q2*q2) - 1),
        pitch =  -asin(2*(q2*q4 + q1*q3)),
        roll  =  atan2(2*(q3*q4 - q1*q2), 2*(q1*q1 + q4*q4) - 1);

    return [yaw, pitch, roll];
}

class OSD {
    private _compass: CompassWidget;
    private _horizon: HorizonWidget;
    private _cpumem:  CPUMEMWidget;
    private _load:    LoadWidget;
    private _temp:    TempWidget;

    constructor(elements: {
                    compass: HTMLCanvasElement;
                    horizon: HTMLCanvasElement;
                    cpumem:  HTMLCanvasElement;
                    load:    HTMLCanvasElement;
                    temp:    HTMLCanvasElement
    }) {
        this._compass = new CompassWidget(elements.compass);
        this._horizon = new HorizonWidget(elements.horizon);
        this._cpumem  = new CPUMEMWidget(elements.cpumem);
        this._load    = new LoadWidget(elements.load);
        this._temp    = new TempWidget(elements.temp);
    }

    public update(data: Protocol.IState) {
        var angles = quaternionToEuler(data.attitude);

        window.requestAnimationFrame(() => {
            this._compass.update(angles[0]);
            this._horizon.update(angles[1], angles[2]);
        });
           
        this._cpumem.update(data.cpu, data.memory);
        this._load.update(data.load);
        this._temp.update(data.temperatures);
    }

}

export = OSD;
