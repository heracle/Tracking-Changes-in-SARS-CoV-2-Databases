#!/bin/bash

cores=32

input_filepath="$(pwd)/${1}"

echo "${input_filepath}"

bs_name="$(basename ${input_filepath})"

tmp_dir="/cluster/scratch/rmuntean/gisaid_data/tmp_${bs_name%%.*}/"

echo "tmp_dir = ${tmp_dir}"
rm -rf "${tmp_dir}"
mkdir "${tmp_dir}"

cp "${input_filepath}" "${tmp_dir}"
tmp_filepath="S{tmp_dir}${bs_name}"

pushd "${tmp_dir}"

bs_name=${bs_name%%.*}
tmp_filepath="${tmp_dir}${bs_name}.provision.json"
output_filepath="/cluster/scratch/rmuntean/gisaid_data/aligned_${bs_name}.provision.json"

echo "bs_name = ${bs_name}"
echo "tmp_dir = ${tmp_dir}"
echo "tmp_filepath = ${tmp_filepath}"

bsub -J "unxz_${bs_name}" unxz "${bs_name}.provision.json.xz"

bsub -W 4:00 -J "split_${bs_name}" -w "done(unxz_${bs_name})" -R "rusage[mem=180000]" python "/cluster/home/rmuntean/git/tracking-changes/scripts/split_snapshot.py" -i "${tmp_filepath}" -o "${tmp_dir}" -n "${cores}" --lookup_hash_align "/cluster/home/rmuntean/git/tracking-changes/data/lookup_2021-09-12.json.xz"
bsub -J "delete_bs_${bs_name}" -w "done(split_${bs_name})" rm "${tmp_filepath}"

bsub -M 300G -n "${cores}" -W 24:00 -J "align_${bs_name}" -w "done(split_${bs_name})" -R "rusage[mem=8000]" python "/cluster/home/rmuntean/git/tracking-changes/scripts/align_snapshot_modules.py" -d "${tmp_dir}" -n "${cores}"

bsub -W 4:00 -J "merge_${bs_name}" -w "done(align_${bs_name})" -R "rusage[mem=180000]" python "/cluster/home/rmuntean/git/tracking-changes/scripts/merge_aligned_modules.py" -i "${tmp_dir}" -o "${output_filepath}" -n "${cores}" --lookup_hash_align "/cluster/home/rmuntean/git/tracking-changes/data/lookup_2021-09-12.json.xz"

bsub -w "done(copy_${bs_name})" rm -rf "${tmp_dir}"
bsub -J "xz_${bs_name}" -w "done(merge_${bs_name})" xz "${output_filepath}"
bsub -J "copy_${bs_name}" -w "done(xz_${bs_name})" cp "${output_filepath}.xz" "/cluster/home/rmuntean/cevo/data/"
