console.log("startPicking before requeired");

var colorPicker = require('../build/RelWithDebInfo/color_picker.node');
console.log("startPicking after reqired");

function startPicking() {
    console.log("startPicking called");
    colorPicker.startColorPicker(
            function(data) {
                console.log(data);
            },
            function() {
                console.log("finished collecting color");
            },
            { onMouseMoveEnabled: true, showPreview: true, showText: false, previewSize: 35 }
        );
};

startPicking();

setTimeout(function() {
    console.log("timeout worked");
    startPicking();
}, 3000);    

setTimeout(function() {
    console.log("timeout worked");
    startPicking();
}, 13000);    