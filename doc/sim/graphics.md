# Cytosim's Graphics
 
Display parameters can be embedded in the config file, in the `display` string of each object.
The `display` strings are ignored by `sim`.

# Global Parameters
 
Global parameters are divided in two groups:

* [Display parameters](display.md)
* [View Parameters](view.md)
 
They are all set in the `simul` object, for example:
  
	set simul system
	{
		...
		display = ( delay=50; point_value=0.01; zoom=0.6; )
	}

or alternatively:
  
	set system display ( back_color=blue )

# Objects

Each class of object has its own display parameters (color, size, etc.),
which should be specified within the 'set' command.
Fibers use @ref FiberDispPar:

	set fiber filament
	{
	...
	display = ( line_width=3; color=blue; )
	}
 
Most objects use @ref PointDispPar:
 
	set hand motor
	{
	...
	display = ( size=3; color=green; )
	}
 
Alternative syntax:
 
	set microtubule display
	{
		line_width = 3
		color = blue
	}


# Play setup file

Display parameters can also be specified in a separate file with extension `.cyp`.
Such setup file should be specified on the command line:

	play display.cyp live

It will be read before the simulation starts, and thus any value can be changed
while cytosim reads `config.cym`.

To control at which time the display parameters are changed, the best technique is to 
`include` the setup in the main config file:

	include display.cyp { required=0 }

The result will be equivalent to having the specifications directly in the config file, 
except that cytosim will continue even if the setup file is not found (because 'required=0').
In this way, you can safely share a single display file for multiple simulations.
 
# Color specifications
 
A color is composed of 4 components (Red, Green, Blue, Alpha), where the alpha value
specifies transparency. Colors can be specified in different ways:
 
Syntax                    | Example      | Note                                                  |
--------------------------|--------------|--------------------------------------------------------
`0xRRGGBBAA` `0xRRGGBB`   | `0xFF0000FF` | Each component specified with 2 hexadecimal character
`(r g b a)` `(r g b)`     | `(1 0 0.5)`  | Components are specified in floating points
`COLOR_NAME`              | `red`        | Cytosim understand most common color names
`#INTEGER`                | `#12`        | This corresponds to an index in the list of colors
 
Examples:

	color = blue

Opaque red:
	
	color = 0xFF0000FF

Transparent blue:
	
	color = 0x0000FF88


