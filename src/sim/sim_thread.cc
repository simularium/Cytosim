// Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include <time.h>
#include "sim_thread.h"
#include "exceptions.h"
#include "print_color.h"
#include "picket.h"

//------------------------------------------------------------------------------

/**
 This uses a Parser that cannot write to disc.
 The function callback is called when Parser::hold() is reached.
 */
SimThread::SimThread(Simul& sim, void (*callback)(void))
: Parser(sim, 1, 1, 1, 1, 0), hold_callback(callback)
{
    hasChild = false;
    mFlag   = 0;
    mHold   = 0;
    mPeriod = 1;
    pthread_mutex_init(&mMutex, nullptr);
    pthread_cond_init(&mCondition, nullptr);
}

/**
 Possible issue:
 When quitting the application, this destructor might be called after the
 destructor of Simul(), in which case it will access non-existent data,
 most likely causing a crash().
 */
SimThread::~SimThread()
{
    //std::cerr << "~SimThread()\n";
    stop();
    pthread_cond_destroy(&mCondition);
    pthread_mutex_destroy(&mMutex);
}

//------------------------------------------------------------------------------
#pragma mark - Process control


void SimThread::debug(const char* msg) const
{
    if ( isChild() )
        fprintf(stdout, "\n- - -  %-16s", msg);
    else
        fprintf(stdout, "\n* * *  %-16s", msg);
}


void SimThread::gubed(const char* msg) const
{
    if ( isChild() )
        fprintf(stdout, "  - - %12s ", msg);
    else
        fprintf(stdout, "  * * %12s ", msg);
}


void SimThread::hold()
{
    assert_true( isChild() );

    if ( mFlag )
        pthread_exit(nullptr);
    
    if ( ++mHold >= mPeriod )
    {
        mHold = 0;
        //debug("holding");
        hold_callback();
        if ( mFlag )
            pthread_exit(nullptr);
        wait();  // this also unlocks and locks the mutex
    }
}


//------------------------------------------------------------------------------
#pragma mark - Lauching threads


void SimThread::run()
{
    assert_true( isChild() );
    try {
        Parser::readConfig();
    }
    catch( Exception & e ) {
        simul.relax();
        std::cerr << "\nError: " << e.what() << '\n';
        //flashText("Error: the simulation died");
    }
    hold_callback();
}


/** C-style function to cleanup after thread has terminated */
void child_cleanup(void * arg)
{
    SimThread * st = static_cast<SimThread*>(arg);
    st->hasChild = 0;
    st->unlock();
    //st->debug("ended");
}


/** C-style function to start a new thread */
void* run_launcher(void * arg)
{
    //std::clog << "slave  " << pthread_self() << '\n';
    SimThread * st = static_cast<SimThread*>(arg);
    st->lock();
    pthread_cleanup_push(child_cleanup, arg);
    st->run();
    pthread_cleanup_pop(1);
    return nullptr;
}


/**
 This attempts to start the live simulation by
 calling run() in the slave thread
 */
void SimThread::start()
{
    assert_false( isChild() );
    if ( !hasChild )
    {
        mFlag = 0;
        //std::clog << "master " << pthread_self() << '\n';
        if ( pthread_create(&child_, nullptr, run_launcher, this) )
            throw Exception("failed to create thread");
        hasChild = 1;
    }
}


//------------------------------------------------------------------------------


void SimThread::extend_run()
{
    assert_true( isChild() );
    try {
        Parser::execute_run(1<<20);
    }
    catch( Exception & e ) {
        std::cerr << "\nError: " << e.what() << '\n';
        simul.relax();
        //flashText("Error: %s", e.what());
    }
    hold_callback();
}


/** C-style function to start a new thread */
void* extend_launcher(void * arg)
{
    //std::clog << "slave  " << pthread_self() << '\n';
    SimThread * st = static_cast<SimThread*>(arg);
    st->lock();
    pthread_cleanup_push(child_cleanup, arg);
    st->extend_run();
    pthread_cleanup_pop(1);
    return nullptr;
}


