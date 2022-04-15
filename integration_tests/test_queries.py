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

        stats_command = '{exe} query -q bp_freq -i {input_h5} --num-to-print 34 --snapshot {snapshot} ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """

top 34 bp:
basepair index = 112	 total number of edits = 3
owner distribution per bp --> #owners=3 #edits=3:
33.3333% 	 owner=ow4
33.3333% 	 owner=ow3
33.3333% 	 owner=ow1
char to char distribution per bp --> #char_to_char_types=3 #edits=3:
33.3333% T>A
33.3333% G>A
33.3333% A>G


basepair index = 42	 total number of edits = 2
owner distribution per bp --> #owners=2 #edits=2:
50% 	 owner=ow4
50% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=2 #edits=2:
50% T>C
50% G>T


basepair index = 157	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% A>C


basepair index = 177	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% A>T




top 34 owners:
2	ow3
1	ow1
1	ow4"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 1:  ""  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --compute-total-owner-cnt --snapshot {snapshot} ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """

top 50 bp:
basepair index = 112	 total number of edits = 3
owner distribution per bp --> #owners=3 #edits=3:
33.3333% 	 owner=ow4
33.3333% 	 owner=ow3
33.3333% 	 owner=ow1
char to char distribution per bp --> #char_to_char_types=3 #edits=3:
33.3333% T>A
33.3333% G>A
33.3333% A>G


basepair index = 42	 total number of edits = 2
owner distribution per bp --> #owners=2 #edits=2:
50% 	 owner=ow4
50% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=2 #edits=2:
50% T>C
50% G>T


basepair index = 157	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% A>C


basepair index = 177	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% A>T




top 50 owners:
2	2	ow3
1	1	ow1
1	1	ow4"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 2:  "Oceania"  --------------------------------------------------------

        stats_command = '{exe} query --compute-total-owner-cnt -q bp_freq -i {input_h5} --snapshot {snapshot} "Oceania"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """

top 50 bp:
basepair index = 42	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% T>C


basepair index = 112	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow1
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% A>G


basepair index = 157	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% A>C




top 50 owners:
1	1	ow1
1	1	ow3"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 3:  "Europe"  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq -i {input_h5} --compute-total-owner-cnt --snapshot {snapshot} "Europe"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """

top 50 bp:
basepair index = 112	 total number of edits = 2
owner distribution per bp --> #owners=2 #edits=2:
50% 	 owner=ow4
50% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=2 #edits=2:
50% T>A
50% G>A


basepair index = 42	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow4
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% G>T


basepair index = 177	 total number of edits = 1
owner distribution per bp --> #owners=1 #edits=1:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=1 #edits=1:
100% A>T




top 50 owners:
1	1	ow3
1	1	ow4"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 4:  "America"  --------------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} --snapshot {snapshot} "America"'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5',
            snapshot="/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json"
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """

top 50 bp:


top 50 owners:"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # -------------------- query 5:  "" version2 vs version3  -----------------------------------------------------

        stats_command = '{exe} query -q bp_freq --compute-total-owner-cnt -i {input_h5} --num-to-print 23 ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """

top 23 bp:
basepair index = 112	 total number of edits = 4
owner distribution per bp --> #owners=2 #edits=4:
50% 	 owner=ow3
50% 	 owner=ow1
char to char distribution per bp --> #char_to_char_types=4 #edits=4:
25% T>A
25% G>A
25% A>T
25% A>G


basepair index = 42	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=2 #edits=2:
50% T>C
50% C>T


basepair index = 157	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=2 #edits=2:
50% C>A
50% A>C


basepair index = 177	 total number of edits = 2
owner distribution per bp --> #owners=1 #edits=2:
100% 	 owner=ow3
char to char distribution per bp --> #char_to_char_types=2 #edits=2:
50% T>A
50% A>T




top 23 owners:
2	2	ow3
1	1	ow1"""

        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)


        # TODO add another integration test, identical to "query 5", but where to not ignore the deleted sequences
        # expected output:

# top 23 bp:
# basepair index = 112	 total number of edits = 6
# owner distribution per bp --> #owners=3 #edits=6:
# 33.3333% 	 owner=ow4
# 33.3333% 	 owner=ow3
# 33.3333% 	 owner=ow1
# char to char distribution per bp --> #char_to_char_types=4 #edits=6:
# 33.3333% G>A
# 33.3333% A>G
# 16.6667% T>A
# 16.6667% A>T


# basepair index = 42	 total number of edits = 4
# owner distribution per bp --> #owners=2 #edits=4:
# 50% 	 owner=ow4
# 50% 	 owner=ow3
# char to char distribution per bp --> #char_to_char_types=4 #edits=4:
# 25% T>G
# 25% T>C
# 25% G>T
# 25% C>T


# basepair index = 157	 total number of edits = 2
# owner distribution per bp --> #owners=1 #edits=2:
# 100% 	 owner=ow3
# char to char distribution per bp --> #char_to_char_types=2 #edits=2:
# 50% C>A
# 50% A>C


# basepair index = 177	 total number of edits = 2
# owner distribution per bp --> #owners=1 #edits=2:
# 100% 	 owner=ow3
# char to char distribution per bp --> #char_to_char_types=2 #edits=2:
# 50% T>A
# 50% A>T




# top 23 owners:
# 2	2	ow3
# 1	1	ow1
# 1	1	ow4