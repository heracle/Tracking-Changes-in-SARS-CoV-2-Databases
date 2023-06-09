#!/bin/bash

# example usage:
# ./GISAID_preprocessing.sh ../../../cevo/data/2022-03-13.provision.json.xz ../data/lookup_2022-03-13.json.xz

# Creates a file with the alignment in "/cluster/home/<user>/cevo/data/aligned_2022-03-13.provision.json.xz" and
# a lookup new file in "/cluster/home/<user>/git/tracking-changes/data/lookup_2022-03-13.json.xz"

# If the lookup decompressed file is not available at '/cluster/home/<user>/git/tracking-changes/data/lookup_2022-03-13.json.xz', then run:
#   cp /cluster/home/<user>/git/tracking-changes/data/lookup_2022-03-13.json.xz /cluster/scratch/<user>/gisaid_data/lookup_2022-03-13.json.xz
#   bsub -R "rusage[mem=200000]" unxz /cluster/scratch/<user>/gisaid_data/lookup_2022-03-13.json.xz

euler_user="<user>"
submitter_info_filepath="/cluster/scratch/${euler_user}/gisaid_data/2022-03-15_gisaid_submitter_information.csv"

cores=32

input_filepath="${1}"

if [ -z ${2} ]; then lookup_output_merge_arg=""; else lookup_output_merge_arg="--lookup_output ${2}"; fi

echo "input_filepath -->  ${input_filepath}"
echo "lookup_output_merge_arg --> ${lookup_output_merge_arg}"

bs_name="$(basename ${input_filepath})"

tmp_dir="/cluster/scratch/${euler_user}/gisaid_data/tmp_${bs_name%%.*}/"

echo "tmp_dir = ${tmp_dir}"
rm -rf "${tmp_dir}"
mkdir "${tmp_dir}"

cp "${input_filepath}" "${tmp_dir}"
tmp_filepath="S{tmp_dir}${bs_name}"

pushd "${tmp_dir}"

bs_name=${bs_name%%.*}
tmp_filepath="${tmp_dir}${bs_name}.provision.json"
output_filepath="/cluster/scratch/${euler_user}/gisaid_data/gisaid_preproc_results/aligned_owner_${bs_name}.provision.json"

echo "bs_name = ${bs_name}"
echo "tmp_dir = ${tmp_dir}"
echo "tmp_filepath = ${tmp_filepath}"

bsub -W 4:00 -R "rusage[mem=280000]" -J "unxz_${bs_name}" unxz "${bs_name}.provision.json.xz"

bsub -W 4:00 -J "split_${bs_name}" -w "done(unxz_${bs_name})" -R "rusage[mem=480000]" python "/cluster/home/${euler_user}/git/tracking-changes/scripts/split_snapshot.py" -i "${tmp_filepath}" -o "${tmp_dir}" -n "${cores}"
bsub -J "delete_bs_${bs_name}" -w "done(split_${bs_name})" rm "${tmp_filepath}"

bsub -M 300G -n "${cores}" -W 24:00 -J "align_${bs_name}" -w "done(split_${bs_name})" -R "rusage[mem=8000]" python "/cluster/home/${euler_user}/git/tracking-changes/scripts/align_snapshot_modules.py" -d "${tmp_dir}" -n "${cores}"

bsub -W 24:00 -J "merge_${bs_name}" -w "done(align_${bs_name})" -R "rusage[mem=280000]" python "/cluster/home/${euler_user}/git/tracking-changes/scripts/merge_aligned_modules.py" -i "${tmp_dir}" -o "${tmp_dir}no_owner_aligned.provision.json" -n "${cores}" ${lookup_output_merge_arg}

bsub -W 24:00 -J "add_onwer_${bs_name}" -w "done(merge_${bs_name})" -R "rusage[mem=280000]" python "/cluster/home/${euler_user}/git/tracking-changes/scripts/add_owner.py" -i "${tmp_dir}no_owner_aligned.provision.json" -o "${output_filepath}" --submitter-info "${submitter_info_filepath}"

bsub -w "done(add_onwer_${bs_name})" rm -rf "${tmp_dir}"
bsub  -W 24:00 -J "xz_${bs_name}" -w "done(add_onwer_${bs_name})" xz "${output_filepath}"
