#!/usr/bin/env python
# -*- coding: utf-8 -*-
import hashbase                                  # hashbase
import sys

if len(sys.argv) != 3:
    print "Usage: python tests.py <host> <port>"
    raise SystemExit

# Connect
hb = hashbase.hashbase()
hb.connect(sys.argv[1], sys.argv[2])

# Write
hb.set("foo", "bar")
hb.set("maciej a.", "czyzewski")
hb.set("delete", "me")
hb.set("plus", "minus")

# Delete
hb.delete("delete")

# Get
print hb.get("foo")           # bar
print hb.get("maciej a.")     # czyzewski
print hb.get("delete")        # -1
print hb.get("plus")          # minus