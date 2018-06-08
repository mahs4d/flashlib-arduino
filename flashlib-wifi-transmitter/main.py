import sys
import os
import socket


def getArguments():
    ip = '192.168.5.1'
    file = os.getcwd() + '/a.hex'

    if len(sys.argv) > 1:
        file = sys.argv[1]

    if len(sys.argv) > 2:
        ip = sys.argv[2]

    return {'file': file, 'ip': ip}


def print_n_byte(target, n):
    return (target & (0xFF << (8 * n))) >> (8 * n)


def encodeString(s):
    t = ""
    for c in s:
        h = ord(c)
        t = t + chr(h + 5)
    return t


def send(ip, file):
    print('ip:' + ip)
    print('file:' + file)

    s = socket.socket()
    s.connect((ip, 8890))

    f = open(file, 'rb')

    lines = f.readlines()
    lines = [line.rstrip(b'\r\n')[9:-2] for line in lines if line[7:9] == b'00']

    size = 0
    for line in lines:
        size += len(line)

    f.close()


    print('File Size: ' + str(size))

    s.send('flashme'.encode('ascii'))

    bytestring = size.to_bytes(2, 'little')
    s.send(bytestring)
    print('Sending...')

    packcount = 0
    total = 0
    for line in lines:
        print(line)
        s.send(line)
        rdata = s.recv(2)
        print(rdata.decode('ascii'))
        total += len(line)
        packcount += 1

    print('Total Bytes Sent: ' + str(total))
    print('Total Packets Sent: ' + str(packcount))

    rdata = s.recv(2)
    rdata = rdata.decode('ascii')

    if rdata == 'OK':
        print('Flash Was Successfull')
    else:
        print('Error in Flashing')

    f.close()
    s.close()
    pass


if __name__ == '__main__':
    args = getArguments()
    send(args['ip'], args['file'])
    pass
