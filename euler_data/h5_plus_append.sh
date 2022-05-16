#!/bin/bash

# Example usage:
# ./h5_plus_append.sh /cluster/scratch/rmuntean/gisaid_data/aligned_owner_2021-08-08.h5 /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-03.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-10.provision.json.xz 

working_dir="/cluster/scratch/rmuntean/gisaid_data/create_append/"

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
        bsub -W 24:00 -J "unxz_fst_append" -R "rusage[mem=250000]" unxz "${bs_name}.provision.json.xz"
        echo "append fst:"
        bsub -W 24:00  -w "done(unxz_fst_append)" -J "append_${bs_name}" -R "rusage[mem=380000]" /cluster/home/rmuntean/git/tracking-changes/build/run append -i "${1}" -o "/cluster/scratch/rmuntean/gisaid_data/${bs_name}.h5" "${bs_name}.provision.json"
        last="${bs_name}" 
        last_job="append_${bs_name}"
        continue
    fi

    bsub -W 24:00 -J "unxz_${bs_name}" -w "done(${last_job})" -R "rusage[mem=250000]" unxz "${bs_name}.provision.json.xz"
    last_job="unxz_${bs_name}"
    echo "append:"
    bsub -W 24:00 -w "done(${last_job})" -J "append_${bs_name}" -R "rusage[mem=380000]" /cluster/home/rmuntean/git/tracking-changes/build/run append -i "/cluster/scratch/rmuntean/gisaid_data/${last}.h5" -o "/cluster/scratch/rmuntean/gisaid_data/${bs_name}.h5" "${bs_name}.provision.json"
    last_job="append_${bs_name}"
    bsub -W 24:00 -J "del_old_${bs_name}" -w "done(${last_job})" rm "/cluster/scratch/rmuntean/gisaid_data/${last}.h5" "${last}.provision.json"
    last_job="del_old_${bs_name}"
    last="${bs_name}"
done

bsub -W 24:00 -J "del_last" -w "done(${last_job})" rm "${last}.provision.json"

# (base) [rmuntean@eu-login-41 euler_data]$ ./h5_plus_append.sh /cluster/scratch/rmuntean/gisaid_data/aligned_owner_2021-11-21.h5 /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-28.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-05.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-12.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-19.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-26.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-02.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-09.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-16.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-23.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-30.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-06.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-13.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-20.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-27.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-03-06.provision.json.xz 
