const colorPicker = require('../build/RelWithDebInfo/color_picker.node');

function startPicking() {
    colorPicker.startColorPicker(
            function(data) {
                console.log(data);
            },
            function() {
                console.log("finished collecting color");
            }
        );
};

startPicking();

setTimeout(function() {
    console.log("timeout worked");
    //startPicking();
}, 10000);    