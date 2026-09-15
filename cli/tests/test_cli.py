#!/usr/bin/env python3
import os
import pathlib
import random
import subprocess
import sys
import tempfile
import time
import unittest

EXE = os.path.abspath(sys.argv[1])
sys.argv[1:] = []


def run(args=(), data=b""):
    return subprocess.run([EXE, *args], input=data, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, check=False)


class Base64CliTests(unittest.TestCase):
    def test_rfc4648_vectors(self):
        vectors = [(b"", b""), (b"f", b"Zg=="), (b"fo", b"Zm8="),
                   (b"foo", b"Zm9v"), (b"foobar", b"Zm9vYmFy")]
        for plain, encoded in vectors:
            self.assertEqual(run(["-w0"], plain).stdout, encoded)
            decoded = run(["-d"], encoded)
            self.assertEqual(decoded.returncode, 0)
            self.assertEqual(decoded.stdout, plain)

    def test_default_wrapping_and_final_newline(self):
        result = run(data=b"a" * 60)
        self.assertEqual(result.returncode, 0)
        lines = result.stdout.splitlines()
        self.assertEqual([len(line) for line in lines], [76, 4])
        self.assertTrue(result.stdout.endswith(b"\n"))
        self.assertEqual(run(data=b"").stdout, b"")

    def test_custom_wrapping(self):
        self.assertEqual(run(["--wrap=4"], b"foobar").stdout,
                         b"Zm9v\nYmFy\n")
        self.assertEqual(run(["-w", "0"], b"foobar").stdout, b"Zm9vYmFy")
        self.assertEqual(run(["--wrap=+4"], b"foobar").stdout,
                         b"Zm9v\nYmFy\n")

    def test_binary_round_trip_across_blocks(self):
        rng = random.Random(12345)
        plain = rng.randbytes(200_003)
        encoded = run(["-w0"], plain)
        self.assertEqual(encoded.returncode, 0)
        decoded = run(["--decode"], encoded.stdout)
        self.assertEqual(decoded.returncode, 0)
        self.assertEqual(decoded.stdout, plain)

    def test_chunked_pipe_input(self):
        plain = b"first" + b"second-part" * 100
        process = subprocess.Popen([EXE, "-w0"], stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        process.stdin.write(plain[:5])
        process.stdin.flush()
        time.sleep(0.02)
        process.stdin.write(plain[5:])
        stdout, stderr = process.communicate()
        self.assertEqual((process.returncode, stderr), (0, b""))
        self.assertEqual(run(["-d"], stdout).stdout, plain)

    def test_file_and_dash_operands(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "binary input.dat"
            path.write_bytes(b"\x00\xffhello\n")
            encoded = run(["-w0", str(path)])
            self.assertEqual(encoded.returncode, 0)
            self.assertEqual(run(["-d", "-"], encoded.stdout).stdout,
                             path.read_bytes())

    def test_decode_newlines_and_ignore_garbage(self):
        self.assertEqual(run(["-d"], b"Zm9v\nYmFy\n").stdout, b"foobar")
        bad = run(["-d"], b"Zm!9v")
        self.assertNotEqual(bad.returncode, 0)
        good = run(["-di"], b"Zm!9v")
        self.assertEqual((good.returncode, good.stdout), (0, b"foo"))
        controls = bytes(range(0x21)).join((b"Zm9v", b"YmFy"))
        self.assertEqual(run(["-di"], controls).stdout, b"foobar")

    def test_large_whitespace_wrapping_across_blocks(self):
        plain = bytes(range(256)) * 1000 + b"tail"
        encoded = run(["-w0"], plain).stdout
        lines = [encoded[i:i + 80] for i in range(0, len(encoded), 80)]
        for separator, args in ((b"\n", ["-d"]),
                                (b"\r", ["-di"]),
                                (b"\r\n", ["-di"])):
            result = run(args, separator.join(lines) + separator)
            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, plain)
        self.assertNotEqual(run(["-d"], b"\r".join(lines)).returncode, 0)

    def test_invalid_arguments_and_input(self):
        for args in (["--no-such-option"], ["-w"], ["-w", "nope"],
                     ["one", "two"]):
            self.assertNotEqual(run(args).returncode, 0)
        self.assertNotEqual(run(["-d"], b"A").returncode, 0)

    def test_help_and_version(self):
        help_result = run(["--help"])
        self.assertEqual(help_result.returncode, 0)
        self.assertIn(b"--ignore-garbage", help_result.stdout)
        self.assertRegex(help_result.stdout,
                         rb"Active processing backend: (AVX2|NEON|scalar)\.")
        version = run(["--version"])
        self.assertEqual(version.returncode, 0)
        self.assertIn(b"Source: https://github.com/gaspardpetit/base64",
                      version.stdout.splitlines())
        self.assertEqual(run(["--dec"], b"Zm9v").stdout, b"foo")
        self.assertEqual(run(["--wr=4"], b"foo").stdout, b"Zm9v\n")


if __name__ == "__main__":
    unittest.main()
