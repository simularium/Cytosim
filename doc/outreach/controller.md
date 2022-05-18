# External Control of Cytosim

Feature first implemented on 8.3.2018.
Works reliably since SVN version 6169.

## Summary

Cytosim can be controlled from another program through a UNIX 'pipe'.
In practice, Cytosim listen to its standard-input, and executes incoming commands.

Any command can be piped to Cytosim, but it need to be on a single line!
This is not a limitation, since Cytosim's config syntax allows for this.

For example:

    change microtubule { rigidity = 45 }
    change kinesin { unloaded_speed = 1 }
    
    change all simul display { back_color = blue }
    change all hand display { size = 6 }

***ATTENTION: Piped commands should be contained in a single line!***

## Details

The input is checked by `SimThread::readInput(int n)`, which reads at most `n` lines from stdin, and executes each line sequencially using a Parser with full right. This function is called when `play` displays the state of the simulation.
It only reads 4 lines per refresh, and thus a limited capacity to get incomming commands.

## Testing

A program `test_pipe` can be used to check the functionality.
It creates a child process running cytosim, and keep control over it. `test_pipe` must be started with the path to the child command:

    > bin/test_pipe bin/play live

You can modify `test_pipe.cc' to implement your own controller.

## Application

Use a USB MIDI controller to control parameters of the simulation live.

This is useful to build an [interactive demo.](installation.md).


Work is ongoing to implement interesting controllers based on this interface.

- Implement sliders to change values of parameters.
- Read a laser-pointer to interact with a live simulation


Francois Nedelec, 19.5.2018.