/// call extend_code() in the slave thread
int SimThread::extend()
{
    assert_false( isChild() );
    if ( !hasChild )
    {
        mFlag = 0;
        //std::clog << "master " << pthread_self() << '\n';
        if ( pthread_create(&child_, nullptr, extend_launcher, this) )
            throw Exception("failed to create thread");
        hasChild = 1;
        return 0;
    }
    return 1;
}


//------------------------------------------------------------------------------
#pragma mark - Thread control & termination


void SimThread::step()
{
    assert_false( isChild() );
    if ( hasChild )
        signal();
}


/**
 ask the slave thread to exit at the next spontaneous halt
*/ 
void SimThread::stop()
{
    assert_false( isChild() );
    if ( hasChild )
    {
        // request clean termination:
        mFlag = 1;
        signal();
        // wait for termination:
        if ( hasChild )
        {
            //debug("join...");
            // wait for termination:
            pthread_join(child_, nullptr);
            hasChild = 0;
        }
    }
}

/**
 kill the slave thread immediately
 */
void SimThread::cancel()
{
    assert_false( isChild() );
    if ( hasChild )
    {
        mFlag = 2;
        //debug("cancel...");
        // force termination:
        if ( 0 == pthread_cancel(child_) )
        {
            // wait for termination:
            pthread_join(child_, nullptr);
            hasChild = 0;
            unlock();
        }
    }
}


void SimThread::restart()
{
    assert_false( isChild() );
    stop();
    clear();
    start();
}

//------------------------------------------------------------------------------
#pragma mark - Mouse-controlled Single


SingleProp * SimThread::getHandleProperty() const
{
    Property * p = simul.properties.find("single", "user_single");
    return static_cast<SingleProp*>(p);
}


SingleProp * SimThread::makeHandleProperty(real range)
{
    // Create a Hand that attaches fast and never detach:
    HandProp * hap = new HandProp("user_hand");
    hap->binding_range   = range;
    hap->binding_rate    = 10000;
    hap->unbinding_rate  = 0;
    hap->unbinding_force = INFINITY;
    hap->complete(simul);
    simul.properties.deposit(hap);

    SingleProp * sip = new SingleProp("user_single");
    sip->hand = "user_hand";
    sip->stiffness = 2000;
    sip->complete(simul);
    simul.properties.deposit(sip);
    
    return sip;
}


Single * SimThread::createHandle(Vector const& pos, real range)
{
    SingleProp * sip = getHandleProperty();
    if ( !sip )
        sip = makeHandleProperty(range);
    Single * res = new Picket(sip, pos);
    simul.singles.add(res);
    mHandle = res;
    return res;
}


ObjectList SimThread::allHandles(SingleProp const* sip) const
{
    return simul.singles.collect(match_property, sip);
}


bool SimThread::selectClosestHandle(Vector const& pos, real range)
{
    SingleProp * sip = getHandleProperty();
    
    if ( sip )
    {
        real dsm = 0;
        Single * res = nullptr;
        for ( Object * i : allHandles(sip) )
        {
            Single * s = static_cast<Single*>(i);
            real d = ( s->posFoot() - pos ).normSqr();
            if ( !res || d < dsm )
            {
                res = s;
                dsm = d;
            }
        }
        if ( res && dsm < range )
        {
            mHandle = res;
            return 1;
        }
    }
    return 0;
}


Single const* SimThread::handle() const
{
    SingleProp * sip = getHandleProperty();
    if ( sip && mHandle )
    {
        for ( Object * i : allHandles(sip) )
            if ( i == mHandle )
                return mHandle;
    }
    mHandle = nullptr;
    return nullptr;
}


void SimThread::detachHandle()
{
    if ( mHandle )
    {
        if ( mHandle->attached() )
            mHandle->detach();
    }
}

