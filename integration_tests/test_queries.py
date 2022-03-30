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
            output_h5=self.tempdir.name + '/second.h5',
            snapshots=TEST_DATA_DIR + 'modified_sequence_v2.provision.json'
        )
        res = subprocess.run([append_command], shell=True)
        self.assertEqual(res.returncode, 0)

        stats_command = '{exe} stats {input_h5}'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """Total number of saved snapshots: 2
Size of 'data' field:8
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v1.provision.json' contains 5 treap nodes.
Snapshot '/cluster/home/rmuntean/git/tracking-changes/integration_tests/../test_data/modified_sequence_v2.provision.json' contains 5 treap nodes."""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)

        stats_command = '{exe} query -q bp_freq -i {input_h5} ""'.format(
            exe=CTC,
            input_h5=self.tempdir.name + '/second.h5'
        )
        res = subprocess.run([stats_command], shell=True, stdout=PIPE)
        self.assertEqual(res.returncode, 0)

        expected_stdout = """

 answer_altered_bp size=3
112	1
157	1
177	1


 top 50 bp:
177\t1
157\t1
112\t1"""
        stdout_pipe = res.stdout.decode().rstrip()
        self.assertEqual(stdout_pipe, expected_stdout)