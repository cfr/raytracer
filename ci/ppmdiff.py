#!/usr/bin/env python3
"""Compare two PPM images.

    ci/ppmdiff.py A.ppm B.ppm                       # require bit-identical
    ci/ppmdiff.py A.ppm B.ppm --rmse 0.5 --bias 0.2 # allow small differences

  rmse -- sqrt(mean((a-b)^2)) over all channels; magnitude of the difference
  bias -- mean(b-a); a systematic shift

"""

import argparse
import sys
from array import array


def read_ppm(path):
    """Return (width, height, maxval, array('B'|'H') of samples)."""
    with open(path, "rb") as f:
        data = f.read()

    pos = 0
    tokens = []
    while len(tokens) < 4:
        while pos < len(data) and data[pos : pos + 1].isspace():
            pos += 1
        if pos < len(data) and data[pos : pos + 1] == b"#":
            while pos < len(data) and data[pos : pos + 1] not in (b"\n", b"\r"):
                pos += 1
            continue
        start = pos
        while pos < len(data) and not data[pos : pos + 1].isspace():
            pos += 1
        if start == pos:
            raise ValueError(f"{path}: truncated header")
        tokens.append(data[start:pos])
    pos += 1

    magic = tokens[0]
    width, height, maxval = (int(t) for t in tokens[1:4])
    if magic not in (b"P3", b"P6"):
        raise ValueError(f"{path}: not a PPM (magic {magic!r})")
    count = width * height * 3

    if magic == b"P6":
        step = 1 if maxval < 256 else 2
        need = count * step
        if len(data) - pos < need:
            raise ValueError(f"{path}: truncated pixel data")
        samples = array("B" if step == 1 else "H")
        samples.frombytes(data[pos : pos + need])
        if step == 2:
            samples.byteswap() if sys.byteorder == "little" else None
    else:
        samples = array("H" if maxval > 255 else "B",
                        (int(t) for t in data[pos:].split()))
        if len(samples) < count:
            raise ValueError(f"{path}: truncated pixel data")
        del samples[count:]

    return width, height, maxval, samples


def compare(path_a, path_b):
    wa, ha, ma, a = read_ppm(path_a)
    wb, hb, mb, b = read_ppm(path_b)
    if (wa, ha) != (wb, hb):
        raise ValueError(f"size differs: {wa}x{ha} vs {wb}x{hb}")
    if ma != mb:
        raise ValueError(f"maxval differs: {ma} vs {mb}")

    if a == b:
        return 0.0, 0.0, 0, 0

    scale = 255.0 / ma
    total_sq = 0
    total_diff = 0
    ndiff = 0
    peak = 0
    for x, y in zip(a, b):
        d = y - x
        if d:
            ndiff += 1
            total_diff += d
            total_sq += d * d
            if abs(d) > peak:
                peak = abs(d)

    n = len(a)
    rmse = ((total_sq / n) ** 0.5) * scale
    bias = (total_diff / n) * scale
    return rmse, bias, ndiff, peak * scale


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a")
    ap.add_argument("b")
    ap.add_argument("--rmse", type=float, default=0.0,
                    help="max allowed RMSE in 8-bit units (default 0: exact)")
    ap.add_argument("--bias", type=float, default=0.0,
                    help="max allowed |mean signed difference| in 8-bit units")
    args = ap.parse_args()

    try:
        rmse, bias, ndiff, peak = compare(args.a, args.b)
    except (OSError, ValueError) as ex:
        print(f"ppmdiff: {ex}", file=sys.stderr)
        return 2

    if ndiff == 0:
        print(f"identical  {args.b}")
        return 0

    ok = rmse <= args.rmse and abs(bias) <= args.bias
    print(f"{'within tol' if ok else 'DIFFERS   '} {args.b}  "
          f"rmse={rmse:.4f} bias={bias:+.4f} peak={peak:.1f} "
          f"samples={ndiff}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
