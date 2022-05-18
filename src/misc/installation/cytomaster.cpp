/**
 Cytosim control using a USB-connected MIDI board, with several scenarios
 For "Nuit Blanche" in Paris, 6.10.2018
 Updated for "Plant festival" in Cambridge 7.05.2019
 Gaelle LETORT and Francois NEDELEC

 @todo: get feedback from Cytosim when a command fails
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <vector>

// Using Real-Time MIDI by Gary Scavone:
#include "RtMidi.h"

typedef unsigned char byte;
typedef std::vector<byte> Message;

/// debug mode
unsigned mode = 0;

/// which scenario is running [0 ... 8]
unsigned config = 0;

/// child process id:
pid_t child = 0;

/// file descriptor for a pipe: [0]=READ, [1]=WRITE
int fds[2] = { 0 };

/// size of strings
const size_t LEN = 1024;

/// path to current working directory
char cwd[LEN] = { 0 };


/// Unix signal handler
void handler(int sig)
{
    psignal(sig, "Cytomaster");
    // _exit(sig);
}


/// stop current child process
void stop(int)
{
    close(fds[1]);
    kill(child, SIGTERM);
    fds[0] = 0;
    fds[1] = 0;
    child = 0;
}

/// start child process executing specified command
void start(const char* path, char *const args[])
{
    // create a unidirectional pipe:
    if ( pipe(fds) < 0 )
    {
        perror("pipe");
        exit(1);
    }
    
    // create a child process
    child = fork();
    
    if ( child == -1 )
    {
        perror("fork");
        exit(1);
    }
    
    if ( child == 0 )
    {
        // this code executed by the child process
        // child closes pipe exit:
        close(fds[1]);
        // map the standard-input to the pipe exit:
        while ((dup2(fds[0], STDIN_FILENO) == -1) && (errno == EINTR)) {}
        // run executable (this should not return, except if error occurred)
        if ( mode )
        {
            write(STDERR_FILENO, " > ", 3);
            write(STDERR_FILENO, path, strlen(path));
            write(STDERR_FILENO, "\n", 1);
        }
        execv(path, args);
        // the command failed, check what is indicated by 'errno':
        perror("Error: could not start child process");
        write(STDERR_FILENO, " > ", 3);
        write(STDERR_FILENO, path, strlen(path));
        for ( int i = 1; args[i]; ++i )
        {
            write(STDERR_FILENO, " ", 1);
            write(STDERR_FILENO, args[i], strlen(args[i]));
        }
        write(STDERR_FILENO, "\n", 1);
        // end child execution here:
        _exit(1);
    }
    
    // this code executed by the parent process
    // close parent entry:
    close(fds[0]);
    //printf("fds %i tty %i\n", fds[1], isatty(fds[1]));
}


/// start cytosim child process with a pipe
void start(unsigned conf)
{
    char path[LEN] = { 0 };
    char mem[5*LEN] = { 0 };
    char* args[5] = { mem, mem+LEN, mem+2*LEN, mem+3*LEN, mem+4*LEN };
    
    snprintf(args[0], LEN, "play%i", conf);
    snprintf(args[1], LEN, "config%i.cym", conf);
    snprintf(args[2], LEN, "live");
    snprintf(args[3], LEN, "fullscreen=1");
    args[4] = nullptr;

    // build full path to executable:
    snprintf(path, LEN, "%s/%s", cwd, args[0]);
    
    if ( mode )
        printf("    > %s %s %s %s\n", args[0], args[1], args[2], args[3]);
    
    start(path, args);
    config = conf;
}


/// stop and start new process
void restart(unsigned arg)
{
    stop(0);
    start(arg);
}

//------------------------------------------------------------------------------
#pragma mark -

/// Converts MIDI value (from 0 to 127) to [min, max] range, linear dependency
float linear(float val, float min, float max)
{
    return min + ( max - min ) * val;
}

/// Converts MIDI value (from 0 to 127) to [min, max] range, 2^n dependency
float quadratic(float val, float min, float max)
{
    return ( min + ( max - min ) * val * val );
}

/// Converts MIDI value (from 0 to 127) to [min, max] range, 2^n dependency
float geometric(float val, float max)
{
    int b = 8 * sizeof(unsigned long) - 1;
    unsigned long i = 1 << (int)roundf(val*b);
    unsigned long u = 1 << (int)roundf(b);
    printf("geometric %lu : %f\n", i, (float)i/(float)u);
    return max * (float)i/(float)u;
}

/// Converts X in [1, 8] to [-max, max] with geometric variation
float srange(int X, float max)
{
    int a, s;
    if ( X > 4 ) { a = 1 << (8-X); s = 1; }
    else         { a = 1 << (X-1); s = -1; }
    return max / (float)(s*a);
}

/// Converts X in [1, 8] to [0, max] with geometric variation
float prange(int X, float max)
{
    int a = 1 << (8-X);
    return max / (float)(a);
}


/// for the Novation Control XL
int makeCommandXL(char * str, size_t len, int slider, int value)
{
    float v = (float)value / 127.0; // map value to [0, 1]
    switch ( slider )
    {
        case 1:    case 95:
            return snprintf(str, len, "change system { viscosity=%.3f }\n", linear(v, 0.1, 1.0));
        case 2:    case 96:
            return snprintf(str, len, "change filament { rigidity=%.3f }\n", geometric(v, 20));
        case 3:    case 97:
            return snprintf(str, len, "change filament { growing_speed=%.3f }\n", linear(v, 0, 0.2));
        case 4:    case 100:
            return snprintf(str, len, "change motor { unloaded_speed=%.3f }\n", linear(v, -0.8, 0.8));
        case 5:    case 101:
            return snprintf(str, len, "change dynein { unloaded_speed=%.3f }\n", linear(v, -0.8, 0.8));
        case 6:    case 102:
            return snprintf(str, len, "change centrosome { stiffness=1000, %.3f }\n", linear(v, 1, 1000));
        case 7:    case 103:
            return snprintf(str, len, "change complex { stiffness=%.3f }\n", linear(v, 0, 256));
        case 8:    case 106:
        {
            float w = linear(v, 0.54, 1.0);
            float h = ( 0.85 * 0.85 * 0.21 ) / ( w * w );
            return snprintf(str, len, "change all space { length=%.3f, %f, %f }\n", w, w, h);
        }
    }
    return 0;
}



/// for the Novation Launchpad MK2
int makeCommand(char * str, size_t len, int X, int Y, int value)
{
    switch ( Y )
    {
        case 1: return snprintf(str, len, "change motor { unloaded_speed=%.3f }\n", srange(X, 0.8));
        case 2: return snprintf(str, len, "change dynein { unloaded_speed=%.3f }\n", srange(X, 0.8));
        case 3: return snprintf(str, len, "change filament { rigidity=%.3f }\n", prange(X, 20));
        case 4: return snprintf(str, len, "change filament { growing_speed=%.3f }\n", prange(X, 0.512));
        case 5: return snprintf(str, len, "change centrosome { stiffness=1000, %.3f }\n", prange(X, 1000));
        case 6: return snprintf(str, len, "change complex { stiffness=%.3f }\n", prange(X, 512));
        case 7: return snprintf(str, len, "change system { viscosity=%.3f }\n", prange(X, 1.0));
        case 8:
        {
            float w = linear((X-1)/7.0, 0.54, 1.0);
            float h = ( 0.85 * 0.85 * 0.21 ) / ( w * w );
            return snprintf(str, len, "change all space { length=%f, %f, %f }\n", w, w, h);
        }
    }
    return 0;
}

/// adjust LEDs on the Novation Launchpad MK2
void resetNovation(RtMidiOut& nova, byte color)
{
    // message to turn all buttons off:
    byte msg[9] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xE, 0, 0xF7 };
    nova.sendMessage(msg, 9);
    // light specific button:
    msg[0] = 144;
    msg[1] = 10*config+9;
    msg[2] = color;
    nova.sendMessage(msg, 3);
}

/// adjust LEDs on the Novation Launchpad MK2
void setNovationCol(RtMidiOut& nova, int X, int Y, byte color)
{
    // message to turn entire column off:
    byte msg[10] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xC, 0, 0, 0xF7 };
    msg[7] = X-1;
    nova.sendMessage(msg, 10);
    // light specific button:
    msg[0] = 144;
    msg[1] = 10*Y+X;
    msg[2] = color;
    nova.sendMessage(msg, 3);
}

/// adjust LEDs on the Novation Launchpad MK2
void setNovationRow(RtMidiOut& nova, int X, int Y, byte color)
{
    // message to turn entire row off:
    byte msg[10] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xD, 0, 0, 0xF7 };
    msg[7] = Y-1;
    nova.sendMessage(msg, 10);
    // light specific button:
    msg[0] = 144;
    msg[1] = 10*Y+X;
    msg[2] = color;
    nova.sendMessage(msg, 3);
    // light scenario button:
    msg[0] = 144;
    msg[1] = 10*config+9;
    msg[2] = 9;
    nova.sendMessage(msg, 3);

    // specify LED in RGB mode:
    //byte msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0x0B, LED, R, G, B, 0xF7 };
}


/// useful for debugging
void printMessage(Message& mes, FILE * out = stdout)
{
    fprintf(out, "MIDI: ");
    for ( size_t i = 0; i < mes.size(); ++i )
        fprintf(out, " %4X", mes[i]);
    fprintf(out, "\n");
}


/// listen to MIDI device and send commands via pipe
void goLive(RtMidiIn& midi, RtMidiOut& nova)
{
    Message msg;
    char cmd[4096] = { 0 };

    start(config);

    while ( child )
    {
        midi.getMessage(&msg);
        if ( mode > 2 ) printMessage(msg);
        
        // A MIDI message contains 3 bytes
        if ( msg.size() == 3 )
        {
            int button = (int) msg[1];
            int value = (int) msg[2];

            // Map button number to row and column indices in [1,9]
            int Y = button / 10;
            int X = button - 10 * Y;

            // last column changes scenario:
            if ( X == 9 )
            {
                if ( value == 0 )
                {
                    restart(Y%10);
                    resetNovation(nova, 9);
                }
                continue;
            }

            // act only when button is pressed down:
            if ( value > 0 )
            {
                //printf("button %i %i at %i\n", X, Y, value);

                // make the command suitable to cytosim
                int len = makeCommand(cmd, sizeof(cmd), X, Y, value);
                if ( len > 0 )
                {
                    if ( mode )
                    {
                        // copy message to terminal
                        write(STDOUT_FILENO, " > ", 3);
                        write(STDOUT_FILENO, cmd, len);
                    }
                    // send string down the pipe:
                    ssize_t s = write(fds[1], cmd, len);
                    if ( s == len )
                        setNovationRow(nova, X, Y, 45);
                }
                else if ( mode )
                    printf("Ignored button %i (value %i)\n", button, value);
            }
        }

        // Sleep for 5 milliseconds.
        usleep(5000);
    }
}

//------------------------------------------------------------------------------

void usage()
{
    printf("Cytomaster 1.0, G. Letort and F. Nedelec 8.5.2019\n");
    printf("   usage: cytomaster PORT MODE\n");
    printf("      PORT = MIDI port device number to listen to\n");
    printf("      MODE = [ 0, 1 ]\n");
    printf("  `cytomaster scan' will list known MIDI ports\n\n");
}


void scanPorts(RtMidiIn& midi)
{
    unsigned np = midi.getPortCount();
    
    if ( np < 1 )
        printf("No MIDI port detected!\n");
    else
    {
        printf("%u MIDI ports detected:\n", np);
        for ( unsigned p = 0; p < np; ++p )
        {
            midi.openPort(p);
            printf("   port %u is `%s'\n", p, midi.getPortName().c_str());
            midi.closePort();
        }
    }
}


int main( int argc, char *argv[] )
{
    RtMidiIn midi(RtMidi::MACOSX_CORE);
    RtMidiOut nova(RtMidi::MACOSX_CORE);

    if ( argc < 2 || 3 < argc || !isdigit(*argv[1]) )
    {
        if ( argc != 2 ) usage();
        scanPorts(midi);
        return 1;
    }

    unsigned nPorts = midi.getPortCount();
    unsigned port = (unsigned)atoi(argv[1]);

    if ( nPorts == 0 )
    {
        printf("No MIDI port detected!\n");
        return 1;
    }
    else if ( port >= nPorts )
    {
        printf("Invalid MIDI port specified\n");
        scanPorts(midi);
        return 1;
    }
    
    try
    {
        midi.openPort(port);
        nova.openPort(port);
        // set Novation to button layout 0
        byte msg[9] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0x22, 0x0, 0xF7 };
        nova.sendMessage(msg, 9);
    }
    catch ( RtMidiError &error )
    {
        error.printMessage();
        return 0;
    }
    
    // Don't ignore sysex, timing, or active sensing messages.
    midi.ignoreTypes(false, false, false);
    
    // Install an interrupt handler function.
    if ( signal(SIGINT, stop) == SIG_ERR )
        fprintf(stderr, "Could not register SIGINT handler\n");

    // Register a function to be called for Floating point exceptions:
    if ( signal(SIGPIPE, handler) == SIG_ERR )
        fprintf(stderr, "Could not register SIGPIPE handler\n");

    getcwd(cwd, LEN);
    printf("Cytomaster is listening to `%s'... please, terminate with Ctrl-C\n", midi.getPortName().c_str());
    
    if ( 2 < argc )
        mode = (unsigned)atoi(argv[2]);
    
    goLive(midi, nova);
    
    return 0;
}
