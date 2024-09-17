# unhexdump

Command-line tool to produce a binary file from a hex dump.

Usage: `unhexdump -oOUTPUT INPUT`

Omit `INPUT` or `-oOUTPUT` to use stdin or stdout.
We will abort with an error if stdout is a TTY.

`INPUT` must consist of raw hexadecimal digits and whitespace.
Each line must contain an even count of digits.

Canonical hexdump formatting, with offset on the left and ASCII on the right, is *not* supported.
It's easy enough to filter that formatting out with sed or similar, before delivering to unhexdump.

Input may contain line comments starting with `#`.
