#! /usr/bin/env python3
# msgbot --- bot for message sample

import socket
import re
import random
import time
import sys

me = "高木貞治"

def sendit(c, s):
    s += "\r\n"
    c.send(s.encode('utf-8'))

# def recvit(c):
#     r = c.recv(4096)
#     print(r.decode('utf-8'))

class ProtocolError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def expect(c, code):
    r = c.recv(4096)
    s = r.decode('utf-8').split()
    if int(s[0]) != code:
        raise ProtocolError(code)

if len(sys.argv) < 3:
    print("usage: msgbot target_host target_port")
    exit(1)

target_host, target_port = sys.argv[1:3]
target_port = int(target_port)

regexp = re.compile('.*\r\n')
max_message_number = 0

try:
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((target_host, target_port))
    expect(client, 201)

    while True:
        sendit(client, "R")
        expect(client, 302)
        mm = ""
        while mm.find('\r\n203 MESSAGE ENDS\r\n') < 0:
            m = client.recv(1024*1000)
            mm += m.decode('utf-8')
            msg = regexp.findall(mm)
            #print(msg)

        while not msg[0].startswith("203"):
            message_number, message_name, message_addr, message_date, message_message = msg[0:5]
            del msg[0:5]

            a = message_number.split(' ')
            b = int(a[1].split('\r')[0])
            if b > max_message_number:
                max_message_number = b
                print(b)
            else:
                continue

            if message_name.find(me) >= 0:
                continue

            # ここにメッセージの処理を書く
            easy = ("易しい", "難しい", "楽しい", "苦しい")
            if message_message.find('数学') >= 0:
                ans = message_name[0:(len(message_name)-2)] + "君、数学は" + easy[random.randint(0, len(easy)-1)] + "ね。"
                #print(ans)
                sendit(client, "P" + me)
                expect(client, 301)
                sendit(client, ans)
                expect(client, 202)
                print("Sent: " + ans)

        time.sleep(10)

    #sendit(client, "Q")
    #recvit(client) # 209
    #expect(client, 209)

except ProtocolError as e:
    print(e)

client.close()
