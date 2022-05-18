# Cytosim's python scripts


These scripts provide their own documentation if invoked with `help`:
	
	preconfig.py help

The help string is defined in the beggining of the file, so you can also just open the file,
with the links below.


### Recommended usage

Copy the scripts that you are using into a folder in your home directory, eg `cytobin`.

Add this directory to your [PATH](http://en.wikipedia.org/wiki/PATH_(variable)).

Make the scripts executable:

	chmod +x cytobin/*.py

Copy also francois's lib of python routines: [pyned.py](pyned.py)

# To start the simulation

* [`preconfig`](run/preconfig.py)
* [`go_sim`](run/go_sim.py) and [`go_sim_lib`](run/go_sim_lib.py)
* [`submit_slurm`](run/submit_slurm.py)

More scripts located in [`python/run`](run)

# To examine the simulation

* [`scan.py`](look/scan.py)
* [`collect.py`](look/collect.py)
* [`tell.py`](look/tell.py)
* [`make_page.py`](look/make_page.py)
* [`read_config.py`](look/read_config.py)

More scripts located in [`python/look `](look)
