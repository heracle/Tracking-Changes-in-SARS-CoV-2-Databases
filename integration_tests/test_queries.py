import unittest
import subprocess
from subprocess import PIPE
from tempfile import TemporaryDirectory
import glob
import os

"""Test queries """

CTC = './run'
TEST_DATA_DIR = os.path.dirname(os.path.realpath(__file__)) + '/../test_data/'

class TestQueries(unittest.TestCase):
    def setUp(self):
        self.tempdir = TemporaryDirectory()

    def test_freq_bp(self):
        create_command = '{exe} create -o {outfile} {input}'.format(
            exe=CTC,
            outfile=self.tempdir.name + '/first',
            input=TEST_DATA_DIR + 'modified_sequence_v1.provision.json'
        )
        res = subprocess.run([create_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/first.h5',
            output_h5=self.tempdir.name + '/inter.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v2.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/inter.h5',
            output_h5=self.tempdir.name + '/second.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v3.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        stats_command = '{exe} stats {input_h5}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Total number of saved snapshots: 3
Size of 'data' field:12
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json' contains 5 treap nodes.
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json' contains 5 treap nodes.
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json' contains 4 treap nodes."""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------------------- query 0:  ""  without --compute-total-owner-cnt --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --num-to-print 34 --snapshot {snapshot} --exclude-deleted ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json

Target key ''


top 34 bp:
basepair index = 112	 total number of edits = 4
owner distribution per bp --> #owners=3 #edits=4:
	50% (2) 	 owner=ow3
		T>G 50% (1) 
		A>C 50% (1) 
	25% (1) 	 owner=ow4
		G>A 100% (1) 
	25% (1) 	 owner=ow1
		A>G 100% (1) 


basepair index = 42	 total number of edits = 2
owner distribution per bp --> #owners=2 #edits=2:
	50% (1) 	 owner=ow4
		G>T 100% (1) 
	50% (1) 	 owner=ow3
		T>C 100% (1) 


basepair index = 157	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow3
		A>C 100% (1) 


basepair index = 177	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow3
		A>T 100% (1) 




top 34 owners:
2	ow3
1	ow1
1	ow4"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 1:  ""  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --compute-total-owner-cnt --snapshot {snapshot} --exclude-deleted ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json

Target key ''


top 50 bp:
basepair index = 112	 total number of edits = 4
owner distribution per bp --> #owners=3 #edits=4:
	50% (2) 	 owner=ow3
		T>G 50% (1) 
		A>C 50% (1) 
	25% (1) 	 owner=ow4
		G>A 100% (1) 
	25% (1) 	 owner=ow1
		A>G 100% (1) 


basepair index = 42	 total number of edits = 2
owner distribution per bp --> #owners=2 #edits=2:
	50% (1) 	 owner=ow4
		G>T 100% (1) 
	50% (1) 	 owner=ow3
		T>C 100% (1) 


basepair index = 157	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow3
		A>C 100% (1) 


basepair index = 177	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow3
		A>T 100% (1) 




top 50 owners:
2	2	ow3
1	1	ow1
1	1	ow4"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 2:  "Oceania"  --------------------------------------------------------

        stats_command = '{exe} query --compute-total-owner-cnt -q bp_freq -i {input_h5} --snapshot {snapshot} --exclude-deleted "Oceania"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json

Target key 'Oceania'


top 50 bp:
basepair index = 112	 total number of edits = 2
owner distribution per bp --> #owners=2 #edits=2:
	50% (1) 	 owner=ow3
		A>C 100% (1) 
	50% (1) 	 owner=ow1
		A>G 100% (1) 


basepair index = 42	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow3
		T>C 100% (1) 


basepair index = 157	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow3
		A>C 100% (1) 




top 50 owners:
1	1	ow1
1	1	ow3"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 3:  "Europe"  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --compute-total-owner-cnt --snapshot {snapshot} --exclude-deleted "Europe"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json

Target key 'Europe'


top 50 bp:
basepair index = 112	 total number of edits = 2
owner distribution per bp --> #owners=2 #edits=2:
	50% (1) 	 owner=ow4
		G>A 100% (1) 
	50% (1) 	 owner=ow3
		T>G 100% (1) 


basepair index = 42	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow4
		G>T 100% (1) 


basepair index = 177	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
	100% (1) 	 owner=ow3
		A>T 100% (1) 




top 50 owners:
1	1	ow3
1	1	ow4"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 4:  "America"  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} --snapshot {snapshot} --exclude-deleted "America"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json

Target key 'America'


top 50 bp:


top 50 owners:"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 5:  "" version2 vs version3  -----------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} --num-to-print 23 --exclude-deleted ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	

Target key ''


top 23 bp:
basepair index = 112	 total number of edits = 6
owner distribution per bp --> #owners=2 #edits=6:
	66.6667% (4) 	 owner=ow3
		T>G 25% (1) 
		G>T 25% (1) 
		C>A 25% (1) 
		A>C 25% (1) 
	33.3333% (2) 	 owner=ow1
		G>A 50% (1) 
		A>G 50% (1) 


basepair index = 42	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
	100% (2) 	 owner=ow3
		T>C 50% (1) 
		C>T 50% (1) 


basepair index = 157	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
	100% (2) 	 owner=ow3
		C>A 50% (1) 
		A>C 50% (1) 


basepair index = 177	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
	100% (2) 	 owner=ow3
		T>A 50% (1) 
		A>T 50% (1) 




top 23 owners:
2	2	ow3
1	1	ow1"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------------------- query 6:  "" version2 vs version3 including deleted sequences -----------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	

Target key ''


top 50 bp:
basepair index = 112	 total number of edits = 7
owner distribution per bp --> #owners=3 #edits=7:
	57.1429% (4) 	 owner=ow3
		T>G 25% (1) 
		G>T 25% (1) 
		C>A 25% (1) 
		A>C 25% (1) 
	28.5714% (2) 	 owner=ow1
		G>A 50% (1) 
		A>G 50% (1) 
	14.2857% (1) 	 owner=ow4
		G>A 100% (1) 


basepair index = 42	 total number of edits = 3
owner distribution per bp --> #owners=2 #edits=3:
	66.6667% (2) 	 owner=ow3
		T>C 50% (1) 
		C>T 50% (1) 
	33.3333% (1) 	 owner=ow4
		G>T 100% (1) 


basepair index = 157	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
	100% (2) 	 owner=ow3
		C>A 50% (1) 
		A>C 50% (1) 


basepair index = 177	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
	100% (2) 	 owner=ow3
		T>A 50% (1) 
		A>T 50% (1) 




top 50 owners:
2	2	ow3
1	1	ow1
1	1	ow4"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

    def test_cnt_indels(self):
        create_command = '{exe} create -o {outfile} {input}'.format(
            exe=CTC,
            outfile=self.tempdir.name + '/first',
            input=TEST_DATA_DIR + 'modified_sequence_v1.provision.json'
        )
        res = subprocess.run([create_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/first.h5',
            output_h5=self.tempdir.name + '/second.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v2.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        append_command = '{exe} append -i {input_h5} -o {output_h5} {snapshots}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            output_h5=self.tempdir.name + '/third.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v3.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        stats_command = '{exe} stats {input_h5}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Total number of saved snapshots: 3
Size of 'data' field:12
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json' contains 5 treap nodes.
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json' contains 5 treap nodes.
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json' contains 4 treap nodes."""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # --------  Query 0 for cnt_indel for the third snapshot -----------------------------------------------------

        stats_command = '{exe} query -q cnt_indels -i {input_h5} --snapshot {snapshot} "" "Europe" "Oceania" "Void"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json,/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json,/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json
1.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json
2.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json

Target key ''
Insertions:	5	5	4	
Deletions:	0	0	1	
Modified:	0	4	7	

Target key 'Europe'
Insertions:	2	2	1	
Deletions:	0	0	1	
Modified:	0	2	3	

Target key 'Oceania'
Insertions:	3	3	3	
Deletions:	0	0	0	
Modified:	0	2	4	

Target key 'Void'
Insertions:	0	0	0	
Deletions:	0	0	0	
Modified:	0	0	0"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------- Query 1 for cnt_indel for the second snapshot (identical to the previous, but with shuffled queries) -----------------------------------------------------

        stats_command = '{exe} query -q cnt_indels -i {input_h5} --snapshot {snapshot} "Void" "Oceania" "Europe" "" '.format( # shuffle queries
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json,/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json,/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json
1.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json
2.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json

Target key 'Void'
Insertions:	0	0	0	
Deletions:	0	0	0	
Modified:	0	0	0	

Target key 'Oceania'
Insertions:	3	3	3	
Deletions:	0	0	0	
Modified:	0	2	4	

Target key 'Europe'
Insertions:	2	2	1	
Deletions:	0	0	1	
Modified:	0	2	3	

Target key ''
Insertions:	5	5	4	
Deletions:	0	0	1	
Modified:	0	4	7"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        # -------- Query 2 for cnt_indel for the second snapshot while excluding deletions -----------------------------------------------------

        stats_command = '{exe} query -q cnt_indels -i {input_h5} --snapshot {snapshot} --exclude-deleted "Void" "Oceania" "Europe" "" '.format( # shuffle queries
            exe=CTC,
            input_h5=self.tempdir.name + '/third.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json,/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json,/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)
        expected_stdout = """Snapshots:
0.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json
1.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json
2.	/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v3.provision.json

Target key 'Void'
Insertions:	0	0	0	
Deletions:	0	0	0	
Modified:	0	0	0	

Target key 'Oceania'
Insertions:	3	3	3	
Deletions:	0	0	0	
Modified:	0	2	4	

Target key 'Europe'
Insertions:	2	2	1	
Deletions:	0	0	0	
Modified:	0	2	2	

Target key ''
Insertions:	5	5	4	
Deletions:	0	0	0	
Modified:	0	4	6"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)
        