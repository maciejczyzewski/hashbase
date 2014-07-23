#!/usr/bin/env python
# -*- coding: utf-8 -*-
import socket                                    # hashbase
import sys

###########################################################
# Command-line interface                                  #
###########################################################
# >>                   | Client input                     #
# =>                   | Server output                    #
###########################################################

if len(sys.argv) != 3:
    print "Usage: python client.py <host> <port>"
    raise SystemExit

TCP_IP       = str(sys.argv[1])
TCP_PORT     = int(sys.argv[2])
BUFFER_SIZE  = 1024

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((TCP_IP, TCP_PORT))
except:
    print "Error: Cannot connect to the hashbase..."
    raise SystemExit    

def run():
    while True:
        try:
            g = raw_input(">> ")
        except KeyboardInterrupt:
            print "Warning: Closing connection..."
            raise SystemExit  

        s.send(g + "\r\n")
        r = s.recv(BUFFER_SIZE)[:-2]
        print "=>", r

    s.close()

run()