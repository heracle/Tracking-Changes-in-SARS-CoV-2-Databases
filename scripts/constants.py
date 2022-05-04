#!/usr/bin/env python
import hashlib, json

LOOKUP_ALIGN_FILEPATH = "/cluster/scratch/rmuntean/gisaid_data/lookup_2022-03-13.json"

def get_hash (seq: str) -> str:
    return hashlib.sha256(str(seq).encode('utf-8')).hexdigest()

def get_hash_lookup(filepath: str) :
    f = open(filepath)
    data = json.load(f)
    return data