void SimThread::moveHandle(Vector const& pos)
{
    if ( mHandle )
    {
        mHandle->setPosition(pos);
    }
}


void SimThread::moveHandles(Vector const& vec)
{
    SingleProp * sip = getHandleProperty();
    if ( sip )
        ObjectSet::translateObjects(allHandles(sip), vec);
}


void SimThread::deleteHandles()
{
    lock();
    SingleProp * sip = getHandleProperty();
    if ( sip )
        simul.erase(allHandles(sip));
    mHandle = nullptr;
    unlock();
}

void SimThread::clear()
{
    assert_false( isChild() );
    simul.erase();
    mHandle = nullptr;
}

//------------------------------------------------------------------------------
#pragma mark - Parameter modifications

#if ( 0 )

#include <fcntl.h>

/// set file to not block on read() even if data is not available:
void set_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

#endif


/// check if file has data for input
inline int has_input(int fd)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0, 10};   // seconds, microseconds
    return select(1, &fds, nullptr, nullptr, &tv);
}


/**
 Read standard input and executes incoming commands.
 This should be executed by a process who already owns the lock on the data
 */
size_t SimThread::readInput(size_t max_nb_lines)
{
    const size_t LINESIZE = 2048;
    clearerr(stdin);
    
    if ( has_input(STDIN_FILENO) > 0 )
    {
        size_t cnt = 0;
        // some input is available, process line-by-line:
        char str[LINESIZE];
        
        // read one line from standard input (including terminating \n):
        while ( fgets(str, LINESIZE, stdin) )
        {
            //write(STDOUT_FILENO, ">>>> ", 5); write(STDOUT_FILENO, str, strlen(str));
            try {
                evaluate(str);
            }
            catch ( Exception & e ) {
                std::cerr << e.brief() << str;
            }
            if ( ++cnt >= max_nb_lines )
                break;
            // check if more input is available:
            if ( has_input(STDIN_FILENO) < 1 )
            {
                //printf("processed %i lines from standard input\n", cnt);
                break;
            }
        }
        return cnt;
    }
    return 0;
}

/**
 Read config file from the start, allowing parameters to be changed, while 
 simulation objects remain as they are. This will pause a running simulation 
 is running live, read the config file, and allow it to proceed.
 */
void SimThread::reloadParameters(std::string const& file)
{
    lock();
    // set a parser that can only change properties:
    Parser(simul, 0, 1, 0, 0, 0).readConfig(file);
    //std::cerr << "reloaded " << simul.prop->config_file << std::endl;
    unlock();
}


/**
 This will execute the given code, with full rights to modify Simul.
 
 A simulation running live will be paused; the code executed in another Parser,
 and the simulation then allowed to proceed.
 
 This can be executed by the parent thread who does not own the data
 */
void SimThread::execute(std::string const& code)
{
    lock();
    try {
        evaluate(code);
    }
    catch( Exception & e ) {
        std::cerr << "Error: " << e.what();
    }
    unlock();
}


/**
 Save current state in two files
 */
void SimThread::exportObjects(bool binary)
{
    lock();
    try {
        char str[64] = { '\0' };
        
        snprintf(str, sizeof(str), "properties%04li.cmo", reader_.currentFrame());
        simul.writeProperties(str, true);
        
        snprintf(str, sizeof(str), "objects%04li.cmo", reader_.currentFrame());
        simul.writeObjects(str, false, binary);
    }
    catch( Exception & e ) {
        std::cerr << "Error in Simul::exportObjects(): " << e.what();
    }
    unlock();
}


void SimThread::writeProperties(std::ostream& os, bool prune)
{
    lock();
    try {
        simul.writeProperties(os, prune);
    }
    catch( Exception & e ) {
        std::cerr << "Error in Simul::writeProperties(): " << e.what();
    }
    unlock();
}


