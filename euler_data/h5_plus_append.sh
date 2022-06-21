#!/bin/bash

# Example usage:
# ./h5_plus_append.sh /cluster/scratch/rmuntean/gisaid_data/aligned_owner_2021-08-08.h5 /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-03.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-10.provision.json.xz 

user="<user>"
working_dir="/cluster/scratch/${user}/gisaid_data/create_append/"

for var in "$@"
do
    if [ "${var}" == "${1}" ]; then
        continue
    fi
    cp "${var}" "${working_dir}"
done

pushd "${working_dir}"

last=""
last_job=""

for var in "$@"
do
    if [ "${var}" == "${1}" ]; then
        continue
    fi

    input_filepath="${var}"
    bs_name="$(basename ${input_filepath%%.*})"

    echo "input ${input_filepath}"
    echo "bs_name ${bs_name}"
    echo "var ${var}"


    if [ "${var}" == "${2}" ]; then
        bsub -W 24:00 -J "unxz_fst_append" -R "rusage[mem=40000]" unxz "${bs_name}.provision.json.xz"
        echo "append fst:"
        bsub -W 24:00  -w "done(unxz_fst_append)" -J "append_${bs_name}" -R "rusage[mem=40000]" /cluster/home/${user}/git/tracking-changes/build/run append -i "${1}" -o "/cluster/scratch/${user}/gisaid_data/${bs_name}.h5" "${bs_name}.provision.json"
        last="${bs_name}" 
        last_job="append_${bs_name}"
        continue
    fi

    bsub -W 24:00 -J "unxz_${bs_name}" -w "done(${last_job})" -R "rusage[mem=40000]" unxz "${bs_name}.provision.json.xz"
    last_job="unxz_${bs_name}"
    echo "append:"
    bsub -W 24:00 -w "done(${last_job})" -J "append_${bs_name}" -R "rusage[mem=40000]" /cluster/home/${user}/git/tracking-changes/build/run append -i "/cluster/scratch/${user}/gisaid_data/${last}.h5" -o "/cluster/scratch/${user}/gisaid_data/${bs_name}.h5" "${bs_name}.provision.json"
    last_job="append_${bs_name}"
    bsub -W 24:00 -J "del_old_${bs_name}" -w "done(${last_job})" rm "/cluster/scratch/${user}/gisaid_data/${last}.h5" "${last}.provision.json"
    last_job="del_old_${bs_name}"
    last="${bs_name}"
done

bsub -W 24:00 -J "del_last" -w "done(${last_job})" rm "${last}.provision.json"
