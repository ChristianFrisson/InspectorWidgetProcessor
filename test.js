//var addon = require('bindings')('addon');
var addon = require("./build/Release/InspectorWidgetProcessor"); 

var obj = new addon.InspectorWidgetProcessor();

var folder = "";
var video = "";
var msg = "";

function done (err, result) {
    if (err) {
        console.log('Error',err);
        alert(err);
        return;
    }
    else{
        console.log('Result',result);
        alert(result);
        return;
    }
}

console.log(msg);

obj.run(folder,video,msg,done)
