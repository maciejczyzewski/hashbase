#!/usr/bin/env python
# -*- coding: utf-8 -*-
import socket                                    # hashbase

class hashbase:
    def __init__(self):
        self.buffer = 1024
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect(self, host, port):
        self.host = str(host)
        self.port = int(port)

        self.socket.connect((self.host, self.port))

    def set(self, key, value):
        self.socket.send("set \"" + str(key) + "\" \"" + str(value) + "\"\r\n")
        return self.socket.recv(self.buffer)[:-2]

    def get(self, key):
        self.socket.send("get \"" + str(key) + "\"\r\n")
        return self.socket.recv(self.buffer)[:-2]

    def delete(self, key): # del -> delete
        self.socket.send("del \"" + str(key) + "\"\r\n")
        return self.socket.recv(self.buffer)[:-2]