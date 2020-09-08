
# color-picker

  

Function **startColorPicker** set a callback what be called each 100ms with a color under cursor.

Clicking mouse will stop calling a callback.

  

# Usage

    colorPicker.startColorPicker(
	    function(data) {
	    console.log(data);
	    },
	    function() {
		    console.log("finished collecting color");
	    },
	    true,
		true,
	    false, 
		50
    );

startColorPicker() function has 4 params:
* callback with color picking events
* callback when color picking finished
* flag to enable callbacks with color while mouse moving 
* flag to show mini window with current color
* flag to show color hex on miniwindow
* size of color preview in a miniwindow

## Data
	
	{ event: 'mouseMove', hex: '0c0c0c' }
	{ event: 'mouseMove', hex: '0c0c0c' }
	{ event: 'mouseClick', hex: '0c0c0c' }

  

# To build

    yarn install
    
    mkdir build
    
    cd build
    
    cmake -G "Visual Studio 16 2019" -A x64 ../
    
    cmake --build . --target install --config RelWithDebInfo

  

## Test example

File **tests\test.js**

  

### To launch

**yarn electron tests\test.js**

