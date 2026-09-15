#!/usr/bin/env python3
"""Differential smoke tests against GNU coreutils base64.

Usage: python oracle.py PATH_TO_CANDIDATE [PATH_TO_GNU_BASE64]
The second argument defaults to `base64` and should resolve to GNU coreutils.
"""
import os
import random
import subprocess
import sys
import tempfile

candidate = os.path.abspath(sys.argv[1])
oracle = sys.argv[2] if len(sys.argv) > 2 else "base64"


def invoke(exe, args, data):
    return subprocess.run([exe, *args], input=data, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, check=False)


version = invoke(oracle, ["--version"], b"")
if version.returncode or b"GNU coreutils" not in version.stdout:
    raise SystemExit(f"oracle is not GNU coreutils: {oracle}")

rng = random.Random(0xB64)
cases = [b"", b"f", b"fo", b"foo", bytes(range(256))]
cases += [rng.randbytes(n) for n in (31, 32, 47, 48, 49, 4095, 49152, 100003)]

for data in cases:
    for args in ([], ["-w0"], ["--wrap=1"], ["--wr=5"], ["-w", "77"]):
        expected = invoke(oracle, args, data)
        actual = invoke(candidate, args, data)
        assert (actual.returncode == 0, actual.stdout) == \
               (expected.returncode == 0, expected.stdout), (args, len(data))
        assert actual.stdout == expected.stdout, (args, len(data))
    encoded = invoke(oracle, ["-w0"], data).stdout
    for args, source in ((["-d"], encoded), (["--dec"], encoded),
                         (["--decode"], b"\n".join(
                             encoded[i:i+13] for i in range(0, len(encoded), 13))),
                         (["-di"], b"!".join(
                             encoded[i:i+7] for i in range(0, len(encoded), 7)))):
        expected = invoke(oracle, args, source)
        actual = invoke(candidate, args, source)
        assert actual.returncode == expected.returncode, (args, len(data))
        assert actual.stdout == expected.stdout, (args, len(data))

# Compatibility-sensitive decoder cases: truncated quanta, misplaced or
# redundant padding, whitespace, garbage, and concatenated padded groups.
decode_cases = [
    b"A", b"AA", b"AAA", b"AAAA", b"Zg=", b"Zg==", b"Zg===",
    b"=Zg==", b"Z=g=", b"Zg==x", b"Zg==QQ==", b"Zm9v\r\n",
    b"Zm 9v", b"Zm!9v", b"====", b"_w==", b"/w==",
]
for source in decode_cases:
    for args in (["-d"], ["-di"]):
        expected = invoke(oracle, args, source)
        actual = invoke(candidate, args, source)
        assert actual.returncode == expected.returncode, \
            (args, source, expected.returncode, actual.returncode,
             expected.stdout, actual.stdout)
        assert actual.stdout == expected.stdout, \
            (args, source, expected.stdout, actual.stdout)

decoder_alphabet = b"ABZaz019+/=\n\r !-_"
for _ in range(80):
    source = bytes(rng.choice(decoder_alphabet) for _ in range(rng.randrange(24)))
    for args in (["-d"], ["-di"]):
        expected = invoke(oracle, args, source)
        actual = invoke(candidate, args, source)
        assert actual.returncode == expected.returncode and \
               actual.stdout == expected.stdout, \
            (args, source, expected.returncode, actual.returncode,
             expected.stdout, actual.stdout)

print(f"oracle comparison passed against {version.stdout.splitlines()[0].decode()}")
