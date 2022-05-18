# Cytosim's Configuration File

The virtual system and the sequence of actions to be performed is specified in a configuration file. 
This config file must be a plain (ASCII) text file. By default cytosim will look for **`config.cym`**, but any file ending with `.cym` should work.

Many examples can be found in the directory [***cym/***](../../cym).

Cytosim understands a [small set of commands](commands.md) and has a set of [predefined objects](objects.md) with [associated parameters](parameters.md).

The fundamental commands are:

 - `set` to define a new object category, and set its parameter values,
 - `new` to create objects,
 - `run` to perform simulation steps. 

Curly brackets `{  }` after each command are used to group the parameters associated with the command.
 
About the config file:

- Parameter names are predefined in the simulations for each [object class](objects.md)
- Parameters are specified with a ` = ` sign, and can contain multiple values
- Parameter values should be specified in [units of `(seconds, micrometers, picoNewtons)`](units.md)
- A parameter assignment is terminated by the end of a line, or by a semi-column: `;`
- Parentheses `(  )` can be used for subgroups, such as `display`
- Strings can be encapsulated by double quotes `" "` or parentheses if they include spaces
- A line of comment should start with `%`, and longer comments should be enclosed by `%{ }`.

# Editing

Cytosim's configuration files `*.cym` are plain text ASCII file. Avoid word processors such as Microsoft Word, and prefer [PLAIN TEXT editors](https://en.wikipedia.org/wiki/Text_editor), for example:

- [TextMate](https://macromates.com)
- [Atom](https://atom.io)
- [Gleany](https://www.geany.org)
- [Sublime text](https://www.sublimetext.com)

Manuel Lera-Ramirez made a [syntax highlighting configuration](../misc/Cytosim.tmbundle.zip) for TextMate.

# Example

	% Self organization of Microtubules driven by bivalent Motors
	% Adapted from Nedelec et al. Nature, 1998
	
	set simul system
	{
		time_step = 0.01
		viscosity = 0.05
	}
	
	set space cell
	{
		shape = circle
	}
	
	new cell
	{   
		radius = 10
	}

	set fiber microtubule
	{
		rigidity = 20
		segmentation = 0.5
		confine = inside, 100
		display = ( line_width=1; color=white; )
	}
	
	set hand kinesin
	{
		binding = 10, 0.01   % rate, range
		unbinding = 0.1, 3   % rate, force
		
		activity = move
		unloaded_speed = 0.8
		stall_force = 5
		
		display = ( size=7; color=green; )
	}
	
	set couple complex
	{
		hand1 = kinesin
		hand2 = kinesin
		stiffness = 100
		diffusion = 10
	}
	
	new 100 microtubule
	{
		length = 9
	}
	
	new 2000 complex
	
	run 5000 system
	{
		nb_frames = 50
	}
	
