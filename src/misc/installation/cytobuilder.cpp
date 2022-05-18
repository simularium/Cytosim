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

/// which scenario is running [1 ... 8]
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

//------------------------------------------------------------------------------
#pragma mark -

/// adjust LEDs on the Novation Launchpad MK2
void novaReset(RtMidiOut& nova, byte color)
{
    // set button layout 0 (session layout)
    byte msg[9] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0x22, 0x0, 0xF7 };
    nova.sendMessage(msg, 9);
    // turn all buttons off:
    //byte msg[9] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xE, 0, 0xF7 };
    msg[6] = 0xE;
    nova.sendMessage(msg, 9);
    // light specific button:
    if ( 0 < config )
    {
        msg[0] = 144;
        msg[1] = 10*config+9;
        msg[2] = color;
        nova.sendMessage(msg, 3);
    }
}


/// adjust LEDs on the Novation Launchpad MK2
void novaButton(RtMidiOut& nova, int X, int Y, byte color)
{
    byte msg[3] = { 144, 10*Y+X, color };
    nova.sendMessage(msg, 3);
}

/// adjust LEDs on the Novation Launchpad MK2
void novaButton(RtMidiOut& nova, int X, int Y, byte R, byte G, byte B)
{
    byte msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xB, 10*Y+X, R, G, B, 0xF7 };
    nova.sendMessage(msg, 12);
}


/// adjust LEDs on the Novation Launchpad MK2
void novaCol(RtMidiOut& nova, int X, int Y, byte color)
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
void novaRow(RtMidiOut& nova, int X, int Y, byte color)
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
    if ( 0 < config )
    {
        msg[0] = 144;
        msg[1] = 10*config+9;
        msg[2] = 9;
        nova.sendMessage(msg, 3);
    }
    // specify LED in RGB mode:
    //byte msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0x0B, LED, R, G, B, 0xF7 };
}

/// adjust LEDs on the Novation Launchpad MK2
void novaRamp(RtMidiOut& nova, int Y, int S, int E, byte R, byte G, byte B)
{
    byte msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xB, 10*Y+S-1, 0x3F, 0x0, 0x3F, 0xF7 };
    if ( S > 1 )
        nova.sendMessage(msg, 12);
    for ( int X = S; X <= E; ++X )
    {
        msg[ 7] = 10*Y+X;
        float s = (X-S+0.25)/float(E-S+0.25);
        msg[ 8] = R * s;
        msg[ 9] = G * s;
        msg[10] = B * s;
        nova.sendMessage(msg, 12);
    }
    //byte msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xB, LED, R, G, B, 0xF7 };
}


/// adjust LEDs on the Novation Launchpad MK2
void novaWedge(RtMidiOut& nova, int Y, byte R, byte G, byte B)
{
    byte msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xB, 10*Y+1, 0x3F, 0x0, 0x3F, 0xF7 };
    nova.sendMessage(msg, 12);
    for ( int X = 1; X <= 8; ++X )
    {
        msg[ 7] = 10*Y+X;
        float s = (fabs(X-4.5)-0.25) / 3.25;
        msg[ 8] = R * s;
        msg[ 9] = G * s;
        msg[10] = B * s;
        nova.sendMessage(msg, 12);
    }
    //byte msg[12] = { 0xF0, 0x00, 0x20, 0x29, 0x02, 0x18, 0xB, LED, R, G, B, 0xF7 };
}

void novaLights(RtMidiOut& nova)
{
    novaRamp(nova, 1, 2, 8, 63, 63, 63);
    novaRamp(nova, 2, 2, 8,  0, 63,  0);
    novaRamp(nova, 3, 2, 8,  0,  0, 63);
    novaWedge(nova, 4, 0, 63,  0);
    novaWedge(nova, 5, 0, 0, 63);
    novaRamp(nova, 6, 1, 8, 63,  0,  0);
    novaRamp(nova, 7, 1, 8,  0, 63, 63);
    novaButton(nova, 1, 8, 8);
    novaButton(nova, 2, 8, 16);
    novaButton(nova, 3, 8, 40);
    novaButton(nova, 4, 8, 48);
    novaButton(nova, 8, 8, 56);
}

//------------------------------------------------------------------------------
#pragma mark -

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
        execv(path, args);
        // if the command failed, check what is indicated by 'errno':
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
    char mem[6*LEN] = { 0 };
    char* args[6] = { mem, mem+LEN, mem+2*LEN, mem+3*LEN, mem+4*LEN, mem+5*LEN };
    
    snprintf(args[0], LEN, "play%i", conf);
    snprintf(args[1], LEN, "live");
    snprintf(args[2], LEN, "config%i.cym", conf);
    snprintf(args[3], LEN, "fullscreen=1");
    args[4] = nullptr;
    args[5] = nullptr;

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
    start(std::min(arg,8u));
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


