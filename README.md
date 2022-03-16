# Tracking Changes

## Install

```
mkdir build
cd build
cmake ..
make

../src/external_libraries/nextclade-Linux-x86_64 dataset get --name='sars-cov-2' --output-dir='../sars_cov_dataset'

wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.13/hdf5-1.13.0/src/hdf5-1.13.0.tar.gz -o ../src/external_libraries/
gzip -cd hdf5-1.13.0.tar.gz | tar xvf -

```

## Run unit tests:

```
./unit_tests --gtest_brief=0 --gtest_filter="*"
```

## Run integration tests:
```
python ../integration_tests/main.py
```

## Run cli cmd

```
bsub -W 24:00 -R "rusage[mem=150048]" ~/git/tracking-changes/build/run create -o first ../integration_tests/data/0000-01-01.provision.json

bsub -W 24:00 -R "rusage[mem=150048]" ~/git/tracking-changes/build/run append -i first.ctc -o second ../integration_tests/data/0000-01-01_with_2_changes.provision.json ../integration_tests/data/0000-01-01_with_4_changes.provision.json ../integration_tests/data/0000-01-01_with_6_changes.provision.json

bsub -W 24:00 -R "rusage[mem=150048]" ~/git/tracking-changes/build/run stats second.ctc 

```

## Align all sequences

```
bsub -W 4:00 -J "split_provision_json" -R "rusage[mem=180000]" python split_provision_json.py -i /cluster/scratch/rmuntean/gisaid_data/2021-06-27.provision.json -o /cluster/scratch/rmuntean/gisaid_data/tmp/2021-06-27_tmp_align -n 64

bsub -n 64 -W 4:00 -J "align" -w "done(split_provision_json)" -R "rusage[mem=180000]" python align.py -i /cluster/scratch/rmuntean/gisaid_data/tmp/2021-06-27_tmp_align -o /cluster/scratch/rmuntean/gisaid_data/tmp/2021-06-27_aligned -n 64

bsub -W 4:00 -J "merge_aligned" -w "done(align)" -R "rusage[mem=180000]" python merge_aligned_provision_jsons.py -i /cluster/scratch/rmuntean/gisaid_data/tmp/2021-06-27_aligned -o /cluster/scratch/rmuntean/gisaid_data/2021-06-27_aligned -n 64

```
