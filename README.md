# Tool for Tracking Changes in Viral Databases (TCVD)

## How to install

### Clone this repository

### Install all git submodules

```
git clone https://gitlab.ethz.ch/rmuntean/tracking-changes.git
```

### Install all the external libraries: 
 - curl
 - libtool
 - hdf5

 For the Euler cluster

 ```
module load cmake/3.11.0 libtool/2.4
module load gcc/8.2.0
module load python/3.8.5
module load hdf5
```

### Make the TCVD binary:

```
mkdir <project root path>/build
cd <project root path>/build
cmake -D CMAKE_BUILD_TYPE=Release ..
make
```

## Run unit tests:

```
./unit_tests --gtest_brief=0 --gtest_filter="*"
```

## Run integration tests:
```
python ../integration_tests/main.py
```

# OLD readme

## Run cli cmd

```
bsub -W 24:00 -R "rusage[mem=150048]" ~/git/tracking-changes/build/run create -o first ../integration_tests/data/0000-01-01.provision.json

bsub -W 24:00 -R "rusage[mem=150048]" ~/git/tracking-changes/build/run append -i first.ctc -o second ../integration_tests/data/0000-01-01_with_2_changes.provision.json ../integration_tests/data/0000-01-01_with_4_changes.provision.json ../integration_tests/data/0000-01-01_with_6_changes.provision.json

bsub -W 24:00 -R "rusage[mem=150048]" ~/git/tracking-changes/build/run stats second.ctc 

```

## Align all sequences

Decompress the snapshot in focus:

```
bsub -J "unxz" unxz 2021-06-27.provision.json.xz
```

Split the snapshot in 64 modules of equal sizes.
```
bsub -W 4:00 -J "split_provision_json" -w "done(unxz)" -R "rusage[mem=180000]" python split_snapshot.py -i /cluster/scratch/rmuntean/gisaid_data/2021-06-27.provision.json -o /cluster/scratch/rmuntean/gisaid_data/2021-06-27_full -n 64 --lookup_hash_align ../data/lookup_seq_hash_to_alignment.json.xz
```

Align each snapshot module in parallel:
```
bsub -M 300G -n 32 -W 24:00 -J "align" -w "done(split_provision_json)" -R "rusage[mem=8000]" python align_snapshot_modules.py -d /cluster/scratch/rmuntean/gisaid_data/2021-06-27_full -n 64
```

Merge together the results:
```
bsub -W 4:00 -J "merge_aligned" -w "done(align)" -R "rusage[mem=180000]" python merge_aligned_modules.py -i /cluster/scratch/rmuntean/gisaid_data/2021-06-27_full -o /cluster/scratch/rmuntean/gisaid_data/full_2021-06-27_aligned.provision.json -n 64 --lookup_hash_align ../data/lookup_2021-06-27.json.xz 

```
