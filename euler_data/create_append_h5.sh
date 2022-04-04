#!/bin/bash

last=""
last_job=""

for var in "$@"
do
    input_filepath="${var}"
    bs_name="$(basename ${input_filepath%%.*})"

    echo "input ${input_filepath}"
    echo "bs_name ${bs_name}"
    echo "var ${var}"

    if [ "${var}" == "${1}" ]; then
        echo "create:"
        bsub -W 4:00 -J "create_${bs_name}" -R "rusage[mem=100000]" ../build/run create -o "/cluster/scratch/rmuntean/gisaid_data/${bs_name}.h5" "${input_filepath}"
        last="${bs_name}" 
        last_job="create_${bs_name}"
        continue
    fi
    echo "append:"
    bsub -W 4:00 -w "done(${last_job})" -J "append_${bs_name}" -R "rusage[mem=180000]" ../build/run append -i "/cluster/scratch/rmuntean/gisaid_data/${last}.h5" -o "/cluster/scratch/rmuntean/gisaid_data/${bs_name}.h5" "${input_filepath}"
    last_job="append_${bs_name}"
    last="${bs_name}"
done
