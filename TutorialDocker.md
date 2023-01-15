# TCVD Introduction Tutorial for Docker

This tutorial has the purpose of presenting an overview of the TCVD tool capabilities. The small number of steps that will be listed will let the user get the basic experience to understand how and when to use this tool.

The tool works with JSON viral snapshots that contain the following fields:

- Accession ID
- Collection Date
- Location
- Genome Sequence (in the aligned format)
- Owner Lab

Before starting running the following commands, please make sure that the git repository and all the submodules have been cloned to the local machine.

## Get into the docker container

Build the docker image:
```
docker build -t ctc_docker:v1 .
```

And run it inside a docker container:
```
docker run -i -t ctc_docker:v1 /bin/bash
```

## GISAID preprocessing

We need to run several preprocessing steps to add the `owner` and the `aligned sequence` fields to the standard GISAID SARS-CoV-2 snapshot. To facilitate the usage in the `Euler` cluster environment, a couple of scripts are already available to generate a TCVD snapshot file.

In the Docker container, we need to run the following commands:

```
mkdir -p /tmp/gisaid_data/gisaid_preproc_results
cd /tmp/gisaid_data/gisaid_preproc_results
wget --content-disposition https://polybox.ethz.ch/index.php/s/YhiY3ptw5LF6OzJ/download # lookup table: sequence hash to align sequence
wget --content-disposition https://polybox.ethz.ch/index.php/s/okkZtnCqhBinPUy/download # submitter information
bsub unxz lookup_2022-03-13.json.xz
<TCVD git root>/src/external_libraries/nextclade-Linux-x86_64 dataset get --name 'sars-cov-2' --output-dir 'ref_sars-cov-2'
```

We need to use a smaller database.