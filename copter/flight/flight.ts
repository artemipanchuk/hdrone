/// <reference path="../../definitions/node/node.d.ts" />

import config = require('../../config');
var controller: any = require('../../libs/controller');

var loop = setInterval(() => {
    controller.update(1, 0, 0, 0, 1);
}, 0.5);
