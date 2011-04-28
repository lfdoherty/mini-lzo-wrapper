
var lzo = require('mini-lzo-wrapper');

var sys = require('sys');

var uncompBuffer = new Buffer("this is some text, this is some more text, this is some text, this is some more text, this is some text, this is some more text");
var compBuffer = new Buffer(uncompBuffer.length * 2);//this is overkill

sys.debug('compressing ' + uncompBuffer.length + ' bytes.');
sys.debug('target buffer has ' + compBuffer.length + ' bytes.');

var len = lzo.compress(uncompBuffer, compBuffer);

sys.debug('compress happened: ' + len + ' < ' + uncompBuffer.length + ' < ' + compBuffer.length);
