# Getting started with the *Command Line*

This document briefly describes how to use the [Unix Command Line](https://en.wikipedia.org/wiki/Command-line_interface) on a personal computer. 
If you are already familiar with the Command-line, you can skip this.

# Open a terminal

You will need a terminal to start Cytosim using the command line interface.

* On Mac OSX: simply open the Terminal located in Applications/Utilities.
* On Linux: search for `Terminal` in your desktop environment (varies on distribution). 
* On Windows: Always use the Cygwin terminal (and not MS-DOS prompt).

# Navigating the directory structure

Any open terminal window has a 'current working directory', which is a location of the directory structure of your hard disc. To check where youa are, type:

	pwd
	
and hit 'enter'. Another command `cd` allow you to navigate through the directories. 
If there is directory named `run` in your current directory, you can enter the following line and press 'enter':

	cd run
	
The 'current working directory' is designated with a dot ('.'), and `cd .` just make you stay in the same place! You can move to the `parent' directory with:

	cd ..

You can list the content of the current working directory with `ls`:

	ls
	
and list the content of a directory `foo` with:

	ls foo

We have used `cd` and `ls`, which are two basic commands of the `Command Line Interface`.
If you want to know more about these tools, please follow one of the many excellent tutorials available online.

# Working with directories

You can create a directory named `foo` with:

	mkdir foo

This empty directory is deleted by:

	rmdir foo
   
If the directory is not empty, you can delete it with all of its content:

	rm -r foo

We recomand running cytosim in a fresh directory, because cytosim produces a number of files, which always have the same names. Thus a new run will overwrite any older run in the same directory. Typically, we use names like `run0000`, `run0001`, etc.

# Starting programs (executables)

You should now navigate to the directory where cytosim's executable and parameter files are located.

To run a program (also called an *executable file* or just *executable*), you need to specify the full path of this file, like this:

    ./sim

If cytosim's executable `play` is present in the current directory, you can type:
 
	./play live
	
Here the dot designates the current working directory. If `play` is inside a subdirectory `bin`, you need to enter:

	bin/play live

this should open a graphical window and start a live simulation using the file
`config.cym` that is located in the current working directory.

Traditionally, one would put `play` in a subdirectory `bin` of the Home directory. In this case, you would type:

	~/bin/play live

Here the tilde designates the Home Directory. By configuring your [PATH](http://en.wikipedia.org/wiki/PATH_(variable))
to include the directory where the executable is located, you will be able to ommit the full path specification ('./'), and simply enter:
 
	play live spindle.cym

In other tutorial, only the executable name will be specified. If you get the
error "`command not found`", you should either specify the full path of the
executable, or configure the search path on your system (`$PATH`).

# Followup

You are now ready to [try the different ways of running Cytosim](runs.md).

For more info on the Unix Command Line, check the resources online:

- [Unix Command Line](https://en.wikipedia.org/wiki/Command-line_interface)
- [lifehacker](https://lifehacker.com/5633909/who-needs-a-mouse-learn-to-use-the-command-line-for-almost-anything)
- [Shell Scripting by Apple](https://developer.apple.com/library/archive/documentation/OpenSource/Conceptual/ShellScripting/shell_scripts/shell_scripts.html)
