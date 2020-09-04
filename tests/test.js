const colorPicker = require('../build/RelWithDebInfo/color_picker.node');

function startPicking() {
    colorPicker.startColorPicker(
            function(data) {
                console.log(data);
            },
            function() {
                console.log("finished collecting color");
            },
            true,
            true,
            false
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