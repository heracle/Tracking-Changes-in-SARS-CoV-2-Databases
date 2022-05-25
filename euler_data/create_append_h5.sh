#!/bin/bash

working_dir="/cluster/scratch/rmuntean/gisaid_data/create_append/"

last=""
last_job=""

for var in "$@"
do
    cp "${var}" "${working_dir}"
done

pushd "${working_dir}"

for var in "$@"
do
    input_filepath="${var}"
    bs_name="$(basename ${input_filepath%%.*})"

    echo "input ${input_filepath}"
    echo "bs_name ${bs_name}"
    echo "var ${var}"


    if [ "${var}" == "${1}" ]; then
        bsub -W 24:00 -J "unxz_create" -R "rusage[mem=250000]" unxz "${bs_name}.provision.json.xz"
        echo "create:"
        bsub -W 24:00 -J "create_${bs_name}" -w "done(unxz_create)" -R "rusage[mem=200000]" /cluster/home/rmuntean/git/tracking-changes/build/run create -o "/cluster/scratch/rmuntean/gisaid_data/${bs_name}.h5" "${bs_name}.provision.json"
        last="${bs_name}" 
        last_job="create_${bs_name}"
        continue
    fi

    bsub -W 24:00 -J "unxz_${bs_name}" -w "done(${last_job})" -R "rusage[mem=250000]" unxz "${bs_name}.provision.json.xz"
    last_job="unxz_${bs_name}"
    echo "append:"
    bsub -W 24:00 -w "done(${last_job})" -J "append_${bs_name}" -R "rusage[mem=380000]" /cluster/home/rmuntean/git/tracking-changes/build/run append -i "/cluster/scratch/rmuntean/gisaid_data/${last}.h5" -o "/cluster/scratch/rmuntean/gisaid_data/${bs_name}.h5" "${bs_name}.provision.json"
    last_job="append_${bs_name}"

    bsub -W 24:00 -J "ls_${bs_name}" -w "done(${last_job})" ls -lah "/cluster/scratch/rmuntean/gisaid_data/${last}.h5"
    last_job="ls_${bs_name}"

    bsub -W 24:00 -J "ls2_${bs_name}" -w "done(${last_job})" ls -lah "${last}.provision.json"
    last_job="ls2_${bs_name}"


    bsub -W 24:00 -J "del_old_${bs_name}" -w "done(${last_job})"  rm "/cluster/scratch/rmuntean/gisaid_data/${last}.h5" "${last}.provision.json"
    last_job="del_old_${bs_name}"
    last="${bs_name}"
done

bsub -W 24:00 -J "ls_last" -w "done(${last_job})" ls -lah "${last}.provision.json"
bsub -W 24:00 -J "del_last" -w "done(ls_last)" rm "${last}.provision.json"


# (base) [rmuntean@eu-login-07 euler_data]$ ./create_append_h5.sh /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-06-27.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-04.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-11.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-18.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-25.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-01.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-08.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-15.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-22.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-29.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-05.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-12.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-19.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-26.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-03.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-10.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-17.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-24.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-31.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-07.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-14.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-21.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-28.provision.json.xz

# (base) [rmuntean@eu-login-47 euler_data]$ ./create_append_h5.sh /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-06-27.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-04.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-11.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-18.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-07-25.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-01.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-08.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-15.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-22.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-08-29.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-05.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-12.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-19.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-09-26.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-03.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-10.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-17.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-24.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-10-31.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-07.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-14.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-21.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-11-28.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-05.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-12.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-19.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2021-12-26.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-02.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-09.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-16.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-23.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-01-30.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-06.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-13.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-20.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-02-27.provision.json.xz /cluster/scratch/rmuntean/gisaid_data/to_add_owner/ready/aligned_owner_2022-03-06.provision.json.xz 
