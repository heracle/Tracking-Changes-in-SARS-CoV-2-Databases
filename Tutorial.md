# TCVD Introduction Tutorial

This tutorial has the purpose of presenting an overview of the TCVD tool capabilities. The small number of steps that will be listed will let the user get the basic experience to understand how and when to use this tool.

Firstly, the tool works with JSON viral snapshots that contain the following fields:

- Accession ID
- Collection Date
- Location
- Genome Sequence (in the aligned format)
- Owner Lab

## GISAID preprocessing

We need to run several preprocessing steps to add the `owner` and the `aligned sequence` fields to the standard GISAID SARS-CoV-2 snapshot. To facilitate the usage in the `Euler` cluster environment, a couple of scrips are already available to generate the proper TCVD snapshot file.

On the `Euler` cluster, we need to run the following commands:

```
mkdir /cluster/scratch/<user>/gisaid_data/
cd /cluster/scratch/<user>/gisaid_data/
mkdir gisaid_preproc_results/
wget https://polybox.ethz.ch/index.php/s/YhiY3ptw5LF6OzJ/download # lookup table: sequence hash to align sequence
wget https://polybox.ethz.ch/index.php/s/okkZtnCqhBinPUy/download # submitter information
<TCVD gir root>/src/external_libraries/nextclade-Linux-x86_64 dataset get --name 'sars-cov-2' --output-dir 'ref_sars-cov-2'
bsub unxz lookup_2022-03-13.json.xz
```

Modify multiple paths (line 4-6) with the proper content in:
```
vim <TCVD git root>/scripts/constants.py
```

Modify line 13 with the proper `Euler username` in:
```
vim <TCVD git root>/euler_data/GISAID_preprocessing.sh
```

Next, we need to provide the GISAID snapshot files in the `/cluster/scratch/<user>/gisaid_data/` directory.

At this stage, an `ls -lah /cluster/scratch/<user>/gisaid_data/` should return something similar to:

```
-rw-r----- 1 rmuntean rmuntean-group 323M Jun 20 15:58 2021-06-27.provision.json.xz
-rw-r----- 1 rmuntean rmuntean-group 341M Jun 20 15:59 2021-07-04.provision.json.xz
-rw-r--r-- 1 rmuntean rmuntean-group 2.9G Mar 15 13:55 2022-03-15_gisaid_submitter_information.csv
drwxr-x--- 2 rmuntean rmuntean-group 4.0K Jun 20 15:51 gisaid_preproc_results
-rw------- 1 rmuntean rmuntean-group 222G Jun 20 17:03 lookup_2022-03-13.json
drwxr-x--- 2 rmuntean rmuntean-group 4.0K May 22 08:10 __MACOSX
drwxr-x--- 2 rmuntean rmuntean-group 4.0K Jun 20 16:41 ref_sars-cov-2
```

Then, we can run the preprocessing on multiple snapshots in parallel:

```
cd <TCVD git root>/euler_data/
./GISAID_preprocessing.sh /cluster/scratch/<user>/gisaid_data/2021-06-27.provision.json.xz
./GISAID_preprocessing.sh /cluster/scratch/<user>/gisaid_data/2021-07-04.provision.json.xz
```

This last command can take around 4 hours for one GISAID snapshot of 100GB. 

## Merge multiple snapshots in one HDF5 file

For merging together multiple GISAID snapshots we need to set the `Euler Username` in the following two scripts:

```
vim <TCVD git root>/euler_data/create_append_h5.sh  # line 6
vim <TCVD git root>/euler_data/h5_plus_append.sh    # line 6
```

Then, we need to create a new directory where to save the temporary data during snapshot merging:

```
mkdir /cluster/scratch/<user>/gisaid_data/create_append
```

To compile the tool we need to:
```
mkdir <TCVD git root>/build
cd <TCVD git root>/build
cmake -D CMAKE_BUILD_TYPE=Debug ..
make
```

Now, let's suppose that `ls -lah /cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/` returns:

```
-rw-r----- 1 rmuntean rmuntean-group 254M Jun 21 14:14 aligned_owner_2021-06-27.provision.json.xz
-rw-r----- 1 rmuntean rmuntean-group 262M Jun 21 14:15 aligned_owner_2021-07-04.provision.json.xz
-rw-r----- 1 rmuntean rmuntean-group 272M Jun 21 14:15 aligned_owner_2021-07-11.provision.json.xz
-rw-r----- 1 rmuntean rmuntean-group 284M Jun 21 14:15 aligned_owner_2021-07-18.provision.json.xz
```

All the files have to be compressed in the XZ format. To create a H5 file from scratch, we need to run:

```
<TCVD git root>/euler_data/create_append_h5.sh /cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/aligned_owner_2021-06-27.provision.json.xz /cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/aligned_owner_2021-07-04.provision.json.xz 
```

At this stage, we should be able to find a new file at path `/cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/aligned_owner_2021-07-04.h5` that merges together the `2021-06-27` and `2021-07-04` GISAID snapshots.

To check if this new file is valid, we need to run:
```
(base) [rmuntean@eu-login-39 build]$ <TCVD git root>/build/run stats /cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/aligned_owner_2021-07-04.h5

Total number of saved snapshots: 2
Size of 'data' field:2257850
Snapshot 'aligned_owner_2021-06-27.provision.json' contains 2104885 treap nodes.
Snapshot 'aligned_owner_2021-07-04.provision.json' contains 2212241 treap nodes.
```

To append the remaining snapshots we need to run:

```
<TCVD git root>/euler_data/h5_plus_append.sh /cluster/scratch/<user>/gisaid_data/aligned_owner_2021-07-04.h5 /cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/aligned_owner_2021-07-11.provision.json.xz /cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/aligned_owner_2021-07-18.provision.json.xz 
```

To check one more time if the new generated file is valid and contains all the four snapshots, we need to run:
```
(base) [rmuntean@eu-login-39 build]$ <TCVD git root>/build/run stats /cluster/scratch/<user>/gisaid_data/gisaid_preproc_results/aligned_owner_2021-07-18.h5
```

## Run queries against the TCVD H5 file

For running a `frequency base-pairs` (`bp_freq`) query on the latest snapshot for the `North America` location prefix:

```
(base) [rmuntean@eu-login-39 build]$ ./run query -q bp_freq -i /cluster/scratch/<user>/gisaid_data/aligned_owner_2021-07-18.h5 "North America"
```

For running a `frequency base-pairs` (`bp_freq`) query on the `2021-07-04` snapshot for the `North America` location prefix:

```
(base) [rmuntean@eu-login-39 build]$ ./run query -q bp_freq -i /cluster/scratch/<user>/gisaid_data/aligned_owner_2021-07-04.h5 --snapshot "aligned_owner_2021-07-04.provision.json" "North America"
```

For running a `count indels` (`cnt_indels`) query on all the four snapshots for "North America", "Europe" and "Europe Switzerland" location prefixes:

```
(base) [rmuntean@eu-login-39 build]$ ./run query -q cnt_indels -i /cluster/scratch/<user>/gisaid_data/aligned_owner_2021-07-04.h5 --snapshot "aligned_owner_2021-06-27.provision.json,aligned_owner_2021-07-04.provision.json,aligned_owner_2021-07-11.provision.json,aligned_owner_2021-07-18.provision.json" "North America" "Europe" "Europe / Switzerland"
```