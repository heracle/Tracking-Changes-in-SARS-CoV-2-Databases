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
vim <TCVD git root>/euler_data/GISAID_preprocessing
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

## 
