
# color-picker

Function **startColorPicker** set a callback what be called each 100ms with a color under cursor.

Click a mouse button to finish picking color.

  

# Usage

    colorPicker.startColorPicker(
	    function(data) {
	    console.log(data);
	    },
	    function() {
		    console.log("finished collecting color");
	    },
	    { onMouseMoveEnabled: true, showPreview: true, showText: false, previewSize: 35 }
    );

startColorPicker() function params:
* callback with color picking events
* callback when color picking finished
* params:
** flag to enable callbacks with color while mouse moving 
** flag to show mini window with current color
** flag to show color hex on miniwindow
** size of color preview in a miniwindow


## Data
	
	{ event: 'mouseMove', hex: '0c0c0c' }
	{ event: 'mouseMove', hex: '0c0c0c' }
	{ event: 'mouseClick', hex: '0c0c0c' }

  

# To build

    yarn install
    
    yarn local:config
    
    yarn local:build 

    yarn local:test 
  

## Usage code example

File **tests\test.js**

  

### Tests
Launch electron and call to start color picking. Call repeated 3 times at 0, 3, 10 seconds. 

**yarn test** 