/// for the Novation Launchpad MK2
int makeCommand(char * str, size_t len, int X, int Y, int value)
{
    switch ( Y )
    {
        case 1:
            if ( X == 1 )
                return snprintf(str, len, "delete all protein; delete all filament\n");
            else
                return snprintf(str, len, "new %i protein\n", 1<<(X-1));
        case 2:
            if ( X == 1 )
                return snprintf(str, len, "delete all bimotor\n");
            else
                return snprintf(str, len, "new %i bimotor\n", 1<<(X+1));
        case 3:
            if ( X == 1 )
                return snprintf(str, len, "delete all bidynein\n");
            else
                return snprintf(str, len, "new %i bidynein\n", 1<<(X+1));
        case 4:
            return snprintf(str, len, "change motor { unloaded_speed=%.1f }\n", srange(X, 0.8));
        case 5:
            return snprintf(str, len, "change dynein { unloaded_speed=%.1f }\n", srange(X, 0.8));
        case 6:
            return snprintf(str, len, "change filament { rigidity=%.3f }\n", prange(X, 20));
        case 7:
            return snprintf(str, len, "change bimotor { stiffness=%.3f }\n", prange(X, 512));
        case 8:
        {
            switch ( X )
            {
                case 1: return snprintf(str, len, "change cell { points=17 -10, 17 10, -17 10, -17 -10; }\n");
                case 2: return snprintf(str, len, "change cell { points=10 -10, 10 10, -10 10, -10 -10; }\n");
                case 3: return snprintf(str, len, "change cell { points=16 0.0, 8 10, -8 10, -16 0.0, -8 -10, 8 -10; }\n");
                case 4: return snprintf(str, len, "change cell { order=64; radius=10; }\n");
                case 8: return snprintf(str, len, "delete all filament\n");
            }
        }
    }
    return 0;
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
    novaReset(nova, 9);
    novaLights(nova);

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
                    restart(Y);
                    novaReset(nova, 9);
                    novaLights(nova);
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
                    // send string down the pipe:
                    ssize_t s = write(fds[1], cmd, len);
                    if ( mode )
                    {
                        if ( s != len )
                            write(STDOUT_FILENO, "Broken pipe", 11);
                        // copy message to terminal
                        write(STDOUT_FILENO, " > ", 3);
                        write(STDOUT_FILENO, cmd, len);
                    }
                }
                else if ( mode )
                    printf("Ignored button %i (value %i)\n", button, value);
            }
        }

        // Sleep for 5 milliseconds.
        usleep(5000);
    }
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#pragma mark -

void usage()
{
    printf("Cytobuilder 1.0, G. Letort and F. Nedelec 17.5.2019\n");
    printf("   usage: cytomaster PORT MODE\n");
    printf("      PORT = MIDI port device number to listen to\n");
    printf("      MODE = [ 0, 1 ]\n");
}


void scanPorts(RtMidiIn& midi)
{
    unsigned np = midi.getPortCount();
    
    if ( np < 1 )
        printf("No MIDI port detected!\n");
    else
    {
        printf("   %u MIDI ports detected:\n", np);
        for ( unsigned p = 0; p < np; ++p )
        {
            midi.openPort(p);
            printf("      port %u is `%s'\n", p, midi.getPortName().c_str());
            midi.closePort();
        }
    }
}


int main( int argc, char *argv[] )
{
    RtMidiIn midi(RtMidi::MACOSX_CORE);
    RtMidiOut nova(RtMidi::MACOSX_CORE);

    if ( 1 < argc && !isdigit(*argv[1]) )
    {
        usage();
        scanPorts(midi);
        return 1;
    }

    unsigned nPorts = midi.getPortCount();
    if ( nPorts == 0 )
    {
        printf("No MIDI port detected!\n");
        return 1;
    }
    
    unsigned port = 0;
    if ( argc > 1 && isdigit(*argv[1]) )
        port = (unsigned)atoi(argv[1]);
    if ( port >= nPorts )
    {
        printf("Invalid MIDI port specified\n");
        scanPorts(midi);
        return 1;
    }
    //fprintf(stderr, "port %u\n", port);

    try
    {
        midi.openPort(port);
        nova.openPort(port);
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
    printf("Cytobuilder is listening to `%s'... please, terminate with Ctrl-C\n", midi.getPortName().c_str());
    
    if ( 2 < argc )
        mode = (unsigned)atoi(argv[2]);
    
    goLive(midi, nova);
    
    return 0;
}
