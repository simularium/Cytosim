# Compute Cluster - SLURM

This is a step-by-step introduction to running simulations on a SLURM cluster.
The instructions work at EMBL, where the cluster is running [SLURM](https://slurm.schedmd.com), with access to [central storage space](http://intranet.embl.de/it_services/services/data_storage/index.html).
 
Check also the <a href="https://wiki.embl.de/cluster">EMBL Cluster wiki</a>.

(F. Nedelec, 17.02.2018)

# Storage
 
You need a storage space that is accessible from any node of the cluster, 
and also from your local machine.
For this we will use a group-disc.
The first step is thus to make sure you know how access the group disc,
and to create a new directory on this disc:
	 
	cd /g/nedelec/steve
	mkdir screen
 
Here 'nedelec' is the name of the disc, and 'steve' is the name of the user.
You will need to substitute your username instead of 'steve' in the following
commands. The name of the working directory `screen` can be changed.
 
# Login
 
Log into the cluster:

	ssh login.cluster.embl.de

Go to a working directory (you may need to create it first):

	cd /g/nedelec/steve/screen
 
Copy the necessary python scripts located in `python/run`:

	cp cytosim/python/run/submit_slurm.py .
	cp cytosim/python/run/go_sim.py .
	cp cytosim/python/run/go_sim_lib.py .
	cp cytosim/python/run/preconfig.py .

# Compile
 
Copy your code to this new directory on the group disc:

	cp -R ~/cytosim /g/nedelec/steve/screen/cytosim

Load modules needed to compile and run cytosim:

	module load foss
	module load LAPACK OpenBLAS

Loading these modules is necessary, even if you are only going to submit jobs.

Edit `makefile.inc` to adjust compilation parameters for the cluster:

	cd /g/nedelec/steve/screen/cytosim
	edit makefile.inc

You need to change the target machine:

	MACHINE := cluster

and you should probably tell the compiler to optimize for speed:

	MODE := F

Moreover, you can disable assertions by defining `NDEBUG` in `src/base/assert_macro.h`:

	#define NDEBUG

Altogether, this will make the executable faster.
To compile using 4 parallel processes:

	make -j4 sim

Copy `sim` to the base directory:

	cp bin/sim ../sim

and check that it works:

	cd ..
	./sim info

# Prepare
 
As an example, here is how to configure a parametric sweep using a template
configuration file `config.cym.tpl`, as explained in @ref RunCytosim.
 
Generate the config files. Depending on your template, this may be:

	./preconfig.py config.cym.tpl

or, if the template uses random sampling, you should specify the number of files
(here 65) with:

	./preconfig.py 65 config.cym.tpl

# Submit

Submit the jobs:

	./submit_slurm.py sim config????.cym

The queue system attributes a job ID to your submission.

Note that if the submission is successful, all the config files will be copied
to the job folder, (in job??/todo) and you can thus delete the config files.

Optional: check that your jobs are in the queue, with these commands:

	squeue -l ID
	squeue -u $USER

If the jobs are correctly submitted, you can log out from the cluster:

	exit

# Optimal Cluster Usage

Multiple config files will be distributed to different nodes, but one can also
specify that each 'config' should be repeated several times when you submit.
This can be interesting to runs multiple simulations with the same config,
because cytosim is a stochastic engine, and this will allow you to probe the
intrisic variability in the outcome of the simulation, due to stochasticity.
The repeated runs are performed sequentially on the same node.

Examples:

To run the different config file on different nodes:

	./submit_slurm.py sim config1.cym config2.cym config3.cym config4.cym

It is naturally always possible to submit the same task multiple times:

	./submit_slurm.py sim config1.cym config1.cym config1.cym

The config file will be copied and submitted 3 times, and these 3 runs will
be distributed to different nodes.

To run the same config file 8 times, sequentially on the same node:

	./submit_slurm.py 'sim 8' config.cym

By combining thes options, each config files can be repeated sequentially on the
same node, while different configs files will be distributed to different nodes:

	./submit_slurm.py 'sim 3' config1.cym config2.cym config3.cym

Note: When repeating a simulation, you need to make sure to set `random_seed=0`
otherwise the results might be identical. With `random_seed=0`, the random number
generator is initialized automatically. If `random_seed` is not specified in the
`simul` section, then it is automatically set to zero.


# Cleanup
 
The interesting results will be in `/g/nedelec/steve/screen/save`, 
which should contain the completed runs as `run0000`, `run0001`, etc.
The other directories will have accessory informations.
 
Directory       |   Content                                                |
----------------|-----------------------------------------------------------
`todo`          | scripts created by `submit.py`, before or during the runs
`logs`          | standard output and standard error file for each job
`done`          | completed scripts that were moved from `todo`
`save`          | Completed results directories run0000, run0001, etc.

The config files in `todo` will be deleted once the simulations are completed.
The data in `logs` is only interesting if some error occurred,
and if everything went smoothly, you can delete it:

	rm -r todo
	rm -r logs
	rm -r done

There is also a log file in every completed run, that can be deleted if everything
proceeded without errors:

	rm save/run*/log.txt
	

# Analysis

The analysis should normally be done on a local machine, rather than on the cluster.
You are not allowed to do it on the machine used to submit jobs.
 
The type of analysis depends very much on the project.
Some general techniques are described in @ref RunCytosim.
 
