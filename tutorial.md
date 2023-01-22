# TCVD Introduction Tutorial

This tutorial has the purpose of presenting an overview of the Traching Changes tool capabilities. The following steps will make basic setup for the minimal experience of making the tool work.

The tool works with JSON viral snapshots that contain the following fields:

- Accession ID
- Collection Date
- Location
- Genome Sequence (in the aligned format)
- Owner Lab

## Install the tool with all dependencies

<details>
  <summary markdown="span"> Build the docker container </summary>

Build the docker image:
```
docker build -t ctc_docker:v1 .
```

And run it inside a docker container:
```
docker run -i -t ctc_docker:v1 /bin/bash
```
</details>

<details>
  <summary markdown="span"> Install all dependencies </summary>

Build the docker image:
```
docker build -t ctc_docker:v1 .
```

And run it inside a docker container:
```
docker run -i -t ctc_docker:v1 /bin/bash
```
</details>
